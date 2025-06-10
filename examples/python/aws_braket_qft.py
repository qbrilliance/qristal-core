print("Executing qft on AWS braket in Python...")
import qristal.core
import time
from yaml import safe_load, dump

num_qubits = 4

# Make two sessions
s = [qristal.core.session(), qristal.core.session()]

# Running on aws braket requires user to have AWS account set up (e.g., via CLI)
# AWS account needs to have Braket enabled and in regions that support Braket (e.g., us-east-1)
# AWS account needs to have S3 access and can create S3 Buckets with prefix "amazon-braket-*"
# Set s3 to be user's AWS S3 Bucket name, below is an example
# Set path to be user's AWS S3 Bucket folder name, below is an example

# Change the remote_backends.yaml file entry or user can call set_remote_backend_database_path
# to directly specify backend options
for i in range(2):
  stream = open(s[i].remote_backend_database_path, 'r')
  db = safe_load(stream)["aws-braket"]
  db["device"] = "DM1"
  db["s3"] = "amazon-braket-qbsdk-2023"
  db["path"] = "dm1-async-" + str(i)
  stream = open(s[i].remote_backend_database_path + ".temp." + str(i), 'w')
  dump({'aws-braket': db}, stream)
  s[i].remote_backend_database_path = s[i].remote_backend_database_path + ".temp." + str(i)

# Define the quantum program to run
# Use Qristal circuit builder to construct a QFT circuit
circ = qristal.core.Circuit()
circ.qft(range(num_qubits))
circ.measure_all()

# Other session settings
for ses in s:
  ses.input_language = qristal.core.circuit_language.XASM
  ses.noise = True
  ses.qn = num_qubits
  ses.irtarget = circ
  ses.nooptimise = True
  ses.acc = "aws-braket"

# Set up 2 jobs with the same circuit but different number of shots
s[0].sn = 64
s[1].sn = 256


# Submit the two jobs asynchronously
handles = [s[0].run_async(), s[1].run_async()]

# Check if jobs are completed
for i in range(2):
  while (not handles[i].complete()):
      time.sleep(1)
  print("Output for job " + str(i+1) + " (async):")
  print(handles[i].get())

# Submit the two jobs synchronously
for ses in s:
  ses.run()

print("Output for job 1 (synchronous):")
print(s[0].results)
print("Output for job 2 (synchronous):")
print(s[1].results)
