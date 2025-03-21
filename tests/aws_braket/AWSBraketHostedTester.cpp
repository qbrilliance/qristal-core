// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/cmake_variables.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <pybind11/embed.h>
#include <gtest/gtest.h>
#include "yaml-cpp/yaml.h"


// add job handler
std::shared_ptr<qristal::async_job_handle> run_async_internal(qristal::session& s, const std::size_t i, const std::size_t j) {
  std::shared_ptr<xacc::Accelerator> qpu(s.get_executor().getNextAvailableQpu());
  auto handler = s.run_async(i, j, qpu);
  return handler;
};

TEST(AWSBraketHostedTester, Simple) {
  std::cout << "Executing AWSBraketHosted C++ test" << std::endl;
  // Create a simple Bell state circuit.
  qristal::CircuitBuilder my_circuit;
  my_circuit.H(0);
  my_circuit.CNOT(0, 1);
  my_circuit.MeasureAll(2);
  pybind11::scoped_interpreter guard{};
  // Start a Qristal session
  auto s = qristal::session(false);
  // 2 qubits, 100 shots
  s.set_qn(2);
  s.set_sn(100);
  // Set the input circuit
  s.set_irtarget_m(my_circuit.get());
  // Set up AWS defaults with 32 workers
  s.aws_setup(32);
  // Load the AWS settings from the remote backends file, make a copy for each accelerator, run and print results
  auto db = YAML::LoadFile("remote_backends.yaml");
  for (const std::string x : {"SV1", "DM1", "TN1"}) //"Rigetti"}) // Rigetti commented out as devices are not available on Braket at the time of writing.
  {
    db["aws-braket"]["device"] = x;
    std::ofstream fout("remote_backends_"+x+".yaml");
    fout << db << std::endl;
    s.set_remote_backend_database_path("remote_backends_"+x+".yaml");
    auto handler = run_async_internal(s, 0, 0);
    while (!handler->done()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
    // Get results
    std::cout << x << " ran successfully!" << std::endl;
    std::cout << s.results()[0][0] << std::endl;
  }
}
