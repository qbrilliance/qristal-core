# Copyright (c) 2022 Quantum Brilliance Pty Ltd
import os
import pytest

def test_AWS_Braket_hosted():
    print("* test_AWS_braket_hosted: A series of simple tests of AWS Braket with a 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    from braket.circuits import Circuit
    import boto3

    aws_account_id = boto3.client("sts").get_caller_identity()["Account"]
    my_bucket = "amazon-braket-qbsdk-2023"
    my_prefix = "simple_aws"
    s3_folder = (my_bucket,my_prefix)

    bell = Circuit().h(0).cnot(control=0,target=1)
    for sim in ['sv1', 'dm1', 'tn1']: #'Rigetti]: Rigetti commented out as devices are not available on Braket at the time of writing
      device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/"+sim)
      result = device.run(bell, s3_folder, shots = 256).result()
      counts = result.measurement_counts
      assert (counts['11'] + counts['00']) == 256,  "Failed test for "+sim
      assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "Failed test for "+sim

def test_AWS_Braket_qristal():
    print("* test_AWS_braket_qristal: Calling AWS Braket asynchronously from Qristal with a 2-qubit Bell state.")
    import qb.core
    import time
    from yaml import safe_load, dump

    # Start a QB SDK session
    s = qb.core.session()
    s.qn = 2
    shots = (64, 256, 512)
    s.sn[0] = qb.core.VectorSize_t(shots)
    s.aws_setup(32)
    # Set the input circuit
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 3.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[0] -> c[0];
        measure q[1] -> c[1];
    }'''
    # Load the remote_backends.yaml file entry
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["aws-braket"]
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    # Repeat for all backends
    for device in ["SV1", "DM1", "TN1"]: #"Rigetti"]) // Rigetti commented out as devices are not available on Braket at the time of writing.
      print("Testing "+device)

      # Change the remote_backends.yaml file entry
      db["device"] = device
      stream = open(s.remote_backend_database_path, 'w')
      dump({'aws-braket': db}, stream)

      # Launch asynchronous tasks now
      task = []
      for i in range(len(shots)):
          task.append(s.run_async(0, i))

      # Let all asynchronous tasks run to completion
      print(f'{len(shots)} asynchronous tasks launched.')
      for i in range(len(shots)):
          while not task[i].complete():
              time.sleep(1)
          print(f'Task {i} complete.')

      # Now show the finished results
      for i in range(len(shots)):
          assert s.results[0][i][(0,0)] + s.results[0][i][(1,1)] == shots[i]
