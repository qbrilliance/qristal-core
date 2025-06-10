// (c) 2021 Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>
#include <qristal/core/jensen_shannon.hpp>
#include <gtest/gtest.h>

TEST(TestJensenShannonDivergence, maximum_divergence) {
  auto s = qristal::session();
  s.acc = "qpp";
  s.qn = 2;
  s.sn = 1000;

  // Set the expected amplitudes
  std::map<std::vector<bool>, std::complex<double>> amp;
  amp[{0,0}] = pow(2,-0.5);
  amp[{0,1}] = 0;
  amp[{1,0}] = pow(2,-0.5);
  amp[{1,1}] = 0;

  // Set the target circuit
  s.instring = R"(
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

  // Run the circuit
  s.run();
  std::cout << "bits (high..low) : counts" << std::endl;
  for (const auto& [bits, count] : s.results()) {
    for (size_t i = bits.size(); i-- > 0; ) std::cout << bits[i];
    std::cout << ": " << count << std::endl;
  }

  // Calculate Jensen-Shannon divergence
  double divergence = qristal::jensen_shannon(s.results(), amp);
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_NEAR(divergence, std::log(2), 2e-3);
}

TEST(TestJensenShannonDivergence, minimum_divergence) {
  auto s = qristal::session();
  s.acc = "qpp";
  s.qn = 2;
  s.sn = 1000;

  // The expected amplitudes
  std::map<std::vector<bool>, std::complex<double>> amp;
  amp[{0,0}] = 0;
  amp[{0,1}] = pow(2,-0.5);
  amp[{1,0}] = 0;
  amp[{1,1}] = pow(2,-0.5);

  // Set the target circuit
  s.instring = R"(
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

  // Run the circuit
  s.run();
  std::cout << "bits (high..low) : counts" << std::endl;
  for (const auto& [bits, count] : s.results()) {
    for (size_t i = bits.size(); i-- > 0; ) std::cout << bits[i];
    std::cout << ": " << count << std::endl;
  }

  // Calculate Jensen-Shannon divergence
  double divergence = qristal::jensen_shannon(s.results(), amp);
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_NEAR(divergence, 0, 2e-3);
}

TEST(TestJensenShannonDivergence, simple) {
  auto s = qristal::session();
  s.acc = "qpp";
  s.qn = 2;
  s.sn = 1000;

  // Set the expected amplitudes
  std::map<std::vector<bool>, std::complex<double>> amp;
  amp[{0,0}] = pow(2,-0.5);
  amp[{0,1}] = 0;
  amp[{1,0}] = 0;
  amp[{1,1}] = pow(2,-0.5);

  // Set the target circuit
  s.instring = R"(
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

  // Run the circuit
  s.run();
  std::cout << "bits (high..low) : counts" << std::endl;
  for (const auto& [bits, count] : s.results()) {
    for (size_t i = bits.size(); i-- > 0; ) std::cout << bits[i];
    std::cout << ": " << count << std::endl;
  }

  // Calculate Jensen-Shannon divergence
  double divergence = qristal::jensen_shannon(s.results(), amp);
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_GT(divergence, 0.3);
  EXPECT_LT(divergence, 0.4);
}
