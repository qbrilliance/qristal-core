// Copyright Quantum Brilliance

#include "qristal/core/session.hpp"
#include "qristal/core/backends/qb_hardware/qb_qpu.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <chrono>
using json = nlohmann::json;

TEST(qbqpuTester, testInstantiation)
{
  auto hardware_device = std::make_shared<xacc::quantum::qb_qpu>("test_qbqpu", true);
  std::cout << "* Signature: " << hardware_device->getSignature() << std::endl;
  EXPECT_EQ(hardware_device->getSignature(), "test_qbqpu:");
}

TEST(qbqpuTester, testInstantiationGetDetails)
{
  int delay = 20;
  int shots = 256;
  double poll_secs = 5;
  uint poll_retries = 10;
  int n_qubits = 4;
  std::vector<uint> init_qubits(n_qubits, 0);

  auto hardware_device = std::make_shared<xacc::quantum::qb_qpu>("test_qbqpu", true);
  std::vector<std::string> config_qb_qdk = hardware_device->configurationKeys();
  xacc::HeterogeneousMap mm = hardware_device->getProperties();

  std::cout << "* Keys:" << "\n";
  for (std::string cel : config_qb_qdk) std::cout << "    " << cel << std::endl;

  YAML::Node db = YAML::LoadFile("remote_backends.yaml");
  const std::string url = db["example_hardware_device"]["url"].as<std::string>();
  mm.insert("url", url.back() == '/' ? url : url + '/');
  mm.insert("shots", shots);
  mm.insert("poll_secs", poll_secs);
  mm.insert("poll_retries", poll_retries);
  mm.insert("init", init_qubits);
  mm.insert("exclusive_access", false);
  mm.insert("use_default_contrast_settings", false);
  mm.insert("init_contrast_threshold", double(0));
  std::map<int,double> qubit_contrast_thresholds = {{0,0},{1,0}};
  mm.insert("qubit_contrast_thresholds", qubit_contrast_thresholds);

  // Simple checker and printer
  std::cout << "* shots: " << mm.get<int>("shots") << std::endl;
  for (auto s : {"results", "url"}) std::cout << "* " << s << ": " << mm.get<std::string>(s) << std::endl;
  if (mm.keyExists<std::vector<uint>>("init"))
  {
    std::cout << "* init_: " << "\n";
    for (int elein : mm.get<std::vector<uint>>("init")) std::cout << "    " << elein << "\n";
  }

  // Update configuration of the remote accelerator
  hardware_device->updateConfiguration(mm);

  // Read the configuration back and check against sent values
  xacc::HeterogeneousMap mm2 = hardware_device->getProperties();
  ASSERT_EQ(mm.get<int>("shots"), mm2.get<int>("shots"));
  ASSERT_EQ(mm.get<std::vector<uint>>("init"), mm2.get<std::vector<uint>>("init"));

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
  auto ir = quil->compile(tcct, hardware_device);

  json retresult = json::parse(hardware_device->processInput(buffer, ir->getComposites()));
  std::cout << "* Input quantum circuit: " << tcct << std::endl;
  std::cout << "* Processed input into: " << retresult.dump(4) << std::endl;

  // Set up QB hardware
  hardware_device->setup_hardware(true);

  // Proceed to RemoteAccelerator::execute(buffer, ir->getComposites())
  hardware_device->execute(buffer, ir->getComposites(), true);
  std::cout << "* HTTP POST done..." << std::endl;

  // Delay until it is time to poll for results
  std::cout << "* Waiting..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(delay));
  std::cout << "* Waited " << delay << " seconds..." << std::endl;
  std::cout << "* Start to poll hardware to retrieve results..." << std::endl;

  // Poll with HTTP GET
  std::map<std::string, int> counts = hardware_device->poll_for_results();
  int sum = std::accumulate(std::begin(counts), std::end(counts), 0,
   [](const int previous, const auto& el) { return previous + el.second; });
  ASSERT_EQ(sum, shots);
}
