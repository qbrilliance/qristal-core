from qiskit_nature.drivers import UnitsType
from qiskit_nature.drivers.second_quantization import PySCFDriver
from qiskit_nature.problems.second_quantization import ElectronicStructureProblem
from qiskit_nature.converters.second_quantization import QubitConverter
from qiskit_nature.mappers.second_quantization import JordanWignerMapper
import re 

#function returning Pauli string for a given molecule
def get_Pauli_string(molecule, basis = "sto-3g", spin = 0):
    driver = PySCFDriver(atom = molecule, basis= basis, unit = UnitsType.BOHR, spin = spin)
    es_problem = ElectronicStructureProblem(driver)
    second_q_op = es_problem.second_q_ops()
    qubit_converter = QubitConverter(JordanWignerMapper())
    qubit_op = qubit_converter.convert(second_q_op["ElectronicEnergy"])
    nuc_rep = es_problem.grouped_property_transformed.get_property("ElectronicEnergy").nuclear_repulsion_energy
    return qubit_op, nuc_rep

#linear hydrogen chain with 1.4 bohr distance between H atoms
def create_hydrogen_chain(n):
    molecule = ""
    j = 0
    for i in range (1, n+1):
        h_term = "H 0 0 " + str(j) + "; "
        molecule += h_term
        j += 1.4
    return(molecule)

def replaceTerms(string):
    terms = ""
    for i in range(len(string)):
        if (string[-i] == " " or string[-i] == "*"):
           continue
        if (string[-i] != "I"): 
           terms += string[-i]+str(i-1)
    return terms

for n_H in [2]: #range(1,4): # max 11
    pauliObj, nuc_rep = get_Pauli_string(create_hydrogen_chain(n_H), "sto-3g", spin = n_H % 2)
    print("\n\n#H = " + str(n_H) + ", #qubits = " + str(pauliObj.num_qubits) + ", Pauli terms = "  + str(len(pauliObj)))
    print(pauliObj)
    pauliSplit = re.split(r'\n', str(pauliObj)) # this is list of string
    offset = 2 + pauliObj.num_qubits
    for i, pauli in enumerate(pauliSplit):
        pauliSplit[i] = pauli[:-offset:1] + replaceTerms(pauli[-offset::1])
    val0 = float(pauliSplit[0])+nuc_rep
    pauliSplit[0] = ('- ' if (val0<0) else '+ ') + str(abs(val0))
    pauliString = ' '.join(pauliSplit)
    print("\n\n" + pauliString)
