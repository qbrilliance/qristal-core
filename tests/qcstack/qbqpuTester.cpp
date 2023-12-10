// Copyright Quantum Brilliance

#include "qb/core/session.hpp"
#include "qb/core/backends/qb_hardware/qb_qpu.hpp"
#include "qb/core/backends/qb_hardware/qcstack_client.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <chrono>
using json = nlohmann::json;

TEST(qbqpuTester, testInstantiation)
{
  // A QCStack client - provide argument 'true' for debug mode
  std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>(true);
  auto tqdk = std::make_shared<xacc::quantum::qb_qpu>(qcs_qdk, true);
  std::cout << "* Signature: " << tqdk->getSignature() << std::endl;
  EXPECT_EQ(tqdk->getSignature(), "QB-hardware:");
}

TEST(qbqpuTester, testInstantiationGetDetails)
{
  int shots = 256;
  int n_qubits = 4;
  std::vector<int> init_qubits(n_qubits, 0);
  
  // A QCStack client - provide argument 'true' for debug mode
  std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>(true);
  auto tqdk = std::make_shared<xacc::quantum::qb_qpu>(qcs_qdk, true);

  std::vector<std::string> config_qb_qdk = tqdk->configurationKeys();
  xacc::HeterogeneousMap mm = tqdk->getProperties();

  std::cout << "* Keys:" << "\n";
  for (std::string cel : config_qb_qdk) std::cout << "    " << cel << std::endl;

  char* val = std::getenv("QCSTACK_TEST_SERVER_URL");
  if (val == NULL) throw std::runtime_error("Environment variable QCSTACK_TEST_SERVER_URL not set.");
  std::string url = std::string(val);
  mm.insert("url", url.back() == '/' ? url : url + '/');
  mm.insert("post_path", std::string(""));
  mm.insert("shots", shots);
  mm.insert("init", init_qubits);

  // Simple checker and printer
  auto printout = [&](const std::vector<std::string>& vec)
  { 
    for (auto s : vec)
      if (mm.keyExists<std::string>(s)) std::cout << "* " << s << ": " << mm.get<std::string>(s) << std::endl;
  };
  printout({"shots", "results", "hwbackend", "url", "post_path"});
  if (mm.keyExists<std::vector<int>>("init"))
  {
    std::cout << "* init_: " << "\n";
    for (int elein : mm.get<std::vector<int>>("init")) std::cout << "    " << elein << "\n";
  }

  // Update configuration of the remote accelerator
  tqdk->updateConfiguration(mm);

  // Read the configuration back and check against sent values
  xacc::HeterogeneousMap mm2 = tqdk->getProperties();
  ASSERT_EQ(mm.get<std::string>("post_path"), mm2.get<std::string>("post_path"));
  ASSERT_EQ(mm.get<int>("shots"),             mm2.get<int>("shots"));
  ASSERT_EQ(mm.get<std::vector<int>>("init"), mm2.get<std::vector<int>>("init"));

  // Create a test quantum circuit
  // Allocate some qubits
  auto buffer = xacc::qalloc(2);
  // Compile
  xacc::Initialize();
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
  tqdk->validate_capability();

  // Proceed to RemoteAccelerator::execute(buffer, ir->getComposites())
  tqdk->execute(buffer, ir->getComposites());
  std::cout << "* HTTP POST done..." << std::endl;

  // Delay until it is time to poll for results
  std::cout << "* Waiting..." << std::endl;
  //using namespace std::chrono_literals;
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
