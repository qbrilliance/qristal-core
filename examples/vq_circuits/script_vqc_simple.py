import qbos as qb
import numpy as np
import torch
from torch import Tensor
from torch.optim import LBFGS, SGD, Adam, RMSprop
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(sys.path[0])),'plugins','optimisation_modules','QML'))
import qb_qml 
from datetime import datetime
import time
from collections import deque
# import torchviz
import dataFormat
from subjectJoins import SubjectJoins
from imports import layer

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
    "encoding": "rx",
    "reuploading" : True,
    "reps" : 5,
    "calc" : "yz",
    "entangleType": "circular",
    "entangle" : "cx",
    "reward" : "rational",
    "numEpisodes" : 20,
    "optimizer": "adam",
    "lr" : 0.01
}

num_qubits = 4

# --------------------------------------------------------------
# ----------------- Begin quantum section ----------------------
# --------------------------------------------------------------

torch.set_default_dtype(torch.float64)
calcCircuits = qb_qml.stringToCircuitList(settings["calc"])
reps = settings["reps"]
num_params = num_qubits*reps*len(calcCircuits)

# Connect to PyTorch
initial_weights = np.pi*(2*np.random.rand(num_params) - 1)
quantum_nn = qb_qml.QuantumLayer(init_weights=initial_weights)

# --------------------------------------------------------------
# ------------------- End quantum section ----------------------
# --------------------------------------------------------------


# Define neural network and loss

normLayer = layer.NormLayer()
model = torch.nn.Sequential(quantum_nn, normLayer)

loss_fn = torch.nn.MSELoss()


""" 


Uncomment code below to test if the model works + visualize the model



"""

# testtensor = torch.Tensor([1.0]*num_qubits)
# testresult = model(testtensor)
# print([param for param in model.named_parameters()]) # Printing all parameters that can have weights varied
# torchviz.make_dot(testresult,params=dict(model.named_parameters())) # To visualise the computational graph of the NN


"""

Code for running the actual model


"""

# create reinforcement environment
env = SubjectJoins()
env.setRewardType(settings["reward"])
env.setFeatureType(settings["features"])

# load data
i_filename= "data.csv"
with open(i_filename, "r") as input:
    env.load_data(input)

# choose optimizer
if settings["optimizer"]=="adam":
    optimizer = Adam(model.parameters(), lr=settings["lr"], amsgrad=True)
# # elif settings["optimizer"]=="SGD":
#      optimizer = SGD(model.parameters(), lr=settings["lr"], momentum=0.9)
# # else:
# #     optimizer = Adam(model.parameters(), lr=settings["lr"])

logInterval = 100
numEpisodes = settings["numEpisodes"]

print("Settings: ", settings)

 # initialize variables for live evaluation
rewards = []
av_rewards = []
best_score = 0
rewardList = deque(maxlen=40)

# # initial observation
state = env.reset()  

loss_arr = np.zeros(numEpisodes)
reward_arr = np.zeros(numEpisodes)
scheduler = torch.optim.lr_scheduler.ExponentialLR(optimizer, gamma=0.9)
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
    sumRewards = 0
    countRewards = 0
    for reward in rewardList:
        sumRewards+= reward
        countRewards+= 1
    averageReward = sumRewards/countRewards

    # optimize
    loss= 0
    loss = loss_fn(prediction, torch.Tensor(rewards))
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
    loss_arr[episode] = loss.item()
    reward_arr[episode] = averageReward
    # print current result
    print("Episode: {}, loss: {:.3f}, Reward : {:.3f}".format(episode, loss_arr[episode] , averageReward), end="\n")

now = datetime.now()
current_time = now.strftime("%H:%M:%S")
print("End Time =", current_time)
t2 = time.time()
print("Time Taken = " + str(t2-t1))
