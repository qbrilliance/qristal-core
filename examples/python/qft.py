import qristal.core
import numpy as np
import multiprocessing

nb_qubits = 5
nb_shots = 1000
circ = qristal.core.Circuit()

# Apply the QFT circuit
circ.qft(range(nb_qubits))

# Apply measurement
circ.measure_all()

# Print out circuit
circ.print()

# List of backends
backends = ["aer_mps", "aer_dm", "tnqvm", "cudaq:dm"]

# Create list of sessions to be executed in parallel
sessions = []
for backend in backends:
    s = qristal.core.session()
    s.init()
    s.qn = nb_qubits
    s.sn = nb_shots
    s.ir_target = circ

    if backend == "aer_mps":
        s.acc = "aer"
        s.aer_sim_type = "matrix_product_state"
        s.aer_omp_threads = 4
    elif backend == "aer_dm":
        s.acc = "aer"
        s.aer_sim_type = "density_matrix"
        s.aer_omp_threads = 4
    else:
        s.acc = backend
    sessions.append(s)

# Execute circuit and retrieve distribution
def get_prob_dist(sim, results):
    s.run()
    measured_probs_dict = {}
    for i in s.results[0][0]:
        measured_probs_dict[str(i)[::-1]] = s.results[0][0][i] / s.results[0][0].total_counts()
    results.append(measured_probs_dict)

# Execute backends in parallel
processes = []
manager = multiprocessing.Manager()
all_results = manager.list()  # This will be shared across all processes
for sim in sessions:
    p = multiprocessing.Process(
        target = get_prob_dist,
        args = (sim, all_results)
    )
    processes.append(p)
    p.start()

# Wait for all workers to complete task
for p in processes:
    p.join()

# Print all results
for i in np.arange(len(backends)):
    print("Results", backends[i], ":", all_results[i], "\n")
