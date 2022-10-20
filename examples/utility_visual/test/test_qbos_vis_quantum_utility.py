# Copyright (c) 2021 Quantum Brilliance Pty Ltd
import os
import pytest

import qbos_vis
import matplotlib.pyplot as plt
import numpy as np
import cv2
from mock import patch

def test_CI_211202_visualise_quantum_utility_1() :
    print("* CI_211202_visualise_quantum_utility_1:")
    print("* Imports qbos_vis and calls quantum_utility.plot.png(), which should match a reference .png image.")
    
    def mse(imageA, imageB):
        err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
        err /= float(imageA.shape[0] * imageA.shape[1])
        return err
        
    # DJ quantum utility - Expected results
    reference_plot = "reference_DJ_Feather-weight class_quantum_utlity.png"
    err_tolerance = 1.0e-6
    
    # Dataset
    qn = [25,26,27,28,29,30,31,32,33,34,35,36,37]
    exec_ms = [1065.0501120000001, 1106.010112, 1146.9701119999997, 1187.930112, 1228.8901120000003, 1269.850112, 1310.8101119999997, 1351.770112, 1392.7301119999997, 1433.690112, 1474.6501119999998, 1515.661312, 1556.6213119999998]
    classical_exec_ms = [1.7666937589645386, 3.534844160079956, 7.067774748802185, 14.136766242980958, 28.27072343826294, 57.18937656879425, 113.07382431030274, 222.7695102930069, 441.92586266994476, 879.6264528512954, 1756.0611808300018, 3507.879772925377, 7009.2622062206265]
    fidelity = [0.7802734375, 0.7662109375, 0.76123046875, 0.75673828125, 0.74365234375, 0.7369140625, 0.7341796875, 0.72734375, 0.71650390625, 0.7078125, 0.689453125, 0.6986328125, 0.701171875]
    classical_fidelity = len(qn)*[1]
    fidelity_std = [0.014036705944399179, 0.016023959340600524, 0.010744406730991552, 0.011827699923215536, 0.010876731997718584, 0.015571196428120897, 0.016947228597293015, 0.014763868116378327, 0.011952821814986631, 0.011569690015328036, 0.018138843379889365, 0.012621480016721493, 0.01620623049133515]
    classical_fidelity_std = len(qn)*[0]
    
    qbos_vis.quantum_utility.plot.png(qn, exec_ms, classical_exec_ms, fidelity, classical_fidelity, fidelity_std, classical_fidelity_std, in_title="Feather-weight class", in_data_name="DJ")
    DJ_ref = cv2.imread("reference_DJ_Feather-weight class_quantum_utility.png")
    DJ_test = cv2.imread("DJ_Feather-weight class_quantum_utility.png")
    DJ_ref = cv2.cvtColor(DJ_ref, cv2.COLOR_BGR2GRAY) 
    DJ_test = cv2.cvtColor(DJ_test, cv2.COLOR_BGR2GRAY)
    assert (mse(DJ_ref,DJ_test ) < err_tolerance, "[qbos] Failed test: CI_211202_visualise_quantum_utility_1")
