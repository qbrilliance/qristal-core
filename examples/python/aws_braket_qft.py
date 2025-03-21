print("Executing qft on AWS braket in Python...")
import qristal.core
import time
from yaml import safe_load, dump

# Import the core of Qristal
s = qristal.core.session()

# Set up meaningful defaults for session parameters
s.aws_setup(2)

# Running on aws braket requires user to have AWS account set up (e.g., via CLI)
# AWS account needs to have Braket enabled and in regions that support Braket (e.g., us-east-1)
# AWS account needs to have S3 access and can create S3 Buckets with prefix "amazon-braket-*"
# Set s.aws_s3 to be user's AWS S3 Bucket name, below is an example
# Set s.aws_s3_path to be user's AWS S3 Bucket folder name, below is an example

# Change the remote_backends.yaml file entry or user can call set_remote_backend_database_path
# to directly specify backend options
stream = open(s.remote_backend_database_path, 'r')
db = safe_load(stream)["aws-braket"]
db["device"] = "DM1"
db["path"] = "dm1-async"
stream = open(s.remote_backend_database_path + ".temp", 'w')
dump({'aws-braket': db}, stream)
s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

# Other session settings
s.xasm = True
s.noise = True
num_qubits = 4
s.qn = num_qubits

# Set up 2 jobs with the same circuit but different number of shots
s.sn[0].clear()
s.sn[0].append(64)
s.sn[0].append(256)

# Define the quantum program to run
# Use Qristal circuit builder to construct a QFT circuit
circ = qristal.core.Circuit()
circ.qft(range(num_qubits))
circ.measure_all()
s.ir_target = circ

# run_async(0, 0) submits the job with 64 shots asynchronously
# run_async(0, 1) submits the job with 256 shots asynchronously
handles = [s.run_async(0, 0), s.run_async(0, 1)]

# Check if jobs are completed
for h in handles:
  while (not h.complete()):
      time.sleep(1)

print("Output for job 1:")
print(s.results[0][0])
print("Output for job 2:")
print(s.results[0][1])
