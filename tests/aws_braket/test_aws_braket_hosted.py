# Copyright (c) Quantum Brilliance Pty Ltd
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
    import qristal.core
    import time
    import copy
    from yaml import safe_load, dump

    # Shot numbers to run
    shots = (64, 256, 512)

    # The input circuit
    circuit = '''
    __qpu__ void qristal_circuit(qreg q) {
        OPENQASM 3.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[0] -> c[0];
        measure q[1] -> c[1];
    }'''

    # Loop over all backends
    for device in ["SV1", "DM1", "TN1"]: #"Rigetti"]) // Rigetti commented out as devices are not available on Braket at the time of writing.
      print("Testing "+device)

      tasks = []
      sessions = []
      for i in range(len(shots)):
          sessions.append(qristal.core.session())
          sessions[i].qn = 2
          sessions[i].acc = "aws-braket"
          sessions[i].instring = circuit
          sessions[i].sn = shots[i]

          # Load the remote_backends.yaml file entry
          stream = open(sessions[i].remote_backend_database_path, 'r')
          db = safe_load(stream)["aws-braket"]
          sessions[i].remote_backend_database_path = sessions[i].remote_backend_database_path + "." + device + "." + str(i)

          # Change the remote_backends.yaml file entry
          db["device"] = device
          stream = open(sessions[i].remote_backend_database_path, 'w')
          dump({'aws-braket': db}, stream)

          tasks.append(sessions[i].run_async())

      # Let all asynchronous tasks run to completion
      print(f'{len(shots)} asynchronous tasks launched.')
      for i in range(len(shots)):
          while not tasks[i].complete():
              time.sleep(1)
          print(f'Task {i} complete.')

      # Now show the finished results
      for i in range(len(shots)):
          assert sessions[i].results[(0,0)] + sessions[i].results[(1,1)] == shots[i]
