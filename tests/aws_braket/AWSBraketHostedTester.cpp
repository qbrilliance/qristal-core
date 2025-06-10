// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/session.hpp>
#include <qristal/core/cmake_variables.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <pybind11/embed.h>
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>


TEST(AWSBraketHostedTester, Simple) {
  std::cout << "Executing AWSBraketHosted C++ test" << std::endl;
  // Create a simple Bell state circuit.
  qristal::CircuitBuilder my_circuit;
  my_circuit.H(0);
  my_circuit.CNOT(0, 1);
  my_circuit.MeasureAll(2);
  pybind11::scoped_interpreter guard{};
  // Start a Qristal session
  qristal::session s;
  // 2 qubits, 100 shots
  s.qn = 2;
  s.sn = 100;
  // Set the input circuit
  s.irtarget = my_circuit.get();
  // Use Braket
  s.acc = "aws-braket";

  // Load the AWS settings from the remote backends file, make a copy for each accelerator, run and print results
  auto db = YAML::LoadFile("remote_backends.yaml");
  for (const std::string x : {"SV1", "DM1", "TN1"}) //"Rigetti"}) // Rigetti commented out as devices are not available on Braket at the time of writing.
  {
    db["aws-braket"]["device"] = x;
    std::ofstream fout("remote_backends_"+x+".yaml");
    fout << db << std::endl;
    s.remote_backend_database_path = "remote_backends_"+x+".yaml";
    std::shared_ptr<qristal::async_job_handle> handler = s.run();
    while (!handler->done()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
    // Get results
    std::cout << x << " ran successfully!" << std::endl;
    std::cout << s.results() << std::endl;
  }
}
