// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include <string>
#include <iostream>
#include <cudaq.h>

// Define a quantum kernel with CUDAQ at compile time.
template<std::size_t N>
struct ghz {
  auto operator()() __qpu__ {
    cudaq::qarray<N> q;
    h(q[0]);
    for (int i = 0; i < N - 1; i++) {
      x<cudaq::ctrl>(q[i], q[i + 1]);
    }
    mz(q);
  }
};

int main()
{
  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  // Number of qubits we want to run
  constexpr int NB_QUBITS = 20;

  // Choose a CUDAQ simulator backend, e.g., qpp
  my_sim.set_acc("cudaq:qpp");

  // Add CUDAQ ghz kernel to the current session
  my_sim.set_cudaq_kernel(ghz<NB_QUBITS>{});

  // Set up sensible default parameters
  my_sim.init();

  // Add more qubits
  my_sim.set_qn(NB_QUBITS);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);

  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results
  std::cout << "Results:" << std::endl << my_sim.results()[0][0] << std::endl;
}
