from math import log, pi
import random
import numpy as np

from torch import Tensor
import torch
import dataFormat
import pandas as pd

#device = torch.device('cuda')
device = torch.device('cpu')

class SubjectJoins():
    
    def __init__(self):
        self.data = []
        self.current_query = 0
        self.rewardType = "linear" 
        self.featureType = "simple"
        self.setMaxId(50)

    def evaluate(self, state, action):
        
        for entry in self.data:
            diff = abs(sum(state-entry["features"]))
            if diff<0.01:
                return entry["values"][action]
        print(state, " not found")

    def step(self, action):
        # datapoint = self.data.iloc[self.current_query]
        datapoint = self.data[self.current_query]
        # select new random query
        self.current_query = random.randrange(0, len(self.data))
        newDatapoint = self.data[self.current_query]
        # newDatapoint = self.data.iloc[self.current_query]
        observation = newDatapoint["features"]
        rewards = self.rewards(datapoint)

        # 7. Return observation space, reward, if episode is done, {}
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
        print("Best: ",datapoint["best_value"],
            ", Worst: ",datapoint["worst_value"],
            ", Chosen:", datapoint["values"][action])
        print("Counts: ",datapoint["values"])

    # returns initial state
    def reset(self):
        self.current_query = random.randrange(0, len(self.data))
        newDatapoint = self.data[self.current_query]
        observation = newDatapoint["features"]
        # observation = self.data.features.iloc[self.current_query]
        return observation
        
    def calculateReward(self, datapoint, action):

        if self.rewardType=="linear":
            return self.linearReward(datapoint,action)
        elif self.rewardType=="log":
            return self.logReward(datapoint,action)
        else:
            return self.rationalReward(datapoint,action)
    

    def linearReward(self, datapoint, action):
        selected = datapoint["values"][action]
        best = datapoint["best_value"]
        worst = datapoint["worst_value"]
        return 1.0-(selected-best)/(worst-best)

    def rationalReward(self, datapoint, action):
        selected = datapoint["values"][action]
        best = datapoint["best_value"]
        worst = datapoint["worst_value"]
        return best/selected
    
    def logReward(self, datapoint, action):
        selected = log(datapoint["values"][action],10)
        best = log(datapoint["best_value"],10)
        worst = log(datapoint["worst_value"],10)
        return 1.0-(selected-best)/(worst-best)


    def feature_map(self, features):
        if self.featureType=="double":
            return self.featureMapDouble(features)
        elif self.featureType=="shuffle":
            return self.featureMapDoubleShuffle(features)
        else:
            return self.featureMapSimple(features)

    def normalize(self, value):
        maxvalue = self.maxId
        newmax = pi
        return value*newmax/maxvalue

    def featureMapSimple(self, features):
        result = []
        for f in features:
            result.append(self.normalize(f))
        return result

    def featureMapDouble(self, features):
        result = []
        for f in features:
            result.append(self.normalize(f))
        for f in features:
            result.append(self.normalize(f))
        return result

    def featureMapDoubleShuffle(self, features):
        result = []
        for f in features:
            result.append(self.normalize(f))
        for f in features:
            result.append(self.normalize(self.shuffleArray[f]))
        return result

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
            for i in range(0,dataFormat.NUM_JOINS):
                value = int(tmp[dataFormat.FIRST_RESULT+i])
                values.append(value)
                #find best value
                if value<min_value:
                    min_value=value
                    min_id=i
                if value>max_value:
                    max_value=value
                    max_id=i
            datapoint = {
                "features" : self.feature_map(features),
                "values" : values,
                "best_value": min_value,
                "best_id" : min_id,
                "worst_value": max_value,
                "worst_id" : max_id,
                "lupos_choice": lupos_choice 
            }
            self.data.append(datapoint)
        # self.data = pd.DataFrame(self.data)
        # print(torch.Tensor(self.data.features))
    
    def checkData(self):
        for entry in self.data:
            print("Num values: ",len(entry["values"]))


    def elvaluateModel(self, model):
        with torch.no_grad():
            sumLinear = 0
            sumLog = 0
            sumRational = 0
            countBest = 0
            countWorst = 0
            countWrong = 0
            betterLupos = 0
            for entry in self.data:
                state = Tensor(entry["features"])
                # state.to(device)
                prediction = model(state)
                selected = prediction.argmax()
                luposChoice = entry["lupos_choice"]
                if selected==15:
                    countWrong+=1
                    continue
                if entry["values"][selected]==entry["best_value"]:
                    countBest+=1
                if entry["values"][selected]==entry["worst_value"]:
                    countBest+=1
                if entry["values"][luposChoice]>=entry["best_value"]:
                    betterLupos+=1
                sumLinear+=self.linearReward(entry, selected)
                sumLog+=self.logReward(entry, selected)
                sumRational+=self.rationalReward(entry, selected)

        count = len(self.data)
        return [sumLinear/count, sumLog/count, sumRational/count, countBest, countWorst, countWrong, betterLupos]

    def setRewardType(self, type):
        self.rewardType = type

    def setFeatureType(self, type):
        self.featureType = type

    def setMaxId(self, id):
        self.maxId = id
        self.shuffleArray = list(range(id+1))
        random.shuffle(self.shuffleArray)

