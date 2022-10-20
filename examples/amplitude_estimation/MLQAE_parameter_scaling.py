# Example demonstration amplitude estimation circuit

# Estimate the amplitude of the state:
# sqrt(1-p)|00...0> + sqrt(p)|11...1>
# for various numbers of qubits

import numpy as np
import qbos as qb
import ast
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d 
from matplotlib import cm
from qbos import run_MLQAE
from qbos import run_canonical_ae_with_oracle
tqb = qb.core()

method = 'mlqae'
ps = [0.49]
for p in ps: 
    theta_p = 2 * np.arcsin(np.sqrt(p))
    n = 1 # num qubits
    print("amplitude = ", np.sqrt(p))
    while n<=10:
        # State prep circuit: (preparing the state that we want to estimate the amplitude)
        state_prep = qb.Circuit()
        state_prep.ry(0, theta_p)
        for i in range(n-1):
            state_prep.cnot(i,i+1)
        
        # Oracle
        oracle = qb.Circuit()
        # The oracle should be a multi-controlled z gate
        if n == 1:
            oracle.z(0)
        else:
            oracle.h(n-1)
            controls = list(range(n-1))
            oracle.mcx(controls, n-1)
            oracle.h(n-1)

        num_runs = [1,2,3,4,5,6,7,8,9,10]
        shots = [100,200,300,400,500,600,700,800,900,1000]
        best_score = 0

        def is_in_good_subspace(s,x):
            count = 0
            for i in range(len(s)):
                if int(s[i]) == 1:
                    count += 1
            if count == len(s):
                return 1
            else:
                return 0

        total_num_qubits = n
        score_qubits = list(range(n))

        num_evaluation_qubits = 5
        num_state_prep_qubits = n
        num_trial_qubits = n 
        precision_qubits = list(range(n,n+num_evaluation_qubits))
        trial_qubits = list(range(n))

        # Execute:
        data = []
        indicator = []
        if method == 'mlqae':
            for i in num_runs:
                for j in shots:
                    result = run_MLQAE(state_prep, oracle, is_in_good_subspace, score_qubits, total_num_qubits, i, j, qpu = "qsim")
                    result_dict = ast.literal_eval(result)
                    amplitude_estimate = float(result_dict['AcceleratorBuffer']['Information']['amplitude-estimation'])
                    if abs(amplitude_estimate - np.sqrt(p)) <= 0.001:
                        indicator.append(1)
                    else:
                        indicator.append(0)
                    data_val = [i,j,amplitude_estimate]
                    data.append(data_val)
                    print(n, data_val)

            # Plot results
            fig = plt.figure()
            ax = plt.axes(projection ='3d')
            x = [data[i][0] for i in range(len(data))]
            y = [data[i][1] for i in range(len(data))]
            z = [data[i][2] for i in range(len(data))]
            colours = np.array(["red", "green"])
            ax.scatter(x,y,z,c=colours[indicator])
            lengthx = len(x)
            lengthy = len(y)
            X,Y = np.meshgrid(x,y)
            actual_amplitude = np.array([np.sqrt(p)]*(lengthx*lengthy)).reshape(lengthx,lengthy)
            ax.plot_surface(X,Y,actual_amplitude,color='green',alpha=0.1)
            ax.set_title(str(n) + " qubit MLQAE")
            ax.set_xlabel("num_runs")
            ax.set_ylabel("num_shots")
            ax.set_zlabel("Amplitude Estimate")
            name = "MLQAE_parameter_test_" + str(n) + "_qubits.png"
            plt.savefig(name)
            print(str(n) + " qubit plot made!")
            n+=1
        if method == 'cqae':
            result = run_canonical_ae_with_oracle(state_prep,oracle,num_evaluation_qubits,num_state_prep_qubits, num_trial_qubits,precision_qubits, trial_qubits)
            result_dict = ast.literal_eval(result)
            amplitude_estimate = float(result_dict['AcceleratorBuffer']['Information']['amplitude-estimation'])
            print(n, amplitude_estimate)
            n+=1