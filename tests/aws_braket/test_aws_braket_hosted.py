# Copyright (c) 2022 Quantum Brilliance Pty Ltd
import os
import pytest
def test_CI_220601_1_simple_aws_sv1():
    print("* CI_220601_1_simple_aws_sv1:")
    print("* A simple test of AWS Braket with SV1, 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    from braket.circuits import Circuit
    import boto3
    aws_account_id = boto3.client("sts").get_caller_identity()["Account"]

    my_bucket = "amazon-braket-qbsdk-2023"
    my_prefix = "simple_aws"
    s3_folder = (my_bucket,my_prefix)
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/sv1")
    result = device.run(bell, s3_folder, shots = 256).result()
    counts = result.measurement_counts
    assert (counts['11'] + counts['00']) == 256,  "[QB SDK] Failed test: CI_220601_1_simple_aws_sv1"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "[QB SDK] Failed test: CI_220601_1_simple_aws_sv1"

def test_CI_220601_2_simple_aws_dm1():
    print("* CI_220601_2_simple_aws_dm1:")
    print("* A simple test of AWS Braket with DM1, 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    from braket.circuits import Circuit
    import boto3
    aws_account_id = boto3.client("sts").get_caller_identity()["Account"]

    my_bucket = "amazon-braket-qbsdk-2023"
    my_prefix = "simple_aws"
    s3_folder = (my_bucket,my_prefix)
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/dm1")
    result = device.run(bell, s3_folder, shots = 256).result()
    counts = result.measurement_counts
    assert (counts['11'] + counts['00']) == 256,  "[QB SDK] Failed test: CI_220601_2_simple_aws_dm1"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "[QB SDK] Failed test: CI_220601_2_simple_aws_dm1"


def test_CI_220602_3_simple_aws_tn1():
    print("* CI_220602_3_simple_aws_tn1:")
    print("* A simple test of AWS Braket with TN1, 2-qubit Bell state.")
    import braket
    from braket.aws import AwsDevice
    from braket.circuits import Circuit
    import boto3
    aws_account_id = boto3.client("sts").get_caller_identity()["Account"]

    my_bucket = "amazon-braket-qbsdk-2023"
    my_prefix = "simple_aws"
    s3_folder = (my_bucket,my_prefix)
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/tn1")
    result = device.run(bell, s3_folder, shots = 256).result()
    counts = result.measurement_counts
    assert (counts['11'] + counts['00']) == 256,  "[QB SDK] Failed test: CI_220602_3_simple_aws_tn1"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "[QB SDK] Failed test: CI_220602_3_simple_aws_tn1"

def test_CI_220616_1_aws_dm1_async():
    print("* CI_220616_1_aws_dm1_async:")
    print("* Asynchronous operation - offload to AWS DM1, 2-qubit Bell state.")
    import qb.core, ast
    import json
    import time
    from yaml import safe_load, dump

    s = qb.core.session()
    s.qb12()
    s.aws32dm1()
    # Change the remote_backends.yaml file entry
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["aws_braket"]
    db["device"] = "DM1"
    db["path"] = "dm1-async"
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'aws-bracket': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    s.sn[0].clear()
    s.sn[0].append(64)
    s.sn[0].append(256)
    s.sn[0].append(512)
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 3.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''

    # Launch asynchronous tasks now
    jtask_64 = s.run_async(0, 0)
    jtask_256 = s.run_async(0, 1)
    jtask_512 = s.run_async(0, 2)

    # Let all asynchronous tasks run to completion
    while (not jtask_64.complete()) :
        time.sleep(1)

    while (not jtask_256.complete()) :
        time.sleep(1)

    while (not jtask_512.complete()) :
        time.sleep(1)

    # Now show the finished results
    result1 = s.out_raw[0][0]
    result2 = s.out_raw[0][1]
    result3 = s.out_raw[0][2]
    res1 = ast.literal_eval(result1)
    res2 = ast.literal_eval(result2)
    res3 = ast.literal_eval(result3)
    assert (res1[''.join(reversed(bin(0)[2:].zfill(2)))] + res1[''.join(reversed(bin(3)[2:].zfill(2)))]) == 64,  "[QB SDK] Failed test: CI_220616_1_aws_dm1_async"
    assert (res2[''.join(reversed(bin(0)[2:].zfill(2)))] + res2[''.join(reversed(bin(3)[2:].zfill(2)))]) == 256, "[QB SDK] Failed test: CI_220616_1_aws_dm1_async"
    assert (res3[''.join(reversed(bin(0)[2:].zfill(2)))] + res3[''.join(reversed(bin(3)[2:].zfill(2)))]) == 512, "[QB SDK] Failed test: CI_220616_1_aws_dm1_async"

def test_CI_220616_2_aws_sv1_async():
    print("* CI_220616_2_aws_sv1_async:")
    print("* Asynchronous operation - offload to AWS SV1, 2-qubit Bell state.")
    import qb.core, ast
    import json
    import time
    from yaml import safe_load, dump
    #
    s = qb.core.session()
    s.qb12()
    s.aws32sv1()
    # Change the remote_backends.yaml file entry
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["aws-braket"]
    db["device"] = "SV1"
    db["path"] = "sv1-async"
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'aws-braket': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    s.sn[0].clear()
    s.sn[0].append(64)
    s.sn[0].append(256)
    s.sn[0].append(512)
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 3.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''

    # Launch asynchronous tasks now
    jtask_64 = s.run_async(0, 0)
    jtask_256 = s.run_async(0, 1)
    jtask_512 = s.run_async(0, 2)

    # Let all asynchronous tasks run to completion
    while (not jtask_64.complete()) :
        time.sleep(1)

    while (not jtask_256.complete()) :
        time.sleep(1)

    while (not jtask_512.complete()) :
        time.sleep(1)

    # Now show the finished results
    result1 = s.out_raw[0][0]
    result2 = s.out_raw[0][1]
    result3 = s.out_raw[0][2]
    res1 = ast.literal_eval(result1)
    res2 = ast.literal_eval(result2)
    res3 = ast.literal_eval(result3)
    assert (res1[''.join(reversed(bin(0)[2:].zfill(2)))] + res1[''.join(reversed(bin(3)[2:].zfill(2)))]) == 64,  "[QB SDK] Failed test: CI_220616_2_aws_sv1_async"
    assert (res2[''.join(reversed(bin(0)[2:].zfill(2)))] + res2[''.join(reversed(bin(3)[2:].zfill(2)))]) == 256, "[QB SDK] Failed test: CI_220616_2_aws_sv1_async"
    assert (res3[''.join(reversed(bin(0)[2:].zfill(2)))] + res3[''.join(reversed(bin(3)[2:].zfill(2)))]) == 512, "[QB SDK] Failed test: CI_220616_2_aws_sv1_async"

def test_CI_220616_3_aws_tn1_async():
    print("* CI_220616_3_aws_tn1_async:")
    print("* Asynchronous operation - offload to AWS TN1, 2-qubit Bell state.")
    import qb.core, ast
    import json
    import time
    from yaml import safe_load, dump
    
    s = qb.core.session()
    s.qb12()
    s.aws8tn1()
    # Change the remote_backends.yaml file entry
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["aws-braket"]
    db["device"] = "TN1"
    db["path"] = "tn1-async"
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'aws-braket': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    s.sn[0].clear()
    s.sn[0].append(64)
    s.sn[0].append(256)
    s.sn[0].append(512)
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''

    # Launch asynchronous tasks now
    jtask_64 = s.run_async(0, 0)
    jtask_256 = s.run_async(0, 1)
    jtask_512 = s.run_async(0, 2)

    # Let all asynchronous tasks run to completion
    while (not jtask_64.complete()) :
        time.sleep(1)

    while (not jtask_256.complete()) :
        time.sleep(1)

    while (not jtask_512.complete()) :
        time.sleep(1)

    # Now show the finished results
    result1 = s.out_raw[0][0]
    result2 = s.out_raw[0][1]
    result3 = s.out_raw[0][2]
    res1 = ast.literal_eval(result1)
    res2 = ast.literal_eval(result2)
    res3 = ast.literal_eval(result3)
    assert (res1[''.join(reversed(bin(0)[2:].zfill(2)))] + res1[''.join(reversed(bin(3)[2:].zfill(2)))]) == 64,  "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"
    assert (res2[''.join(reversed(bin(0)[2:].zfill(2)))] + res2[''.join(reversed(bin(3)[2:].zfill(2)))]) == 256, "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"
    assert (res3[''.join(reversed(bin(0)[2:].zfill(2)))] + res3[''.join(reversed(bin(3)[2:].zfill(2)))]) == 512, "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"

def test_CI_220908_2_aws_tn1_async_s3_s3_prefix():
    print("* CI_220908_2_aws_tn1_async_s3_s3_prefix:")
    print("* Asynchronous operation - offload to AWS TN1, 2-qubit Bell state, specify s3_prefix.")
    import qb.core, ast
    import json
    import time
    from yaml import safe_load, dump

    s = qb.core.session()
    s.qb12()
    s.aws8tn1()
    
    # Change the remote_backends.yaml file entry
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["aws-braket"]
    db["device"] = "TN1"
    db["path"] = "tn1-async"
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'aws-braket': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    s.sn[0].clear()
    s.sn[0].append(64)
    s.sn[0].append(256)
    s.sn[0].append(512)
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 3.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''


    # Launch asynchronous tasks now
    jtask_64 = s.run_async(0, 0)
    jtask_256 = s.run_async(0, 1)
    jtask_512 = s.run_async(0, 2)

    # Let all asynchronous tasks run to completion
    while (not jtask_64.complete()) :
        time.sleep(1)

    while (not jtask_256.complete()) :
        time.sleep(1)

    while (not jtask_512.complete()) :
        time.sleep(1)

    # Now show the finished results
    result1 = s.out_raw[0][0]
    result2 = s.out_raw[0][1]
    result3 = s.out_raw[0][2]
    res1 = ast.literal_eval(result1)
    res2 = ast.literal_eval(result2)
    res3 = ast.literal_eval(result3)
    assert (res1[''.join(reversed(bin(0)[2:].zfill(2)))] + res1[''.join(reversed(bin(3)[2:].zfill(2)))]) == 64,  "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"
    assert (res2[''.join(reversed(bin(0)[2:].zfill(2)))] + res2[''.join(reversed(bin(3)[2:].zfill(2)))]) == 256, "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"
    assert (res3[''.join(reversed(bin(0)[2:].zfill(2)))] + res3[''.join(reversed(bin(3)[2:].zfill(2)))]) == 512, "[QB SDK] Failed test: CI_220616_3_aws_tn1_async"
