# Test cases for setting the number of threads that the AER backend should run with

import pytest
import os
import qristal.core

def run_circuit(num_threads, pass_via_OMP_NUM_THREADS = False):

  # Create a quantum computing session using Qristal
  my_sim = qristal.core.session()

  # Choose aer backend
  my_sim.acc = "aer"

  # Choose how many qubits to simulate
  my_sim.qn = 26

  # Choose how many shots to run through the circuit
  my_sim.sn = 1000

  # Keep things simple by turning off circuit optimisation
  my_sim.nooptimise = True

  # Choose how many OpenMP threads to use to run the AER simulator
  if (pass_via_OMP_NUM_THREADS):
    os.environ['OMP_NUM_THREADS'] = str(num_threads)
  else:
    my_sim.aer_omp_threads = num_threads
    # This just to test that session.omp_num_threads takes precedence over OMP_NUM_THREADS
    os.environ['OMP_NUM_THREADS'] = str(num_threads + 1)

  # Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  my_sim.instring = '''
  __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
  {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[26];
        h q[0];
        cx q[0],q[1];
        cx q[1],q[2];
        cx q[2],q[3];
        cx q[3],q[4];
        cx q[4],q[5];
        cx q[5],q[6];
        cx q[6],q[7];
        cx q[7],q[8];
        cx q[8],q[9];
        cx q[9],q[10];
        cx q[10],q[11];
        cx q[11],q[12];
        cx q[12],q[13];
        cx q[13],q[14];
        cx q[14],q[15];
        cx q[15],q[16];
        cx q[16],q[17];
        cx q[17],q[18];
        cx q[18],q[19];
        cx q[19],q[20];
        cx q[20],q[21];
        cx q[21],q[22];
        cx q[22],q[23];
        cx q[23],q[24];
        cx q[24],q[25];
        measure q[0] -> c[0];
        measure q[1] -> c[1];
        measure q[2] -> c[2];
        measure q[3] -> c[3];
        measure q[4] -> c[4];
        measure q[5] -> c[5];
        measure q[6] -> c[6];
        measure q[7] -> c[7];
        measure q[8] -> c[8];
        measure q[9] -> c[9];
        measure q[10] -> c[10];
        measure q[11] -> c[11];
        measure q[12] -> c[12];
        measure q[13] -> c[13];
        measure q[14] -> c[14];
        measure q[15] -> c[15];
        measure q[16] -> c[16];
        measure q[17] -> c[17];
        measure q[18] -> c[18];
        measure q[19] -> c[19];
        measure q[20] -> c[20];
        measure q[21] -> c[21];
        measure q[22] -> c[22];
        measure q[23] -> c[23];
        measure q[24] -> c[24];
        measure q[25] -> c[25];
  }
  '''

  # Run the circuit
  my_sim.run()


def test_aer_omp_thread():
  run_circuit(7)

def test_omp_num_threads():
  run_circuit(3, True)


