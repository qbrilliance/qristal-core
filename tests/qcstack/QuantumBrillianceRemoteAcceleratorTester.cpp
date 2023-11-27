// Copyright Quantum Brilliance

#include "qb/core/session.hpp"
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <chrono>
using json = nlohmann::json;

TEST(QuantumBrillianceRemoteAcceleratorTester, testInstantiation) {
  int shots = 1024;
  int n_qubits = 2;
  
  // A QCStack client - provide argument 'true' for debug mode
  std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>(true);

  std::shared_ptr<xacc::quantum::QuantumBrillianceRemoteAccelerator> tqdk =
      std::make_shared<xacc::quantum::QuantumBrillianceRemoteAccelerator>(qcs_qdk,true);
  EXPECT_EQ(shots, 1024);
}

TEST(QuantumBrillianceRemoteAcceleratorTester, testInstantiationGetDetails) {
  int shots = 256;
  int n_qubits = 4;
  std::vector<int> init_qubits(n_qubits, 0);
  
  // A QCStack client - provide argument 'true' for debug mode
  std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>(true);

  std::shared_ptr<xacc::quantum::QuantumBrillianceRemoteAccelerator> tqdk =
      std::make_shared<xacc::quantum::QuantumBrillianceRemoteAccelerator>(qcs_qdk,true);
  std::cout << "* Signature: " << tqdk->getSignature() << std::endl;
  EXPECT_EQ(tqdk->getSignature(), "qb-qdk:");
  std::vector<std::string> config_qb_qdk = tqdk->configurationKeys();
  xacc::HeterogeneousMap mm = tqdk->getProperties();

  std::cout << "* Keys:"
            << "\n";
  for (std::string cel : config_qb_qdk) {
    std::cout << "    " << cel << std::endl;
  }

  mm.insert("remote_url", std::string("https://myuser23:myPass*23@5795-13-211-253-224.au.ngrok.io/api/v1/"));
  mm.insert("post_path", std::string(""));
  mm.insert("shots", shots);
  mm.insert("init", init_qubits);

  if (mm.keyExists<std::string>("command")) {
    std::cout << "* command_ = " << mm.get<std::string>("command") << std::endl;
  }
  if (mm.keyExists<std::vector<int>>("init")) {
    std::cout << "* init_ =  "
              << "\n";
    for (int elein : mm.get<std::vector<int>>("init")) {
      std::cout << "    " << elein << "\n";
    }
  }
  if (mm.keyExists<int>("shots")) {
    std::cout << "* shots_ = " << mm.get<int>("shots") << std::endl;
  }

  if (mm.keyExists<std::string>("results")) {
    std::cout << "* results_ = " << mm.get<std::string>("results") << std::endl;
  }
  if (mm.keyExists<std::string>("hwbackend")) {
    std::cout << "* hwbackend_ = " << mm.get<std::string>("hwbackend")
              << std::endl;
  }
  if (mm.keyExists<std::string>("remote_url")) {
    std::cout << "* remoteUrl = " << mm.get<std::string>("remote_url")
              << std::endl;
  }
  if (mm.keyExists<std::string>("post_path")) {
    std::cout << "* postPath = " << mm.get<std::string>("post_path")
              << std::endl;
  }

  // Update configuration of the remote accelerator
  tqdk->updateConfiguration(mm);

  // Read the configuration back and check against sent values
  xacc::HeterogeneousMap mm2 = tqdk->getProperties();
  ASSERT_EQ(mm.get<std::string>("post_path"),
            mm2.get<std::string>("post_path"));
  ASSERT_EQ(mm.get<int>("shots"), mm2.get<int>("shots"));
  ASSERT_EQ(mm.get<std::vector<int>>("init"),
            mm2.get<std::vector<int>>("init"));

  // Create a test quantum circuit
  // Allocate some qubits
  auto buffer = xacc::qalloc(2);
  // Compile
  auto quil = xacc::getCompiler("quil");
  std::string tcct = R"(__qpu__ void bell(qbit q) {
RX(pi) 0
RY(pi) 1
RX(pi/16) 0
RY(pi/16) 1
RX(-pi/32) 0
RY(-pi/32) 1
RX(1.0101) 0
RY(1.0101) 1
MEASURE 0 [0]
MEASURE 1 [1]
})";
  auto ir = quil->compile(tcct, tqdk);

  json retresult = json::parse(tqdk->processInput(buffer, ir->getComposites()));
  std::cout << "* Input quantum circuit: " << tcct << std::endl;
  std::cout << "* Processed input into: " << retresult.dump(4) << std::endl;

  // Validate support on QB hardware for session settings
  // tqdk->validate_capability();

  // Proceed to RemoteAccelerator::execute(buffer, ir->getComposites())
  tqdk->execute(buffer, ir->getComposites());
  std::cout << "* HTTP POST done..." << std::endl;

  // Delay until it is time to poll for results
  std::cout << "* Waiting..." << std::endl;
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(std::chrono::seconds(15));
  std::cout << "* Waited 15 seconds..." << std::endl;
  std::cout << "* Start to poll hardware to retrieve results..." << std::endl;

  // Poll with HTTP GET
  std::map<std::string, int> out_counts;
  int polling_interval_when_recursive = 5;
  int polling_attempts_when_recursive = 10;
  int poll_return = tqdk->pollForResults(buffer, ir->getComposites(), out_counts, polling_interval_when_recursive, polling_attempts_when_recursive);
  std::cout << "* Polling returned: " << poll_return << std::endl;
  ASSERT_EQ(poll_return, 0);
}
