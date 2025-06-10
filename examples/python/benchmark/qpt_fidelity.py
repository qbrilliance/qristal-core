import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone

n_qubits = 2
n_shots = 1000

#(1) define session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = n_shots
sim.qn = n_qubits

#(2) define workflow
workflow = benchmark.RotationSweep(['X', 'I'], -180, +180, 5, sim)
qstworkflow = benchmark.QuantumStateTomography(workflow)
qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

#(3) pass to metric
metric = benchmark.QuantumProcessFidelity(qptworkflow)

#(4) evaluate and print
results = metric.evaluate()
for timestamp, fidelities in results.items():
    # Convert timestamp to UTC and local time
    utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    local_time = datetime.fromtimestamp(timestamp)  # Local timezone
    print(f"Evaluated metric from UTC: {utc_time.strftime('%c %Z')} "
          f"(local: {local_time.strftime('%c %Z')}) : ")
    print(fidelities)
