// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/passes/noise_aware_placement_pass.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/session.hpp"
#include <string>
#include <iostream>

int main()
{
  auto my_sim = qb::session(false);
  qb::CircuitBuilder my_circuit;
  // Create a simple Bell state circuit.
  my_circuit.H(0);
  my_circuit.CNOT(0, 1);
  my_circuit.MeasureAll(2);
  my_circuit.print();
  qb::noise_aware_placement_config my_device;
  // Create a dummy 5-qubit device with linear connectivity
  // 0 - 1 - 2 - 3 - 4
  my_device.qubit_connectivity = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
  my_device.avg_single_qubit_gate_errors = {
      {0, 0.01}, {1, 0.01}, {2, 0.01}, {3, 0.01}, {4, 0.01}};
  my_device.avg_qubit_readout_errors = {
      {0, 0.05}, {1, 0.05}, {2, 0.05}, {3, 0.05}, {4, 0.05}};
  my_device.avg_two_qubit_gate_errors = {
      {{0, 1}, 0.1}, {{1, 2}, 0.1}, {{2, 3}, 0.02}, {{3, 4}, 0.1}};
  auto tket_placement = qb::create_noise_aware_placement_pass(my_device);
  tket_placement->apply(my_circuit);
  std::cout << "After placement:\n";
  my_circuit.print();
}
