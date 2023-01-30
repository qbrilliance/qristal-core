# Copyright (c) 2023 Quantum Brilliance Pty Ltd
# Methods adapted from source by T. Winkler (winker@ifis.uni-luebeck.de)
#         cf. https://www.ifis.uni-luebeck.de/index.php?id=762

import torch
import sys
import os
import time
import torchviz
import dataFormat
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
from datetime import datetime
from torch import Tensor
from torch.optim import SGD, Adam
from subjectJoins import SubjectJoins
from qb.core import String, VectorString 
from qb.core.optimization import defaultAnsatzes, QMLParamCirc, QMLExecutor
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(sys.path[0]))), 'src', 'optimization', 'qml'))
from qb_qml_torch_wrapper import QuantumLayer, NormLayer

now = datetime.now()
current_time = now.strftime("%H:%M:%S")
print("Start Time =", current_time)
t1 = time.time()

# Fix seed for reproducibility
seed = 42
np.random.seed(seed)
torch.manual_seed(seed)

# default settings
settings = {
  "features": "simple",
  "reps": 5,
  "reward": "rational",
  "numEpisodes": 3000,
  "optimizer": "adam",
  "lr": 0.01
}

num_qubits = 4

# --------------------------------------------------------------
# ----------------- Begin quantum section ----------------------
# --------------------------------------------------------------

torch.set_default_dtype(torch.float64)
reps = settings["reps"]
param_gates = VectorString()
param_gates.append(String(["Ry"]))
param_gates.append(String(["Rz"]))
num_params = num_qubits*reps*len(param_gates)
circuit = QMLParamCirc(num_qubits, defaultAnsatzes.qrlRDBMS, reps, param_gates)
# Connect to PyTorch
initial_weights = np.pi*(2*np.random.rand(num_params) - 1)
quantum_nn = QuantumLayer(circuit, init_weights=initial_weights)

# --------------------------------------------------------------
# ------------------- End quantum section ----------------------
# --------------------------------------------------------------


# Define neural network 

normLayer = NormLayer()
model = torch.nn.Sequential(quantum_nn, normLayer)

"""


Uncomment code below to test if the model works + visualize the model


"""

# testtensor = torch.Tensor([1.0]*num_qubits)
# testresult = model(testtensor)
# # Printing all parameters that can have weights varied
# print([param for param in model.named_parameters()]) 
# # To visualise the computational graph of the NN
# torchviz.make_dot(testresult,params=dict(model.named_parameters()))


"""


Code for running the model w/backprop


"""

# create reinforcement environment
env = SubjectJoins()
env.setRewardType(settings["reward"])
env.setFeatureType(settings["features"])

# load data
i_filename = "data.csv"
with open(i_filename, "r") as input:
  env.load_data(input)

# choose optimizer
if settings["optimizer"] == "adam":
  optimizer = Adam(model.parameters(), lr=settings["lr"], amsgrad=True)
elif settings["optimizer"] == "SGD":
  optimizer = SGD(model.parameters(), lr=settings["lr"], momentum=0.9)
else:
  optimizer = Adam(model.parameters(), lr=settings["lr"])

logInterval = 100
numEpisodes = settings["numEpisodes"]

print("Settings: ", settings)

# initialize variables for live evaluation
rewards = []
av_rewards = []
best_score = 0
rewardList = deque(maxlen=40)

# initial observation
state = env.reset()

scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=300, gamma=0.0009)

loss_arr = np.zeros(numEpisodes)
avg_reward_arr = np.zeros(numEpisodes - 100)
reward_arr = np.zeros(numEpisodes)
loss_fn = torch.nn.MSELoss()
# train the agent
for episode in range(numEpisodes):
  # learn a new state
  state = Tensor(state)
  prediction = model(state)
  selected = prediction.argmax()
  state, rewards, done, info = env.step(selected)
  # pad rewards to 16
  rewards.append(0)
  reward = rewards[selected]

  # calculate average for console output
  rewardList.append(reward)
  sumRewards = sum(rewardList)
  countRewards = len(rewardList)
  averageReward = sumRewards/countRewards
  # optimize
  loss = 0
  for i in range(0, len(rewards)):
      loss += (prediction[i] - rewards[i])**2
  optimizer.zero_grad()
  loss.backward()
  optimizer.step()
  loss_arr[episode] = loss.item()
  reward_arr[episode] = averageReward
  if episode >= 100:
      avg_reward_arr[episode - 100] = np.mean(reward_arr[episode-100:episode])
  scheduler.step()
  # print current result
  print("Episode: {}, loss: {:.3f}, Reward : {:.3f}".format(episode, loss_arr[episode] , averageReward), end="\n")

now = datetime.now()
current_time = now.strftime("%H:%M:%S")
print("End Time =", current_time)
t2 = time.time()
print("Time Taken = " + str(t2-t1) + " seconds")

"""


Uncomment code below to save reward plots and arrays as .npy file


"""

# plt.plot(np.arange(numEpisodes), reward_arr)
# plt.savefig('reward_cpp.png')

# plt.plot(np.arange(100, numEpisodes), avg_reward_arr)
# plt.ylim([0.35, 0.95])
# plt.savefig('avg_reward_cpp.png')

# with open('cpp.npy', 'wb') as f:
#   np.save(f, reward_arr)
#   np.save(f, avg_reward_arr)
