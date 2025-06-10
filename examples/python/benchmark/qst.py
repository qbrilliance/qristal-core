import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone
import numpy as np

n_qubits = 2
n_shots = 1000

#(1) define session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = n_shots
sim.qn = n_qubits

#(2) define workflow
circuit = qristal.core.Circuit()
circuit.h(0)
circuit.cnot(0, 1)
workflow = benchmark.SimpleCircuitExecution([circuit], sim)
qstworkflow = benchmark.QuantumStateTomography(workflow, True) #wrap into QST object and enable maximum likelihood estimation

#(3) pass to metric
metric = benchmark.QuantumStateDensity(qstworkflow)

#(4) evaluate and print
results = metric.evaluate()
for timestamp, densities in results.items():
    # Convert timestamp to UTC and local time
    utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    local_time = datetime.fromtimestamp(timestamp)  # Local timezone
    print(f"Evaluated metric from UTC: {utc_time.strftime('%c %Z')} "
          f"(local: {local_time.strftime('%c %Z')}):")

    # Iterate through densities
    for i, density in enumerate(densities):
        print(f"Quantum state density of circuit {i}:")
        print(density)
