# Copyright (c) 2022 Quantum Brilliance Pty Ltd
from pylab import figure, show, legend, ylabel
import numpy as np
import math
import matplotlib
import matplotlib.pyplot as plt
from matplotlib import pyplot

def png(qn, execution_time, classical_execution_time, fidelity, classical_fidelity, fidelity_std, classical_fidelity_std = 0.0, in_dpi = 300, in_title = "Set by providing: in_title", in_data_name = "Set by providing: in_data_name"):
    if classical_fidelity_std == 0.0 :
        relative_precision = 1-np.array(fidelity_std)  
    else :
        relative_precision = np.divide(1-np.array(fidelity_std), 1-np.array(classical_fidelity_std))

    fig, ax = plt.subplots(figsize=(10, 6))
    plt.axis([0.001, 100, 0.65, 0.95])

    x = np.divide(np.array(classical_execution_time), np.array(execution_time))  # relative speed
    y = np.divide(np.array(fidelity), np.array(classical_fidelity))   # relative accuracy
    props = dict(boxstyle='square', facecolor='white', linewidth=0.0, alpha=1.0)

    matplotlib.rc('xtick', labelsize=12) 
    matplotlib.rc('ytick', labelsize=12)

    ax.fill_between([0.001,1],1.5,1, hatch='/', color='grey', alpha=0.1)
    ax.fill_between([1,1000],1.5,1, hatch='X', color='grey', alpha=1.0)
    ax.fill_between([1,1000],1,0.25, hatch='\\', color='grey' ,alpha=0.1)

    # Prepare a list of sizes that increases with values in fidelity
    sizevalues = [((8*i)**2) for i in relative_precision]
    plt.scatter(x, y, marker = 's', c=relative_precision, s = sizevalues, cmap="gray_r", edgecolors='black', linewidth=0.5, alpha=1.0)
    plt.colorbar(label="Relative Precision", orientation="vertical")

    # Set plot title and axes labels
    ax.set( xlabel = "Relative Speed" ,
           ylabel = "Relative Accuracy")

    ax.set_title(in_title) 
    pyplot.yscale('linear')
    pyplot.xscale('log')

    # annotate the first and last circuit
    ax.annotate(qn[0],
        xy=(x[0], y[0]), xycoords='data',
        xytext=(x[0], y[0]+0.04), textcoords='data', fontsize=10, bbox=props
            )
    ax.annotate(qn[len(qn)-1],
        xy=(x[len(qn)-1], y[len(qn)-1]), xycoords='data',
        xytext=(x[len(qn)-1], y[len(qn)-1]+0.04), textcoords='data', fontsize=10, bbox=props
            )

    # changing x and y text axis
    position_x = (0.001,0.01,0.1,1,10,100,1000)
    label_x = ((10)**-3,0.01,0.1,1,10,100,1000)
    plt.xticks(position_x, label_x)
    position_y = (1.5,1,0.5,0.25)
    label_y =(1.5,1,0.5,0.25)
    plt.yticks(position_y, label_y)
    ax.grid(True, which='both', linestyle = '--', linewidth = 0.5)
    plt.vlines(1.0,0.001,20)
    plt.hlines(1.0,1e-8,1e8)

    ax.annotate("Region of Quantum Speed Up \nand Accuracy Gain",(1.0,1.0),(1.5,1.1),bbox=props, fontsize = 9)
    ax.annotate("Region of Quantum Speed Up",(1.0,0.3),(1.5,0.3),bbox=props, fontsize = 9)
    ax.annotate("Region of Quantum Accuracy Gain",(0.0015,1.0),(0.0015,1.1),bbox=props, fontsize = 9)

    ax.annotate(in_data_name,
        xy=(x[len(qn)-1], y[len(qn)-1]), xycoords='data',
        xytext=(x[len(qn)-1]+ 4, y[len(qn)-1]), textcoords='data', fontsize=10,bbox=props)

    plt.savefig(in_data_name+'_'+in_title+'_quantum_utility'+'.png', dpi=in_dpi)
    plt.show()