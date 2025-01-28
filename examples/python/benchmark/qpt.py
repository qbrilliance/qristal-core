import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone
import numpy as np 

n_qubits = 1 
n_shots = 1000

#(1) define session
sim = qristal.core.session(False)
sim.init() 
sim.acc = "qpp"
sim.sn = n_shots 
sim.qn = n_qubits

#(2) define workflow
circuit = qristal.core.Circuit() 
circuit.rx(0, np.pi / 2.0)
workflow = benchmark.SimpleCircuitExecution([circuit], sim)
qstworkflow = benchmark.QuantumStateTomography(workflow)
qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

#(3) pass to metric
metric = benchmark.QuantumProcessMatrix(qptworkflow)

#(4) evaluate and print 
results = metric.evaluate()
for timestamp, processes in results.items():
    # Convert timestamp to UTC and local time
    utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    local_time = datetime.fromtimestamp(timestamp)  # Local timezone
    print(f"Evaluated metric from UTC: {utc_time.strftime('%c %Z')} "
          f"(local: {local_time.strftime('%c %Z')}):")
    
    # Iterate through densities
    for i, process in enumerate(processes):
        print(f"Quantum process matrix of circuit {i}:")
        print(process)