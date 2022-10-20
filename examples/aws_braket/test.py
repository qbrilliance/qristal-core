import boto3
from braket.aws import AwsDevice

import matplotlib.pyplot as plt
import numpy as np

from braket.circuits import Circuit
from braket.devices import LocalSimulator

from time import time


# get the account ID
aws_account_id = boto3.client("sts").get_caller_identity()["Account"]
# the name of the bucket
#my_bucket = f"example-bucket"
my_bucket = "amazon-braket-qbos-2022"
# the name of the folder in the bucket
my_prefix = "guest"
s3_folder = (my_bucket, my_prefix)

with open("/mnt/qb/qbos/src/qbos_core/src/build/plugins/aws_braket/tests/AWS_circuit.txt") as f:
    circ_tmp = f.readlines()


circ = circ_tmp[0]

print("--------- C I R C U I T ----------------")
print()
print(eval(circ))
print()
print("--------- C I R C U I T ----------------")

#device = LocalSimulator()
# choose the cloud-based managed simulator to run your circuit
device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/sv1")
#device = AwsDevice("arn:aws:braket:::device/qpu/rigetti/Aspen-11")
#device = AwsDevice("arn:aws:braket:::device/qpu/ionq/ionQdevice")

#print(device.properties.dict())

#result = device.run(eval(circ), s3_folder, shots=1000)

#send 10 tasks to SV1

n_tasks = 1

tasks = [i for i in range(n_tasks)]
results = tasks

t1 = time()

for idx in range(n_tasks):
    tasks[idx] = device.run(eval(circ), s3_folder, shots=1000)
    print('{}-ranked task {}'.format(idx, tasks[idx].state()))

t2 = time()

for idx in range(n_tasks):
    results[idx] = tasks[idx].result()
    #print('{}-ranked task {}'.format(idx, tasks[idx].state()))
    print('Measurement result for task {}: {}'.format(idx, results[idx].measurement_counts))

for idx in range(n_tasks):
    with open('test_'+str(idx)+'.txt', 'w') as f:
        f.write(str(results[idx].measurement_counts))

#print("AWS ac id: ", aws_account_id)

print()
print()
#print(counts)
print()
print()
#print(result.additional_metadata.rigettiMetadata.compiledProgram)
