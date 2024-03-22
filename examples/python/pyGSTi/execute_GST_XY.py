#This script contains a demonstration of how to use pyGSTi with a qristal backend. It will (1) set up a pyGSTi experiment design, 
#(2) generate a list of required quantum circuits (in pyGSTi format) and export it to a file "circuit_list_file_name", (3) write 
#empty protocol data for the given experiment to a folder "name", (4) feed the exported circuit list into the pyGSTi_runner binary
#example to actually execute all the circuits using the session object set within pyGSTi_runner.cpp and write the collected results 
#into the dataset.txt file within the pyGSTi protocol data, (5) perform a standard gate set tomography protocol using pyGSTi, and 
#(6) create a human-readable html report with all the results. 

#To do BEFORE executing this script / intended way of use: 
# -> Modify this script to the specific model / protocol that you want to use with pyGSTi. Per default, a standard gate set 
#    tomography protocol with maximum length 1 for the 1-qubit gates Rx(pi/2) and Ry(pi/2) is set. 
# -> Set the backend, noise model, number of qubits, number of shots etc. that you want to run the pyGSTi circuits on within 
#    examples/cpp/pyGSTi_runner/pyGSTi_runner.cpp and compile it to obtain a "pyGSTi_runner" executable. Per default, pyGSTi_runner
#    is set up to use the "aer" backend with 1 qubit, 1000 shots, and the "default" noise model.
# -> Execute this script passing it the path to the pyGSTi_runner executable as the first argument.

#(0) Obtain pyGSTi_runner path from given arguments
import argparse 
import sys
import os 
parser = argparse.ArgumentParser("This script will execute a a pyGSTi workflow using a given circuit runner executable using qristal. For more details see the comments in the beginning of this script.")
parser.add_argument('path_to_pyGSTi_runner', help = 'Path to the pyGSTi runner executable used for circuit execution.')
parsed_args = parser.parse_args(sys.argv[1:])
assert os.path.isfile(parsed_args.path_to_pyGSTi_runner) == True, "Path to pyGSTi_runner is invalid!" 

#(1) Generate experiment design by 
# (a) importing a standard modelpack or 
# (b) creating your own pyGSTi target model.
# In case of (b), make sure that the all specified gate names are known and set in include/qb/core/benchmark/workflows/PyGSTiBenchmark.hpp 
from pygsti.modelpacks import smq1Q_XY, smq2Q_XYCNOT
maximum_length = 1
edesign = smq1Q_XY.create_gst_experiment_design(maximum_length)
name = "GST_XY"

#(2) generate and print circuit list to circuit_list_file_name
from pygsti.io import write_circuit_list
circuits = list(edesign.all_circuits_needing_data)
circuit_list_file_name = name + "_circuits.out"
write_circuit_list(circuit_list_file_name, circuits)

#(3) Write empty protocol data to 'dirname/'
from pygsti.io import write_empty_protocol_data
overwrite_existing = True
write_empty_protocol_data(name, edesign, clobber_ok=overwrite_existing)

#(4) Feed circuit list from circuit_list_file_name to pygsti_runner from qb::benchmark and write results to dirname/data/dataset.txt
#note: all executed circuit results are automatically stored in a new folder "intermediate_benchmark_results"
os.system(parsed_args.path_to_pyGSTi_runner + " < " + circuit_list_file_name + " > " + name + "/data/dataset.txt")

#(5) perform GST  
from pygsti.io import read_data_from_dir
from pygsti.protocols import StandardGST
data = read_data_from_dir(name)
protocol = StandardGST()
results = protocol.run(data)

#(6) write html report 
from pygsti.report import construct_standard_report
report = construct_standard_report(results, title=name)
report_name = name + "_report"
report.write_html(report_name)