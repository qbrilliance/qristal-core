# Example demonstration amplitude estimation circuit

# Test the example here:
# https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
# i.e., estimate the amplitude of the state:
# sqrt(1-p)|0> + sqrt(p)|1>
import numpy as np
import qristal.core
from qristal.core import run_MLQAE
import ast
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
from matplotlib import cm

p = 0.24
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qristal.core.Circuit()
state_prep.ry(0, theta_p)

# In this case, we don't construct the Grover operator by ourselves,
# instead, just provide the oracle to detect the marked state (|1>)
oracle = qristal.core.Circuit()
oracle.z(0)
num_runs = [1,2,3,4,5,6,7,8,9,10]
shots = [20,40,60,80,100,150,200,250,300,500,750,1000]
best_score = 0
def is_in_good_subspace(s,x):
    if int(s[0]) == 1:
        return 1
    else:
        return 0
total_num_qubits = 1
score_qubits = [0]

# Execute:
data = []
for i in num_runs:
    for j in shots:
        result = run_MLQAE(state_prep, oracle, is_in_good_subspace, score_qubits, total_num_qubits, i, j)
        result_dict = ast.literal_eval(result)
        amplitude_estimate = float(result_dict['AcceleratorBuffer']['Information']['amplitude-estimation'])
        data_val = [i,j,amplitude_estimate]
        data.append(data_val)
        print(data_val)

# Plot results
fig = plt.figure()
ax = plt.axes(projection ='3d')
x = [data[i][0] for i in range(len(data))]
y = [data[i][1] for i in range(len(data))]
z = [data[i][2] for i in range(len(data))]
c = z
ax.scatter(x,y,z,c=c,cmap=cm.coolwarm)
lengthx = len(x)
lengthy = len(y)
X,Y = np.meshgrid(x,y)
actual_amplitude = np.array([np.sqrt(p)]*(lengthx*lengthy)).reshape(lengthx,lengthy)
ax.plot_surface(X,Y,actual_amplitude,color='green',alpha=0.1)
ax.set_title('MLQAE as a function of #runs and #shots')
ax.set_xlabel("num_runs")
ax.set_ylabel("num_shots")
ax.set_zlabel("Amplitude Estimate")
plt.savefig("MLQAE_parameter_test.png")
