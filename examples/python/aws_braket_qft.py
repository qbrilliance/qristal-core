print("Executing qft on AWS braket in Python...")
import qb.core
import time

# Import the core of the QB SDK
s = qb.core.session()

# Set up meaningful defaults for session parameters
s.qb12()

# Choose aws32dm1 backend
s.aws32dm1()

# Running on aws braket requires user to have AWS account set up (e.g., via CLI)
# AWS account needs to have Braket enabled and in regions that support Braket (e.g., us-east-1)
# AWS account needs to have S3 access and can create S3 Buckets with prefix "amazon-braket-*"
# Set s.aws_s3 to be user's AWS S3 Bucket name, below is an example
s.aws_s3 = 'amazon-braket-qbsdk-2023'
# Set s.aws_s3_path to be user's AWS S3 Bucket folder name, below is an example
s.aws_s3_path = 'dm1'

# Other session settings
s.xasm = True
s.noise = True
num_qubits = 4
s.qn = num_qubits
print("Backend chosen: ", s.acc)
print("Device from qft.py: ", s.aws_device)

# Set up 2 jobs with the same circuit but different number of shots
s.sn[0].clear()
s.sn[0].append(64)
s.sn[0].append(256)

# Define the quantum program to run
# Use Qristal circuit builder to construct a QFT circuit
circ = qb.core.Circuit()
circ.qft(range(num_qubits))
circ.measure_all()
s.ir_target = circ

# run_async(0, 0) submits the job with 64 shots asynchronously
# run_async(0, 1) submits the job with 256 shots asynchronously
handle1 = s.run_async(0, 0)
handle2 = s.run_async(0, 1)

# Check if jobs are completed
while (not handle1.complete()):
    time.sleep(1)

while (not handle2.complete()):
    time.sleep(1)

result1 = s.out_raw[0][0]
print("Output for job1: \n", result1)
result2 = s.out_raw[0][1]
print("Output for job2: \n", result2)
