// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>

#include <yaml-cpp/yaml.h>

#include <string>

int main() {
  // Make a Qristal session
  auto my_sim = qristal::session(false);

  my_sim.supervisor_print("Executing Qristal MPI Demo...");

  my_sim.remote_backend_database_path = QRISTAL_DIR + std::string("/examples/cpp/mpi_multi_qpu_demo/localhost_vqpus.yaml");

  // Set up backends for MPI processes
  YAML::Node config = YAML::LoadFile(QRISTAL_DIR + std::string("/examples/cpp/mpi_multi_qpu_demo/mpi_process_accelerators.yaml"));

  YAML::Node accelerators = config["accelerators"];
  for (const auto& accelerator : accelerators) {
    my_sim.mpi_hardware_accelerators.emplace_back(accelerator.as<std::string>());
  }

  // Choose how many 'shots' to run through the circuit
  my_sim.sn = 100000;

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum
  // circuit') and hand the kernel over to the sim object
  my_sim.instring = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      x q[1];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
    }
    )";

  // Run the circuit and count up the results in each of the classical
  // registers
  my_sim.supervisor_print("About to run quantum program...\n");
  my_sim.run();
  my_sim.supervisor_print("Ran successfully!\n");

  // Print the cumulative results in each of the classical registers
  std::stringstream ss;
  ss << "Results:\n" << my_sim.results() << "\n";
  my_sim.supervisor_print(ss.str());
}
