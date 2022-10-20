# AWS Braket offload in qbOS

# 1.0 Background

<aside>
➡️ New in qbOS version 2

</aside>

AWS users can use [AWS Braket](https://docs.aws.amazon.com/braket/index.html) as a backend for offloading the execution of quantum circuits within qbOS.  

<aside>
ℹ️ Basic knowledge of AWS is assumed here, in particular knowledge on generating an AWS Access Key and AWS Secret Access Key.

</aside>

Using this mechanism, you can run quantum circuits on these AWS services:

- [SV1](https://docs.aws.amazon.com/braket/latest/developerguide/braket-devices.html#braket-simulator-sv1) (AWS hosted, state-vector simulator)
- SV1 - limits
    
    ```bash
    34 qubits max. You can expect a 34-qubit, dense, and square circuit (circuit depth = 34) to take approximately 1 to 2 hours to complete, depending on the type of gates used and other factors. 
    SV1 has a maximum runtime of 6 hours. 
    35 concurrent tasks [max: 50]
    
    SV1 Results
    SV1 can provide the following results, given the specified number of shots:
    
    Sample: Shots > 0
    Expectation: Shots >= 0
    Variance: Shots >= 0
    Probability: Shots > 0
    Amplitude: Shots = 0
    ```
    

- [DM1](https://docs.aws.amazon.com/braket/latest/developerguide/braket-devices.html#braket-simulator-dm1) (AWS hosted, noise-model capable, density-matrix simulator)
- DM1 - limits
    
    <aside>
    ℹ️ qbOS only uses the Sample capability of DM1
    
    </aside>
    
    ```bash
    17 qubits max.
    Maximum runtime: 6 hours. 
    35 concurrent tasks [max: 50]
    
    DM1 Results: DM1 provides the following results, given the specified number of shots:
    Sample: Shots > 0
    Expectation: Shots >= 0
    Variance: Shots >= 0
    Probability: Shots > 0
    Reduced density matrix: Shots = 0, up to max 8 qubits
    ```
    

- [TN1](https://docs.aws.amazon.com/braket/latest/developerguide/braket-devices.html#braket-simulator-tn1) (AWS hosted, tensor-network simulator)
- TN1 - limits
    
    <aside>
    ℹ️ qbOS limits TN1 to 48 qubits to fit with the currently available topology model in QB hardware
    
    </aside>
    
    ```bash
      Number of shots [sn]: 0 < sn < 1000 (note: non-zero number of shots)
      Number of qubits[qn]: < 50
      Circuit depth < 1000
      Concurrent tasks < 10 (< 5 if in eu-west-2)
    ```
    

# 2.0 Setting up AWS Braket

To start using AWS Braket, follow the instructions here: 

[Enable Amazon Braket](https://docs.aws.amazon.com/braket/latest/developerguide/braket-enable-overview.html)

**Important notes:**

1. Use an AWS Region that supports AWS Braket (eg. `us-east-1`)
2. Your AWS User Account needs to have the AWS IAM Policy: [AmazonBraketFullAccess](https://us-east-1.console.aws.amazon.com/iam/home#/policies/arn%3Aaws%3Aiam%3A%3Aaws%3Apolicy%2FAmazonBraketFullAccess) 
3. Your AWS User Account must have S3 access, and is able to create a Bucket with the prefix: `amazon-braket-*` (eg. `amazon-braket-qbos`)
4. Within the S3 Bucket created at Step 3, create a path (folder) that will be used later in qbOS to store all measurement results.

<aside>
💡 **In the qbOS JupyterLab session,** remember to **configure your AWS credentials** before attempting to use AWS Braket.

To do this, open a Bash terminal inside a qbOS ver. 2 JupyterLab session and in there, run:

```bash
python3 -m pip install awscli
python3 -m pip install amazon-braket-sdk
aws configure
```

</aside>

# 3.0 Example setup

1. Launch a VM using an EC2 Launch Template for qbOS version 2 appropriate for your AWS Region - see: [1.5 Alpha preview of qbOS version 2](https://www.notion.so/1-5-Alpha-preview-of-qbOS-version-2-1576d05b478e427795da97d43de3f384) 
2. Login with SSH to the VM you launched above
3. Open JupyterLab on: `localhost:8889`
4. Open a Bash terminal inside JupyterLab, and configure your AWS credentials at that Bash terminal:
    - Details
        
        ```bash
        python3 -m pip install awscli
        python3 -m pip install amazon-braket-sdk
        aws configure
        # Make sure to have your AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY ready to be entered when prompted.  
        ```
        
        <aside>
        ℹ️ For AWS_REGION, set this to: `us-east-1` (or any other AWS Region that supports AWS Braket)
        
        </aside>
        
        <aside>
        ℹ️ For AWS_DEFAULT_OUTPUT, set this to: `json`
        
        </aside>
        
5. In AWS S3, create a Bucket with the name prefixed by: `amazon-braket-*`. For this example, we use: `amazon-braket-qbos-22`
    
    ![Untitled](AWS%20Braket%20offload%20in%20qbOS%204631d07005d74d5898dc4ee153199663/Untitled.png)
    
6. Run the example code shown in Section 4.0

# 4.0 Available attributes in qbOS related to AWS Braket

You can specify an AWS S3 bucket in which to store results from quantum tasks that have been executed on AWS Braket.  This is achieved by setting the following `qbos.core` attributes:

| Attribute | Details | Default value |
| --- | --- | --- |
| aws_s3 | S3 bucket name that will be storing the results from AWS Braket. | "amazon-braket-qbos" |
|  | Important: required name prefix = "amazon-braket-*" |  |
|  | Important: you must first create the S3 bucket in your own AWS credentials prior to using it here. |  |
| aws_s3_path | Path inside aws_s3 to keep the results from AWS Braket. | "output" |

# 5.0 Example code - Bell state

## 5.1 Using AWS Braket Python API directly - TN1 backend

- Details
    
    ```python
    import braket
    from braket.aws import AwsDevice
    from braket.circuits import Circuit
    import boto3
    import pytest
    aws_account_id = boto3.client("sts").get_caller_identity()["Account"]
    my_bucket = "amazon-braket-qbos-22"
    my_prefix = "smoke-tests"
    s3_folder = (my_bucket,my_prefix)
    #
    bell = Circuit().h(0).cnot(control=0,target=1)
    device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/tn1")
    result = device.run(bell, s3_folder, shots = 256).result()
    counts = result.measurement_counts
    #
    assert (counts['11'] + counts['00']) == 256,  "Failed test: sum of measurement counts"
    assert (counts['11'] / counts['00']) == pytest.approx(1.0, None, 0.8), "Failed test: odds ratio"
    ```
    

## 5.2 Using qbOS - TN1 backend

```python
import qbos
tqb = qbos.core()
tqb.aws8tn1()
tqb.sn[0].clear()
tqb.sn[0].append(64)
tqb.sn[0].append(256)

tqb.aws_s3 = 'amazon-s3-qbos-2022'
tqb.aws_s3_path = 'tn1-results'

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
}'''
tqb.run()

```

## 5.3 Using qbOS - SV1 backend

```python
import qbos
tqb = qbos.core()
tqb.aws32sv1()
tqb.sn[0].clear()
tqb.sn[0].append(64)
tqb.sn[0].append(256)

tqb.aws_s3 = 'amazon-s3-qbos-2022'
tqb.aws_s3_path = 'sv1-results'

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
}'''
tqb.run()

```

## 5.4 Using qbOS - DM1 backend

```python
import qbos
tqb = qbos.core()
tqb.aws32dm1()
tqb.sn[0].clear()
tqb.sn[0].append(64)
tqb.sn[0].append(256)

tqb.aws_s3 = 'amazon-s3-qbos-2022'
tqb.aws_s3_path = 'dm1-results'

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
}'''
tqb.run()

```

## 5.5 Using qbOS - TN1 and SV1, storing in different S3 paths

```bash
import qbos
tqb = qbos.core()
tqb.acc = 'aws_acc'
tqb.sn = 512

tqb.aws_device[0].clear()
tqb.aws_device[0].append("TN1")
tqb.aws_device[0].append("SV1")

tqb.aws_s3_path[0].clear()
tqb.aws_s3_path[0].append("tn1-results")
tqb.aws_s3_path[0].append("sv1-results")

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
}'''
tqb.run()

# Now show the results
tqb.out_raw[0]
```

## 5.6 Using qbOS asynchronous offload - DM1 backend, preset to 32 workers

```python
import qbos
import time

tqb = qbos.core()

# Setup AWS DM1 (with 32 workers)
tqb.aws32dm1()

# For TN1 (with 8 workers) use: 
# tqb.aws8tn1()

# For SV1 (with 32 workers) use: 
# tqb.aws32sv1()

tqb.aws_s3 = 'amazon-s3-qbos-2022'
tqb.aws_s3_path = 'dm1-results'

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[0];
        cx q[0],q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
}'''

# Setup 3 tasks
tqb.sn[0].clear()
tqb.sn[0].append(64)
tqb.sn[0].append(256)
tqb.sn[0].append(512)

# Launch asynchronous tasks now 
jtask_64 = tqb.run_async(0, 0)
jtask_256 = tqb.run_async(0, 1)
jtask_512 = tqb.run_async(0, 2)

# Let all asynchronous tasks run to completion
while (not jtask_64.complete()) :
    time.sleep(1)

while (not jtask_256.complete()) :
    time.sleep(1)

while (not jtask_512.complete()) :
    time.sleep(1)

# Now show the finished results
tqb.out_raw[0]

# Output:
# String[{
#     "00": 34,
#     "11": 30
# }, {
#     "00": 129,
#     "11": 127
# }, {
#     "00": 252,
#     "11": 260
# }]
```