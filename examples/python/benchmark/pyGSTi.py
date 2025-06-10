#This script contains a demonstration of how to use pyGSTi with a qristal backend. It will (1) set up a qristal session that is
#used to evaluate all circuits, (2) set up a pyGSTi experiment design, (3) generate a list of required quantum circuits (in
#pyGSTi format), (4) evaluate all circuits through qristal.benchmark, (5) create empty protocol data for the given experiment to a
#folder "name" and write the measured circuit results to it, (6) perform a standard gate set tomography protocol using pyGSTi, and
#(7) create a human-readable html report with all the results.

#To do BEFORE executing this script / intended way of use:
# -> Modify this script to the specific model / protocol that you want to use with pyGSTi. Per default, a standard gate set
#    tomography protocol with maximum length 1 for the 2-qubit gate model Rx(pi/2), Ry(pi/2), and CZ is set.
# -> Set the backend, noise model, number of qubits, number of shots etc. that you want to run the pyGSTi circuits on in the
#    qristal.core.session set up in this script.

import qristal.core
import qristal.core.benchmark as benchmark

from pygsti.modelpacks import smq1Q_XY, smq2Q_XYCNOT
import sm2Q_XYCZ #custom QDK model for QDK basis gates (Rx(pi/2), Ry(pi/2), and CZ)
from pygsti.io import write_empty_protocol_data
from pygsti.io import read_data_from_dir
from pygsti.protocols import StandardGST
from pygsti.report import construct_standard_report

n_qubits = 2
n_shots = 100

#(1) Set up session
sim = qristal.core.session()
sim.acc = "qpp"
sim.sn = n_shots
sim.qn = n_qubits

#(2) Generate experiment design by
# (a) importing a standard modelpack or
# (b) creating your own pyGSTi target model.
# In case of (b), make sure that the all specified gate names are known and set in include/qb/core/benchmark/workflows/PyGSTiBenchmark.hpp
maximum_length = 1
edesign = sm2Q_XYCZ.create_gst_experiment_design(maximum_length)
name = "GST_XYCZ"
circuits = qristal.core.VectorString([circuit.str for circuit in edesign.all_circuits_needing_data])

#(3) Initialize qristal workflow
workflow = benchmark.PyGSTiBenchmark(circuits, sim)

#(4) Pass to metric (to write PyGSTi-compatible results) and run
metric = benchmark.PyGSTiResults(workflow)
results = metric.evaluate()

#(5) Create empty PyGSTi database and write results
overwrite_existing = True
write_empty_protocol_data(name, edesign, clobber_ok=overwrite_existing)
with open(name + "/data/dataset.txt", 'w') as file:
    for _, circuit_results in results.items():
        for result in circuit_results:
            print(result, file=file)

#(6) perform GST
data = read_data_from_dir(name)
protocol = StandardGST()
results = protocol.run(data)

#(7) write html report
report = construct_standard_report(results, title=name)
report_name = name + "_report"
report.write_html(report_name)
