// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include <string>
#include <iostream>

int main()
{
  // Define a Qristal session
  const size_t n_qubits = 2;
  auto my_sim = qristal::session(false);
  my_sim.init();
  my_sim.set_acc("aer");
  my_sim.set_qn(n_qubits);
  my_sim.set_sn(100);

  // Add a custom noise model including readout errors only
  const double p_01 = 0.05; 
  const double p_10 = 0.05;
  qristal::NoiseModel SPAM_error;
  for (size_t q = 0; q < n_qubits; ++q) {
    SPAM_error.set_qubit_readout_error(q, qristal::ReadoutError(p_01, p_10));
    for (size_t qq = q+1; qq < n_qubits; ++qq) {
      SPAM_error.add_qubit_connectivity(q, qq);
    }
  }
  my_sim.set_noise(true);
  my_sim.set_noise_model(SPAM_error);

  // Define a Bell circuit to run 
  auto circuit = qristal::CircuitBuilder();
  circuit.H(0);
  circuit.CNOT(0, 1);
  circuit.MeasureAll(2);

  // Hand the kernel over to the my_sim object
  my_sim.set_irtarget_m(circuit.get());

  // Automatically measure a SPAM benchmark for 1000 shots, and enable automatic SPAM correction
  // then run the Bell circuit for the requested 100 shots
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run_with_SPAM(1000);
  std::cout << "Ran successfully!" << std::endl;
  std::cout << "The following SPAM correction matrix was used: \n" << my_sim.get_SPAM_correction_matrix() << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Native Results:" << std::endl << my_sim.results_native()[0][0] << std::endl;
  std::cout << "SPAM-corrected Results:" << std::endl << my_sim.results()[0][0] << std::endl;

}
