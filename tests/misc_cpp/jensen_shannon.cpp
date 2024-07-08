// (c) 2021 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "qb/core/session.hpp"

TEST(TestJensenShannonDivergence, maximum_divergence) {
  auto s = qb::session(false);
  s.init();
  s.set_acc("qpp");
  s.set_qn(2);
  s.set_sn(1000);

  // Set the expected amplitudes
  std::map<std::string, std::complex<double>> output_amplitudes;
  output_amplitudes["00"] = pow(2,-0.5);
  output_amplitudes["01"] = pow(2,-0.5);
  output_amplitudes["10"] = 0;
  output_amplitudes["11"] = 0;
  s.set_output_amplitude(output_amplitudes);

  // Set the target circuit
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      x q[1];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    )";
  s.set_instring(targetCircuit);

  // Run the circuit
  s.run();

  // Calculate Jensen-Shannon divergence
  s.get_jensen_shannon();
  std::vector<std::vector<std::map<int, double>>> jsv = s.get_out_divergences();  
  double divergence = jsv[0][0][0];
  std::cout << s.get_out_raws_json()[0][0] << std::endl;
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_NEAR(divergence, std::log(2), 1e-3);
}

TEST(TestJensenShannonDivergence, minimum_divergence) {
  auto s = qb::session(false);
  s.init();
  s.set_acc("qpp");
  s.set_qn(2);
  s.set_sn(1000);

  // Set the expected amplitudes
  std::map<std::string, std::complex<double>> output_amplitudes;
  output_amplitudes["00"] = 0;
  output_amplitudes["01"] = 0;
  output_amplitudes["10"] = pow(2,-0.5);
  output_amplitudes["11"] = pow(2,-0.5);
  s.set_output_amplitude(output_amplitudes);

  // Set the target circuit
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      x q[1];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    )";
  s.set_instring(targetCircuit);

  // Run the circuit
  s.run();

  // Calculate Jensen-Shannon divergence
  s.get_jensen_shannon();
  std::vector<std::vector<std::map<int, double>>> jsv = s.get_out_divergences();  
  double divergence = jsv[0][0][0];
  std::cout << s.get_out_raws_json()[0][0] << std::endl;
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_NEAR(divergence, 0, 1e-3);
}

TEST(TestJensenShannonDivergence, simple) {
  auto s = qb::session(false);
  s.init();
  s.set_acc("qpp");
  s.set_qn(2);
  s.set_sn(1000);

  // Set the expected amplitudes
  std::map<std::string, std::complex<double>> output_amplitudes;
  output_amplitudes["00"] = pow(2,-0.5);
  output_amplitudes["01"] = 0;
  output_amplitudes["10"] = 0;
  output_amplitudes["11"] = pow(2,-0.5);
  s.set_output_amplitude(output_amplitudes);

  // Set the target circuit
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      x q[1];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    )";
  s.set_instring(targetCircuit);

  // Run the circuit
  s.run();

  // Calculate Jensen-Shannon divergence
  s.get_jensen_shannon();
  std::vector<std::vector<std::map<int, double>>> jsv = s.get_out_divergences();  
  double divergence = jsv[0][0][0];
  std::cout << s.get_out_raws_json()[0][0] << std::endl;
  std::cout << "divergence: " << divergence << std::endl;
  EXPECT_GT(divergence, 0.3);
  EXPECT_LT(divergence, 0.4);
}
