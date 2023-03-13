// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <string>
#include <iostream>
#include <qoda.h>

// Define a quantum kernel with QODA
struct ghz {
  auto operator()(const int N) __qpu__ {
    // Dynamic, vector-like qreg
    qoda::qreg q(N);
    h(q[0]);
    for (int i = 0; i < N - 1; i++) {
      x<qoda::ctrl>(q[i], q[i + 1]);
    }
    mz(q);
  }
};

int main()
{
  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a QB SDK session
  auto my_sim = qb::session(true);
  
  // Number of qubits we want to run
  constexpr int NB_QUBITS = 4;
  
  // Add QODA ghz kernel to the current session
  my_sim.set_qoda_kernel(ghz{}, NB_QUBITS);
  
  // Set up sensible default parameters
  my_sim.qb12();

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(200);
  
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results 
  std::cout << "Results:" << std::endl << my_sim.get_out_raws()[0][0] << std::endl;
}
