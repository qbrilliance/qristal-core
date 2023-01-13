# Copyright (c) 2023 Quantum Brilliance Pty Ltd
# Methods adapted from source by T. Winker (winker@ifis.uni-luebeck.de)
#         cf. https://www.ifis.uni-luebeck.de/index.php?id=762

import random
import dataFormat
import numpy as np
from math import log, pi


class SubjectJoins():
  """

  Helper functions for join order optimization

  """

  def __init__(self):
    self.data = []
    self.current_query = 0
    self.rewardType = "linear"
    self.featureType = "simple"
    self.setMaxId(50)

  def evaluate(self, state, action):
    for entry in self.data:
      diff = abs(sum(state-entry["features"]))
      if diff < 0.01:
        return entry["values"][action]
    print(state, " not found")

  def step(self, action):
    datapoint = self.data[self.current_query]
    # select new random query
    self.current_query = random.randrange(0, len(self.data))
    newDatapoint = self.data[self.current_query]
    observation = newDatapoint["features"]
    rewards = self.rewards(datapoint)

    # Return observation space, reward, if episode is done, {}
    return observation, rewards, False, {}

  def evaluate(self, action):
    datapoint = self.data[self.current_query]
    rewards = self.rewards(datapoint)
    return rewards

  def rewardsId(self, id):
    return self.rewards(self.data[id])

  def rewards(self, datapoint):
    rewards = []
    for i in range(0, len(datapoint["values"])):
      rewards.append(self.calculateReward(datapoint, i))
    return rewards

  def test(self, id, action):
    datapoint = self.data.iloc[id]
    reward = self.calculateReward(datapoint, action)
    print("Id: ", id)
    print("Reward: ", reward)
    print("Best: ", datapoint["best_value"],
          ", Worst: ", datapoint["worst_value"],
          ", Chosen:", datapoint["values"][action])
    print("Counts: ", datapoint["values"])

  # returns initial state
  def reset(self):
    self.current_query = random.randrange(0, len(self.data))
    newDatapoint = self.data[self.current_query]
    observation = newDatapoint["features"]
    return observation

  def calculateReward(self, datapoint, action):
    selected = datapoint["values"][action]
    best = datapoint["best_value"]
    worst = datapoint["worst_value"]
    if self.rewardType == "linear":
      return 1.0-(selected-best)/(worst-best)
    elif self.rewardType == "log":
      selected = log(selected, 10)
      best = log(best, 10)
      worst = log(worst, 10)
      return 1.0-(selected-best)/(worst-best)
    else:
      return best/selected

  def feature_map(self, features):
    result = []
    for f in features:
      result.append(self.normalize(f))
    if self.featureType == "double":
      for f in features:
        result.append(self.normalize(f))
    elif self.featureType == "shuffle":
      for f in features:
        result.append(self.normalize(self.shuffleArray[f]))
    return result

  def normalize(self, value):
    maxvalue = self.maxId
    newmax = pi
    return value*newmax/maxvalue

  def load_data(self, data):
    self.data = []
    for entry in data:
      tmp = entry.split(",")
      lupos_choice = int(tmp[dataFormat.LUPOS_CHOICE])
      features = []
      values = []
      query = tmp[dataFormat.QUERY].split(";")
      # extract features
      for i in range(0, dataFormat.NUM_QUERIES):
          triple = query[i].split(" ")
          features.append(int(triple[1]))

      # extract results
      min_id = -1
      min_value = 1000000
      max_id = -1
      max_value = 0
      for i in range(0, dataFormat.NUM_JOINS):
        value = int(tmp[dataFormat.FIRST_RESULT+i])
        values.append(value)
        # find best value
        if value < min_value:
          min_value = value
          min_id = i
        if value > max_value:
          max_value = value
          max_id = i
      datapoint = {
        "features": self.feature_map(features),
        "values": values,
        "best_value": min_value,
        "best_id": min_id,
        "worst_value": max_value,
        "worst_id": max_id,
        "lupos_choice": lupos_choice
      }
      self.data.append(datapoint)

  def checkData(self):
    for entry in self.data:
      print("Num values: ", len(entry["values"]))

  def setRewardType(self, type):
    self.rewardType = type

  def setFeatureType(self, type):
    self.featureType = type

  def setMaxId(self, id):
    self.maxId = id
    self.shuffleArray = list(range(id + 1))
    random.shuffle(self.shuffleArray)
