from qiskit.circuit import ParameterExpression

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
#function returning xasm circuit string from a qiskt QuantumCircuit
def qiskit2xasm(circuit):
    #gate name translations (taken from https://www.notion.so/quantumbrilliance/Quantum-Gates-7820f4f28e564f69bd74a585bc527471#116bb2c3773b4b25a45f65b0994ca1ab)
    qasm2xasm = {
        "h" : "H",
        "x" : "X",
        "y" : "Y",
        "z" : "Z",
        "rx" : "Rx",
        "ry" : "Ry",
        "rz" : "Rz",
        "cx" : "CNOT",
        "cy" : "CY",
        "cz" : "CZ"
    }

    xasm_string = '''
.compiler xasm
.circuit ansatz
.parameters P
.qbit q
'''
    #build instructions from circuit data
    for i in circuit.data:
        name = qasm2xasm[i[0].name] #get XASM name of gate
        num_qubits = i[0].num_qubits #number of qubits that gate applies to
        params = i[0].params #all parameters required for that gate

        xasm_string += name + "("
        for j in range(num_qubits):
            if (j > 0):
                xasm_string += ", "
            xasm_string += "q[" + str(i[1][j].index) + "]"
        for j in params:
            if isinstance(j, ParameterExpression):
                for k in j._parameter_symbols:
                    xasm_string += ", P[" + str(k.index) + "]"
            else:
                xasm_string += ", " + str(j)
        xasm_string +=");\n"
    return xasm_string
