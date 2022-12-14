#   Copyright (c) Quantum Brilliance Pty Ltd (2022)
#
#   topology.py
#
#   Simple example that prints out circuit topology
#   and connectedness.
#
####################################################

import sys
import os
import numpy as np
import qb.core

def connections(circuit_string) :
    gates = circuit_string.splitlines()
    topology = {}
    for gate in gates :
        qubits = []
        q_pos = gate.find("q[") + 2
        while q_pos > 2 :
            q_end = gate.find("]",q_pos)
            qubits += [gate[q_pos:q_end]]
            q_pos = gate.find("q[",q_end) + 2
            for qubit in qubits:
                if not qubit in topology.keys() :
                    topology[qubit] = []
                    qubits_ = [qubit_ for qubit_ in qubits if qubit_ != qubit]
                    topology[qubit] += [qubit_ for qubit_ in qubits_ if qubit_ not in topology[qubit]]
    return topology

def max_connected(topology_) :
    max_connected = 0
    for qubits_ in topology_.values() :
        if len(qubits_) > max_connected :
            max_connected = len(qubits_)
    return max_connected


if __name__ == "__main__" :
    circuit_string = "This is a circuit\n"
    circuit_string += "x q[1] ;\n"
    circuit_string += "cx q[0] q[1] , q[2]\n"
    circuit_topology = connections(circuit_string)
    print("Connectedness:",max_connected(circuit_topology))
    print("\nTopology:\n",circuit_topology)
