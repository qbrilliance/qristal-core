import numpy as np
import math as m
import sys

def read_jacobians(filename):
    jacobians = []
    with open(filename, 'r') as file:
        for line in file:
            # Replace brackets and ensure separated by ', '
            cleaned = line.strip().replace('] [', ', ').replace('[', '').replace(']', '')
            as_list = [float(x) for x in cleaned.split(', ')]
            jacobians.append(as_list)
    # jacobians is a list of lists for the partial derivatives of each
    # bit string for each parameter. Consolidate the probability of each
    # bitstring (wrt to each parameter) into its own list so the standard
    # deviation can be calculated. This is a simple transpose :)
    return np.transpose(np.array(jacobians))

def calculate_circuit_jacobians_analytically():
    alpha = m.pi/3
    beta = 2*m.pi/7

    dP_00_by_d_alpha = -0.5*m.sin(alpha)*m.pow(m.cos(beta/2.0),2.0)
    dP_10_by_d_alpha = 1/2*m.sin(alpha)*m.pow(m.cos(beta/2),2)
    dP_01_by_d_alpha = -1/2*m.sin(alpha)*m.pow(m.sin(beta/2),2)
    dP_11_by_d_alpha = 1/2*m.sin(alpha)*m.pow(m.sin(beta/2),2)
    dP_00_by_d_beta = -1/2*m.sin(beta)*m.pow(m.cos(alpha/2),2)
    dP_10_by_d_beta = -1/2*m.sin(beta)*m.pow(m.sin(alpha/2),2)
    dP_01_by_d_beta = 1/2*m.sin(beta)*m.pow(m.cos(alpha/2),2)
    dP_11_by_d_beta = 1/2*m.sin(beta)*m.pow(m.sin(alpha/2),2)

    return np.array([dP_00_by_d_alpha,
        dP_10_by_d_alpha,
        dP_01_by_d_alpha,
        dP_11_by_d_alpha,
        dP_00_by_d_beta,
        dP_10_by_d_beta,
        dP_01_by_d_beta,
        dP_11_by_d_beta
    ])

if __name__ == "__main__":
    filename = sys.argv[1]
    jacobians = read_jacobians(filename)
    std_devs = np.std(jacobians, axis=1)
    means = np.mean(jacobians, axis=1)
    analytically_calculated_values = calculate_circuit_jacobians_analytically()

    np.set_printoptions(linewidth=150)
    print("Qristal Jacobian calculation sample set for circuit")
    print(f"std devs:                       {std_devs}")
    print(f"means:                          {means}")
    print(f"analytically calculated values: {analytically_calculated_values}")
    print(f"error (raw):                    {(analytically_calculated_values - means)}")
    print(f"error (factor of std dev):      {(analytically_calculated_values - means)/std_devs}")
