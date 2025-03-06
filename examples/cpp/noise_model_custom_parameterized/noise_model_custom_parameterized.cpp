// Copyright (c) Quantum Brilliance Pty Ltd
/**
 * This example shows how to use your own noise model
 * parameters to generate a noise model object.
 */

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"

int main(int argc, char * argv[])
{
  // Create a dummy device using noise parameters.
  // 4 qubits with a line topology:
  // 0 - 1 - 2 - 3
  const int nb_qubits = 4;

  // Qubit topology
  std::vector<std::pair<int, int>> qubit_topology_list = {};
  for (size_t i = 0; i < nb_qubits - 1; i++) {
    qubit_topology_list.emplace_back(std::make_pair(i, i + 1));
  }

  // Qubit t1 and t2 (microseconds)
  const double t1 = 1e6; // Relaxation time
  const double t2 = 1e3; // Dephasing time

  // Gate time (microseconds)
  const double u1_time = 1.0;
  const double u2_time = 1.0;
  const double u3_time = 1.0;
  const double cx_time = 2.0;

  // Gate error (microseconds)
  const double u1_error = 1e-4;
  const double u2_error = 1e-3;
  const double u3_error = 1e-3;
  const double cx_error = 5e-2;

  // Read-out error
  qristal::ReadoutError read_out_error;
  read_out_error.p_01 = 1e-2;
  read_out_error.p_10 = 1e-2;

  qristal::NoiseProperties device_properties;

  // 1-qubit properties
  for (size_t i = 0; i < nb_qubits; i++) {
    device_properties.t1_us[i] = t1;
    device_properties.t2_us[i] = t2;
    device_properties.readout_errors[i] = read_out_error;
    device_properties.gate_time_us["u1"][{i}] = u1_time;
    device_properties.gate_time_us["u2"][{i}] = u2_time;
    device_properties.gate_time_us["u3"][{i}] = u3_time;
    device_properties.gate_pauli_errors["u1"][{i}] = u1_error;
    device_properties.gate_pauli_errors["u2"][{i}] = u2_error;
    device_properties.gate_pauli_errors["u3"][{i}] = u3_error;
  }

  // 2-qubit properties
  for (size_t i = 0; i < nb_qubits - 1; i++) {
    device_properties.gate_time_us["cx"][{i, i + 1}] = cx_time;
    device_properties.gate_pauli_errors["cx"][{i, i + 1}] = cx_error;
  }

  // Qubit topology
  device_properties.qubit_topology = qubit_topology_list;

  // Create noise model using device properties
  auto nm = qristal::NoiseModel(device_properties);

  auto s = qristal::session(false);
  s.init();
  s.set_qn(nb_qubits);
  s.set_noise(true);
  s.set_noise_model(nm);
  s.set_acc("aer");

  // Simple Bell circuit
  s.set_instring(R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
    )");

  // Execute circuit
  s.run();

  // Print results
  std::cout << "Results:" << std::endl << s.results()[0][0] << std::endl;
}
