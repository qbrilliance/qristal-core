import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone

qubits = {0, 1}
n_shots = 100

#(1) define session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = n_shots
sim.qn = len(qubits)

#(2) define workflow
workflow = benchmark.SPAMBenchmark(qubits, sim)
qstworkflow = benchmark.QuantumStateTomography(workflow)

#(3) pass to metric
metric = benchmark.QuantumStateFidelity(qstworkflow)

#(4) evaluate and print
results = metric.evaluate()
for timestamp, fidelities in results.items():
    # Convert timestamp to UTC and local time
    utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    local_time = datetime.fromtimestamp(timestamp)  # Local timezone
    print(f"Evaluated metric from UTC: {utc_time.strftime('%c %Z')} "
          f"(local: {local_time.strftime('%c %Z')}) : ")
    print(fidelities)
