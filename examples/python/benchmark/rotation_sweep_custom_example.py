import qristal.core
import qristal.core.benchmark as benchmark

from datetime import datetime, timezone
import numpy as np

n_qubits = 1
n_shots = 1000

#(1) define session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = n_shots
sim.qn = n_qubits

#(2) define workflow
base_workflow = benchmark.RotationSweep(['Z'], -180, +180, 5, sim)
#Prepend rotation to |X+> to workflow circuits 
prepend = [benchmark.BlochSphereUnitState(benchmark.BlochSphereUnitStateSymbol.Xp)]
prepended_workflow = benchmark.PreOrAppendWorkflow(base_workflow, prepend, benchmark.Placement.Prepend)
#Append measurements (X, Z) to workflow circuits
append = [
    [benchmark.Pauli(benchmark.PauliSymbol.X)], 
    [benchmark.Pauli(benchmark.PauliSymbol.Z)]
]
final_workflow = benchmark.PreOrAppendWorkflow(prepended_workflow, append, benchmark.Placement.Append)

#(3) pass to metric
metric = benchmark.BitstringCounts(final_workflow) 

#(4) evaluate and print
results = metric.evaluate() 
for timestamp, counts_vec in results.items(): 
    # Convert timestamp to UTC and local time
    utc_time = datetime.fromtimestamp(timestamp, tz=timezone.utc)
    local_time = datetime.fromtimestamp(timestamp)  # Local timezone
    print(f"Evaluated metric from UTC: {utc_time.strftime('%c %Z')} "
          f"(local: {local_time.strftime('%c %Z')}):")
    for counts, circuit in zip(counts_vec, final_workflow.circuits):
        print("Circuit:") 
        circuit.print() 
        print("Measured bitstring counts:")
        print(counts)
        print("---")
