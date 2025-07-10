// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>


int main()
{
  // Make a Qristal session
  auto my_sim = qristal::session(false);

  my_sim.supervisor_print("Executing Qristal MPI Demo...");

  // Configure the session
  my_sim.debug = false;
  my_sim.qn = 20;
  my_sim.noise = true;
  my_sim.noise_model = std::make_shared<qristal::NoiseModel>("default", my_sim.qn);
  my_sim.acc = "aer";
  my_sim.seed = 0;
  my_sim.initial_bond_dimension = 1;
  my_sim.initial_kraus_dimension = 1;
  my_sim.max_bond_dimension = 256;
  my_sim.max_kraus_dimension = 256;
  my_sim.svd_cutoff = 1e-6;
  my_sim.rel_svd_cutoff = 1e-3;
  my_sim.measure_sample_method = "auto";
  my_sim.aer_sim_type = "matrix_product_state";
  my_sim.aer_omp_threads = 1;

    // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[20];
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
    }
  )";

  // Hand the kernel over to the sim object
  my_sim.instring = targetCircuit;

  // Run the circuit and count up the results in each of the classical
  // registers
  my_sim.sn = 100000;
  my_sim.supervisor_print("Running {} shots\n", my_sim.sn);
  my_sim.run();
  my_sim.supervisor_print("Ran successfully!\n");

  // Print the cumulative results in each of the classical registers
  my_sim.supervisor_print("Results:\n{}\n", my_sim.results());
}
