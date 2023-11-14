from flask import Flask, jsonify, request, abort
import uuid, json, numpy, os, subprocess
from qiskit import QuantumCircuit, transpile
from qiskit.compiler import assemble
import threading, queue
from pathlib import Path
import logging
# Setup basic logging
logging.basicConfig(filename='server.log', level=logging.INFO)
logging.info('Logger is set up.')
# This is currently a user-level executable
# We could compile it at root level.
AER_SIMULATOR_PATH = '/mnt/qb/qiskit-aer-local/build/Release/qasm_simulator'
app = Flask(__name__)
job_list = queue.Queue()
# run_with_ngrok(app)
running_job_id = ''

directory = os.path.dirname(os.path.realpath(__file__)) + '/data/'
if not os.path.exists(directory):
    os.makedirs(directory)

def check_for_job():
    global running_job_id
    while True:
        job_id = job_list.get()
        logging.info(f'Working on {job_id}')
        dir_path = os.path.dirname(os.path.realpath(__file__))
        qobj_file = dir_path + '/data/' + str(job_id) + '.qobj'
        config_file = dir_path + '/data/' + str(job_id) + '_config.json'
        # Run the simulation:
        running_job_id = str(job_id)
        result = subprocess.run([AER_SIMULATOR_PATH, '-c', config_file, qobj_file], stdout=subprocess.PIPE)
        result_str = str(result.stdout.decode('utf8'))
        # Handle the case where the simulation failed/crashed
        try:
            result_str = "{" + result_str.split("{", 1) [1]
            result_json = json.loads(result_str)
        except:
            result_json = {"status": "FAILED"}

        result_json = json.loads(result_str)
        result_json_file = dir_path + '/data/' + str(job_id) + '_result.json'
        with open(result_json_file, 'w') as f:
            json.dump(result_json, f)
        logging.info(f'Finished {job_id}')
        running_job_id = ''
        job_list.task_done()

class QobjEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, numpy.ndarray):
            return obj.tolist()
        if isinstance(obj, complex):
            return (obj.real, obj.imag)
        return json.JSONEncoder.default(self, obj)

@app.route('/job', methods=['PUT'])
def new_job():
    request_body = request.get_json(force=True)
    new_job_id = uuid.uuid4()
    dir_path = os.path.dirname(os.path.realpath(__file__))
    # Default configs
    run_config = {
        "device": "GPU",
        "cuStateVec_enable": True,
        "precision": "single",
        "blocking_enable": True,
        "blocking_qubits": 27
    }

    if ("device" in request_body):
        run_config["device"] = str(request_body["device"])

    if ("method" in request_body):
        run_config["method"] = str(request_body["method"])

    nbShots = 1024
    if ("shots" in request_body):
        nbShots = int(request_body["shots"])

    if ("noise_model" in request_body):
        noise_model_str = str(request_body["noise_model"])
        try:
            json.loads(noise_model_str)
        except ValueError as e:
            return jsonify({'job-id' : str(new_job_id), 'status': 'FAILED', 'reason': "Invalid noise model JSON"})
        run_config["noise_model"] = json.loads(noise_model_str)
    run_config_file = dir_path + '/data/' + str(new_job_id) + '_config.json'
    with open(run_config_file, 'w') as f:
        json.dump(run_config, f, cls=QobjEncoder)

    if ("openqasm" in request_body):
        qasm_src = request_body["openqasm"]
        try:
            qc = QuantumCircuit.from_qasm_str(qasm_src)
            result = transpile(qc, basis_gates=['u1', 'u2', 'u3', 'cx'], optimization_level=3)
            qobj = assemble(result, qobj_id=str(new_job_id), shots=nbShots)
            qobj_file = dir_path + '/data/' + str(new_job_id) + '.qobj'
            with open(qobj_file, 'w') as f:
                json.dump(qobj.to_dict(), f, cls=QobjEncoder)
            job_list.put(str(new_job_id))
        except:
            # Failed to compile
            return jsonify({'job-id' : str(new_job_id), 'status': 'FAILED'})
    else:
        return jsonify({'job-id' : str(new_job_id), 'status': 'IGNORED'})
    return jsonify({'job-id' : str(new_job_id), 'status': 'SUBMITTED'})

@app.route('/job/<job_id>', methods=['GET'])
def get_job_status(job_id):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    qobj_filename = dir_path + '/data/' + str(job_id) + '.qobj'
    qobj_file = Path(qobj_filename)
    if qobj_file.is_file():
        result_filename = dir_path + '/data/' + str(job_id) + '_result.json'
        result_file = Path(result_filename)
        if result_file.is_file():
           result_json_str = result_file.read_text()
           return jsonify({'job-id' : str(job_id), 'status': 'COMPLETED', 'data': result_json_str})
    else:
        abort(404)
    if running_job_id == str(job_id):
        return jsonify({'job-id' : str(job_id), 'status': 'RUNNING'})
    return jsonify({'job-id' : str(job_id), 'status': 'PENDING IN QUEUE', 'queue-size': job_list.qsize()})


if __name__ == '__main__':
    threading.Thread(target=check_for_job).start()
    # app.run(debug=True)
    app.run()
