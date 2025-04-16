// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/thread_pool.hpp"

#include <string>
#include <iostream>
#include <future>
#include <chrono>
#include <thread>

// Add job handler
std::string run_async_internal(qristal::session& s, const std::size_t i, const std::size_t j) {
  std::shared_ptr<xacc::Accelerator> qpu(s.get_executor().getNextAvailableQpu());
  s.run_async(i, j, qpu);
  s.get_executor().release(std::move(qpu));
  std::ostringstream out;
  for (const auto& [bits, count] : s.results()[i][j]) {
    for (size_t i = bits.size(); i-- > 0; ) out << bits[i];
    out << ": " << count << std::endl;
  }

  return out.str();
};

int main()
{
  constexpr int num_qubits = 5;
  constexpr int num_shots = 1024;
  constexpr int aer_instance_thread_limit = 4;

  // Make a Qristal session
  auto my_sim = qristal::session(false);
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(num_shots);
  my_sim.set_aer_omp_threads(aer_instance_thread_limit);
  my_sim.set_gpu_device_id({0});

  // Set up sensible default parameters
  my_sim.init();

  size_t num_jobs = 4; // 4 jobs corresponding to the 4 backends we want to run: aer:matrix_product state, aer:density_matrix, cudaq:dm & tnqvm
  std::stringstream async_workers;
  async_workers << "{\"accs\": [";
  async_workers << "{\"acc\": \"aer\"},";
  async_workers << "{\"acc\": \"aer\"},";
  async_workers << "{\"acc\": \"cudaq:dm\"},";
  async_workers << "{\"acc\": \"tnqvm\"}], ";
  async_workers << "\"sim_types\": [";
  async_workers << "{\"sim_type\": \"matrix_product_state\"},";
  async_workers << "{\"sim_type\": \"density_matrix\"}]}";
  my_sim.set_parallel_run_config(async_workers.str());

  // Create QFT circuit
  qristal::CircuitBuilder circ;
  std::vector<int> qft_qubits(num_qubits);
  // Fill the qubit list with 0, 1, ..., n-1
  // i.e., the qubits that we want to apply the QFT circuit to.
  std::iota(std::begin(qft_qubits), std::end(qft_qubits), 0);
  // Apply QFT
  circ.QFT(qft_qubits);
  circ.MeasureAll(num_qubits);

  // Hand the CircuitBuilder over to the sim object
  my_sim.set_irtarget_ms({{circ.get()}, {circ.get()}, {circ.get()}, {circ.get()}});

  // Run async jobs
  std::vector<std::vector<std::string>> instrings{num_jobs};
  std::vector<std::future<std::string>> futures{};
  for (size_t i = 0; i < num_jobs; i++) {
    futures.push_back(qristal::thread_pool::submit(run_async_internal, std::ref(my_sim), i, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::future_status status;
  std::vector<bool> finished(num_jobs, false);
  std::size_t counter{};
  std::size_t loopCounter{0};
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    counter = 0;
    std::stringstream msg;

    for (size_t i = 0; i < num_jobs; i++) {
      if (finished[i] == false) {
        status = futures[i].wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready){
          finished[i] = true;
        }
      }

      if (finished[i] == true) {
        counter++;
      }
    }
    loopCounter++;
  } while (counter < num_jobs);

  // Retrieve results from futures
  std::vector<std::string> results{};
  for (size_t i = 0; i < num_jobs; i++) {
    results.push_back(futures[i].get());
    std::cout << "Results[" << i << "]:\n" << results[i] << std::endl;
    if (results[i].empty()) {
      std::cout << "Results[" << i << "] is empty!" << std::endl;
    }
  }
}
