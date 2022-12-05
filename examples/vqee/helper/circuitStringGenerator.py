from qiskit.compiler import transpile
from qiskit import QuantumCircuit, QuantumRegister
from qiskit.circuit import ParameterVector
from qiskit_nature.converters.second_quantization import QubitConverter
from qiskit_nature.mappers.second_quantization import JordanWignerMapper
from qiskit_nature.circuit.library.ansatzes.ucc import UCC
import qiskit.qasm3 as qasm3
import numpy as np
import qiskit2xasm 

n_H = 5 #for your test case: number of hydrogen atoms

num_spin_orbitals = 2*n_H #the number of 1-particle functions == number of qubits
num_particles = [n_H // 2 + n_H % 2, n_H // 2] #the number of alpha and beta electrons

#initial Hartree-Fock state: occupation number vector |1..10..01..10..0>
initial_state = QuantumCircuit(num_spin_orbitals)
for i in range(num_particles[0]):
    initial_state.x(i)
for i in range(num_particles[1]):
    initial_state.x(i + num_spin_orbitals//2)
 
print("initial state circuit")
print(initial_state.qasm())
print(qiskit2xasm.qiskit2xasm(initial_state))
   
#UCCSD circuit
uccsd = UCC(qubit_converter = QubitConverter(JordanWignerMapper()),
        	num_spin_orbitals = num_spin_orbitals,
        	num_particles = num_particles,
        	excitations = "sd")  
uccsd = transpile(uccsd, basis_gates = ["h","x", "cx", "rx", "ry", "rz"], optimization_level=0) #transpile to basis gates
 
nParameters = len(uccsd.parameters)
parameters = ParameterVector('P', nParameters)
 
uccsd.assign_parameters(parameters, inplace=True)
 
#if not implicitly done by xacc, both initial state and uccsd need to be merged!
circuit = QuantumCircuit(num_spin_orbitals)
circuit.compose(initial_state, inplace=True) #add initial state
circuit.compose(uccsd, inplace=True) #add uccsd
 

print("#Hydrogen atoms: ", n_H)
print("#electrons: ", num_particles)
print("circuit depth: ", circuit.depth())
print("#parameters: ", nParameters)
print("#Qbits: ", num_spin_orbitals)

#print("\nqasm format:")
#print(qasm3.dumps(circuit))

print("\nxasm format:")
#print(qiskit2xasm.qiskit2xasm(circuit))
print("\n\n")

#print(circuit)
