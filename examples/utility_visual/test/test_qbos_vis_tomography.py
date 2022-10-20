# Copyright (c) 2021 Quantum Brilliance Pty Ltd
import os
import pytest


import qbos_vis
import matplotlib.pyplot as plt
import numpy as np
import cv2
from mock import patch


def test_CI_211202_visualise_tomography_1() :
    print("* CI_211202_visualise_tomography_1:")
    print("* Imports qbos_vis and calls tomography.plot.png(), which should match a reference .png image.")

    def mse(imageA, imageB):
        err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
        err /= float(imageA.shape[0] * imageA.shape[1])
        return err

    # Tomography - Expected results
    reference_plot = "reference_tomography.png"
    err_tolerance = 10000
    
    # Dataset
    rho= [9.8987e-01+0.j, 3.3000e-04-0.00188j, 3.3000e-04+0.00188j,1.0130e-02+0.j]
    rho_std=[0.0016862013521522288, 0.021686094623052828, 0.021686094623052828, 0.0016862013521522288]
    
    qbos_vis.tomography.plot.png(rho,rho_std,"tomo")
    
    tomo_ref = cv2.imread(reference_plot)
    tomo_test = cv2.imread("tomo.png")
    
    tomo_ref = cv2.cvtColor(tomo_ref, cv2.COLOR_BGR2GRAY) 
    tomo_test = cv2.cvtColor(tomo_test, cv2.COLOR_BGR2GRAY)
    
    print('mmmmmmmssssseeee',mse(tomo_ref,tomo_test ))
    assert mse(tomo_ref,tomo_test )<= err_tolerance, "[qbos] Failed test: CI_211202_visualise_tomography_1"
    
    print('SUCCESS: tomo plot matches reference image')






