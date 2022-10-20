# Copyright (c) 2021 Quantum Brilliance Pty Ltd
import numpy as np
import pandas as pd
from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt

from pylab import figure, show, legend, ylabel
import math
import matplotlib
from matplotlib import pyplot

def png(rho, rho_std, output_filename = "Set by providing: output_filename"):
    mag = np.abs(rho)
    mag_array = np.array(mag)
    arrays = [['< 0 |','< 1 |','< 0 |','< 1 |'],['| 1 >', '| 1 >', '| 0 >', '| 0 >']]
    tuples = list(zip(*arrays))
    index = pd.MultiIndex.from_tuples(tuples, names=['two', 'three'])         

    df = pd.DataFrame({'one': [mag_array[0], mag_array[1], mag_array[2] , mag_array[3]]}, index=index)

    # Set plotting style
    plt.style.use('seaborn-white')

    L = []
    for i, group in df.groupby(level=1)['one']:
        L.append(group.values)
    z = np.hstack(L).ravel()

    xlabels = df.index.get_level_values('two').unique()
    ylabels = df.index.get_level_values('three').unique()
    x = np.arange(xlabels.shape[0])
    y = np.arange(ylabels.shape[0])

    x_M, y_M = np.meshgrid(x, y, copy=False)
    fig = plt.figure(figsize=(5, 5))
    ax = fig.add_subplot(111, projection='3d')

    ax.w_xaxis.set_ticks(x + 0.5/2.)
    ax.w_yaxis.set_ticks(y + 0.5/2.)

    ax.w_xaxis.set_ticklabels(xlabels, fontsize=12)
    ax.w_yaxis.set_ticklabels(ylabels, fontsize=12)

    ax.set_zlim3d(0,1)

    # Choosing the range of values to be extended in the set colormap
    values = np.linspace(0.2, 1, x_M.ravel().shape[0])


    # Selecting an appropriate colormap
    colors = plt.cm.Spectral(values)
    ax.bar3d(x_M.ravel(), y_M.ravel(), z*0, dx=0.5, dy=0.5, dz=z, color=colors, alpha=0.5, edgecolor="black",  linewidth=0.35 )

    # Erro bars
    X_e= np.array(x_M.ravel())
    Y_e= np.array([1,1,0,0])
    
    for i in np.arange(0, 4):
        zlolims = mag_array[i]-rho_std[i]
        zuplims =  mag_array[i]+rho_std[i]
        ax.plot([X_e[i]+0.5,X_e[i]+0.5] ,[Y_e[i],Y_e[i]], [zlolims, zuplims], marker="_",color='maroon', linewidth=2 )

    plt.savefig(output_filename+'.png')

    plt.show()





