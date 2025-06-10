// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/thread_pool.hpp>

#include <string>
#include <iostream>
#include <future>
#include <chrono>
#include <thread>

// Add job handler
std::string run_async(qristal::session& s) {
  s.run();
  std::ostringstream out;
  for (const auto& [bits, count] : s.results()) {
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

  // Create QFT circuit
  qristal::CircuitBuilder circ;
  std::vector<int> qft_qubits(num_qubits);
  // Fill the qubit list with 0, 1, ..., n-1
  // i.e., the qubits that we want to apply the QFT circuit to.
  std::iota(std::begin(qft_qubits), std::end(qft_qubits), 0);
  // Apply QFT
  circ.QFT(qft_qubits);
  circ.MeasureAll(num_qubits);

  // Make four Qristal sessions corresponding to the 4 backends we want to run: aer:matrix_product state, aer:density_matrix, cudaq:dm & tnqvm
  constexpr size_t num_jobs = 4;
  std::vector<std::future<std::string>> futures{};
  std::vector<qristal::session> my_sims(num_jobs);
  my_sims[0].acc = "aer";
  my_sims[0].aer_sim_type = "matrix_product_state";
  my_sims[1].acc = "aer";
  my_sims[1].aer_sim_type = "density_matrix";
  my_sims[2].acc = "cudaq:dm";
  my_sims[3].acc = "tnqvm";
  for (auto& sim : my_sims) {
    sim.gpu_device_ids = {0};
    sim.qn = num_qubits;
    sim.sn = num_shots;
    sim.aer_omp_threads = aer_instance_thread_limit;
    sim.irtarget = circ.get();
     // Run async jobs
    futures.push_back(qristal::thread_pool::submit(run_async, std::ref(sim)));
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
