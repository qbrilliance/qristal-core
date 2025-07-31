import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone

#(1) define session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = 1000
sim.qn = 2

#(2) define workflow: CZ gate execution
circuit = qristal.core.Circuit()
circuit.cz(0, 1)
# wrap circuit in SimpleCircuitExecution and tell qristal.core.benchmark to 
# evaluate ideal process matrix from ideal simulation
workflow = benchmark.SimpleCircuitExecution(circuit, sim)
workflow = benchmark.AddinIdealProcessFromIdealSimulation(workflow)
qstworkflow = benchmark.QuantumStateTomography(workflow) #wrap workflow into QST object
qptworkflow = benchmark.QuantumProcessTomography(qstworkflow) #wrap QST into QPT object

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
    for idx, fidelity in enumerate(fidelities):
        print("Circuit ", idx)
        print("Process fidelity      = ", fidelity)
        print("Average gate fidelity = ", benchmark.calculate_average_gate_fidelity(fidelity, sim.qn))
    
