# Copyright (c) Quantum Brilliance Pty Ltd
import os
import pytest
def test_CI_220531_1_simple_aws_sv_local():
    print("* CI_220531_1_simple_aws_sv_local:")
    print("* A simple test of AWS Braket with SV running locally, 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    import numpy as np
    from braket.circuits import Circuit
    from braket.devices import LocalSimulator
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = LocalSimulator()
    result = device.run(bell, shots = 1000).result()
    counts = result.measurement_counts
    assert (counts['11'] + counts['00']) == 1000,  "Failed test: CI_220531_1_simple_aws_sv_local"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "Failed test: CI_220531_1_simple_aws_sv_local"

@pytest.mark.filterwarnings("ignore:You are running a noise-free circuit on the density matrix simulator.")
def test_CI_220531_2_simple_aws_dm_local():
    print("* CI_220531_2_simple_aws_dm_local:")
    print("* A simple test of AWS Braket with DM running locally, 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    import numpy as np
    from braket.circuits import Circuit
    from braket.devices import LocalSimulator
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = LocalSimulator(backend="braket_dm")
    result = device.run(bell, shots = 1000).result()
    counts = result.measurement_counts
    assert (counts['11'] + counts['00']) == 1000,  "Failed test: CI_220531_2_simple_aws_dm_local"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "Failed test: CI_220531_2_simple_aws_dm_local"


