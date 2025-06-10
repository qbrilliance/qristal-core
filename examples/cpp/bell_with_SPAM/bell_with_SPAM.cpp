// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/noise_model/noise_model.hpp>
#include <string>
#include <iostream>
#include <memory>

int main()
{
  // Define a Qristal session
  auto my_sim = qristal::session();
  my_sim.acc = "aer";
  my_sim.qn = 2;
  my_sim.sn = 100;

  // Add a custom noise model including readout errors only
  const double p_01 = 0.05;
  const double p_10 = 0.05;
  auto SPAM_error = std::make_shared<qristal::NoiseModel>();
  for (size_t q = 0; q < my_sim.qn; ++q) {
    SPAM_error->set_qubit_readout_error(q, qristal::ReadoutError(p_01, p_10));
    for (size_t qq = q+1; qq < my_sim.qn; ++qq) {
      SPAM_error->add_qubit_connectivity(q, qq);
    }
  }
  my_sim.noise = true;
  my_sim.noise_model = SPAM_error;

  // Define a Bell circuit to run
  auto circuit = qristal::CircuitBuilder();
  circuit.H(0);
  circuit.CNOT(0, 1);
  circuit.MeasureAll(2);

  // Hand the kernel over to the my_sim object
  my_sim.irtarget = circuit.get();

  // Automatically measure a SPAM benchmark for 1000 shots, and enable automatic SPAM correction
  // then run the Bell circuit for the requested 100 shots
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run_with_SPAM(1000);
  std::cout << "Ran successfully!" << std::endl;
  std::cout << "The following SPAM correction matrix was used: \n" << my_sim.SPAM_correction_matrix << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Native Results:" << std::endl << my_sim.results_native() << std::endl;
  std::cout << "SPAM-corrected Results:" << std::endl << my_sim.results() << std::endl;

}
