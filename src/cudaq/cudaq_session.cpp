// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>
#include <qristal/core/cudaq/sim_pool.hpp>
// CUDAQ
#include <cudaq/algorithms/sample.h>

namespace qristal {

  void session::run_cudaq() {
    auto &cudaq_sim_pool = cudaq_sim_pool::get_instance();
    const auto cudaq_sims = cudaq_sim_pool.available_simulators();
    if (std::find(cudaq_sims.begin(), cudaq_sims.end(), acc) == cudaq_sims.end()) {
      // The requested simulator is not valid for using with CUDAQ kernel.
      std::stringstream msg;
      msg << "The requested accelerator (" << acc
          << ") is not compatible with CUDA Quantum kernels." << std::endl
          << "Available CUDAQ simulators are:" << std::endl;
      for (const auto &sim_name : cudaq_sims) msg << "  - " << sim_name << std::endl;
      throw std::invalid_argument(msg.str());
    }
    if (debug) std::cout << "[debug]: Executing CUDAQ kernel with backend " << acc << "." << std::endl;

    // Set the simulator
    cudaq_sim_pool.set_simulator(acc);
    auto &platform = cudaq::get_platform();
    auto cudaq_context = std::make_unique<cudaq::ExecutionContext>("sample", sn);
    platform.set_exec_ctx(cudaq_context.get());

    // Launch the kernel
    xacc::ScopeTimer timer_for_qpu("Walltime, in ms, for simulator to execute quantum circuit", false);
    cudaq_kernel();
    const double xacc_scope_timer_qpu_ms = timer_for_qpu.getDurationMs();

    // Release the execution context.
    // IMPORTANT: we can only access the result in the context after it has been released!
    platform.reset_exec_ctx();

    // Retrieve the measure counts
    auto& cudaq_counts = cudaq_context->result;
    if (debug) {
      std::cout << std::endl;
      // Print out data
      std::cout << "Bit string data: " << std::endl;
      for (const auto &[bits, count] : cudaq_counts) {
        printf("Observed: %s, %lu\n", bits.data(), count);
      }
      std::cout << std::endl;
      std::cout << "Walltime elapsed for CUDAQ to perform the "
                   "requested number of shots of the quantum circuit, in ms: "
                << xacc_scope_timer_qpu_ms << std::endl;
      std::cout << std::endl;
    }

    // CUDAQ reports bitstrings in LSB
    acc_outputs_qbit0_left_ = true;

    // Z expectation value
    z_op_expectation_ = cudaq_counts.exp_val_z();
    if (debug) std::cout << "* Z-operator expectation value: " << z_op_expectation_ << std::endl;

    // Save the counts
    populate_measure_counts_data(cudaq_counts.to_map());
  }

}
