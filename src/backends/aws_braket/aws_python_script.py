import boto3
import botocore
from braket.aws import AwsDevice
from braket.circuits import Circuit
from braket.ir.openqasm import Program
import threading

# mutex used when creating boto clients 
# Note: boto3.client constructor is not thread safe:
# https://github.com/boto/boto3/issues/801
BOTO_CLIENT_CREATE_LOCK = threading.RLock()

def run_aws_braket_async(aws_device, sn, in_string, in_verbatim, in_format, out_s3, out_s3_path):
    """
        Submit a task to AWS Braket.

        Args:
            aws_device (String): Name of the backend device (e.g., DM1, TN1, etc.)
            sn (int): Number of shots.
            in_string (String): String representation of the circuit 
            in_verbatim (bool): Enable Braket verbatime mode.
            in_format (String): Format of in_string (e.g., "openqasm3")
            out_s3 (String): Name of the AWS S3 bucket where the results will be stored.
            out_s3_path (String): Path (key) to the result folder of this job in the S3 bucket. 
        Returns:
            AwsQuantumTask: An AwsQuantumTask that tracks the execution on the device.
    """        
    # Check that AWS credentials are provided and S3 bucket exists
    # Protected within lock context to avoid race conditions in boto3.client() constructor
    with BOTO_CLIENT_CREATE_LOCK:
        client = boto3.client("s3")
        try:
            client.head_bucket(Bucket=out_s3)
        except botocore.exceptions.NoCredentialsError:
            print("Your AWS Key and Secret are not configured.  To do this, use these commands in a JupyterLab Bash Terminal:")
            print("")
            print("python3 -m pip install awscli")
            print("python3 -m pip install amazon-braket-sdk")
            print("aws configure")
        except client.exceptions.ClientError:
            print("The S3 bucket you specified {} in [aws_s3] does not exist.".format(out_s3) +  
                  "Please create this bucket from the AWS Console with your credentials.")

    # Validate that aws_device is in the list of supported AWS Braket devices
    if aws_device == 'DM1':
        device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/dm1")
    elif aws_device == 'TN1':
        device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/tn1")
    elif aws_device == 'SV1':
        device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/sv1")
    elif aws_device == 'Rigetti':
        device = AwsDevice("arn:aws:braket:us-west-1::device/qpu/rigetti/Aspen-M-2")
    else:
        raise ValueError("Your AWS device is not suppported.  Valid AWS devices are: DM1 | TN1 | SV1 | Rigetti")
    
    if in_format == "openqasm3":
        program = Program(source=in_string)
    else:
        raise ValueError("Format is invalid.  Please use: openqasm3")
        # This needs improving - verbatim should work for openqasm3 as well...
        #
        # if in_verbatim == True:
        #     program = Circuit().add_verbatim_box(eval(in_string))
        # else:
        #     print("[Debug]: braket format, non-verbatim... ")
        #     program = eval(in_string)

    my_bucket = out_s3
    my_prefix = out_s3_path

    s3_folder = (my_bucket, my_prefix)
    task = device.run(program, s3_folder, shots=sn, disable_qubit_rewiring=in_verbatim)
    return task
    
def run_aws_braket(aws_device, sn, in_string, in_verbatim, in_format, out_s3, out_s3_path):
    """
        Submit a task to AWS Braket and wait for the result.

        Args:
            aws_device (String): Name of the backend device (e.g., DM1, TN1, etc.)
            sn (int): Number of shots.
            in_string (String): String representation of the circuit 
            in_verbatim (bool): Enable Braket verbatime mode.
            in_format (String): Format of in_string (e.g., "openqasm3")
            out_s3 (String): Name of the AWS S3 bucket where the results will be stored.
            out_s3_path (String): Path (key) to the result folder of this job in the S3 bucket. 

        Returns:
            Dict[str, int]: Measurement bitstring distribution (from bitstring to count).
    """          
    task = run_aws_braket_async(aws_device, sn, in_string, in_verbatim, in_format, out_s3, out_s3_path)
    # Wait for the task to complete using result()
    measurement_counts = task.result().measurement_counts
    count_map = {}
    # Construct the measurement map
    for key in measurement_counts.keys():
        count_map[key] = measurement_counts[key]
    
    return count_map

def get_available_backends(provider_name):
    """
        Get all available backends from an AWS provider 
        
        Args:
            provider_name (String): Name of the provider (e.g., Rigetti, QuEra, Xanadu, IonQ, etc.)
        
        Returns:
            Dict[str, str]: Dict of (backend name -> ARN) from that provider that is currently available.
    """
    avail_backends = {}
    all_devices = AwsDevice.get_devices()
    for device in all_devices:
        if device.provider_name == provider_name and device.is_available:
            avail_backends[device.name] = device.arn
    return avail_backends