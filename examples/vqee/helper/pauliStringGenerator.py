from qiskit_nature.drivers.second_quantization import PySCFDriver
from qiskit_nature.drivers import UnitsType
from qiskit_nature.converters.second_quantization import QubitConverter
from qiskit_nature.mappers.second_quantization import JordanWignerMapper
from qiskit_nature.problems.second_quantization import ElectronicStructureProblem
import re 

#function returning Pauli string for a given molecule
def get_Pauli_string(molecule, basis = "sto-3g", spin = 0):
    driver = PySCFDriver(atom = molecule, basis= basis, unit = UnitsType.BOHR, spin = spin)
    es_problem = ElectronicStructureProblem(driver)
    second_q_op = es_problem.second_q_ops()
    qubit_converter = QubitConverter(JordanWignerMapper())
    qubit_op = qubit_converter.convert(second_q_op[0])
    return qubit_op

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
#    if (terms != ""):
#        terms = "* " + terms
    return terms

for n_H in [5]: #range(1,4): # max 11
    pauli_string = get_Pauli_string(create_hydrogen_chain(n_H), "sto-3g", n_H % 2)
    print("\n\n#H = " + str(n_H) + ", #qubits = " + str(pauli_string.num_qubits) + ", Pauli terms = "  + str(len(pauli_string)))
    #print(pauli_string)

    pauliSplit = re.split(r'\n', str(pauli_string))
    print(len(pauliSplit))
    #print(pauliSplit)
    offset = 2 + pauli_string.num_qubits
    for i, pauli in enumerate(pauliSplit):
        pauliSplit[i] = pauli[:-offset:1] + replaceTerms(pauli[-offset::1])
    pauliString = ' '.join(pauliSplit)
    print(pauliString)
        
