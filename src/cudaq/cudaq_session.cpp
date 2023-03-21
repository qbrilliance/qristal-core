#include "qb/core/session.hpp"

// CUDAQ
#include "qoda/algorithms/sample.h"
#include "qoda/qpud_client.h"

namespace qb {
void session::run_cudaq(size_t ii, size_t jj, const run_i_j_config &run_config) {
  auto [cudaq_kernel_name, cudaq_kernel_functor] = cudaq_kernels_.at(ii);
  if (debug_) {
    std::cout << "[debug]: Executing CUDAQ kernel at "
              << "[location: " << ii << ", condition: " << jj
              << "]: Kernel name: " << cudaq_kernel_name << std::endl;
  }
  auto &platform = cudaq::get_platform();
  const auto shots = run_config.num_shots;
  auto cudaq_context = std::make_unique<cudaq::ExecutionContext>("sample", shots);
  platform.set_exec_ctx(cudaq_context.get());
  // Launch the kernel
  xacc::ScopeTimer timer_for_qpu(
      "Walltime, in ms, for simulator to execute quantum circuit", false);
  cudaq_kernel_functor();
  const double xacc_scope_timer_qpu_ms = timer_for_qpu.getDurationMs();
  // Release the execution context
  // IMPORTANT: we can only access the result in the context after it has been
  // released!
  platform.reset_exec_ctx();
  // Retrieve the measure counts
  auto cudaq_counts = cudaq_context->result;
  if (debug_) {
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
  acc_uses_lsbs_.at(ii).at(jj) = true;
  // Z expectation value
  const double z_expectation_val = cudaq_counts.exp_val_z();

  if (debug_) {
    std::cout << "* Z-operator expectation value: " << z_expectation_val
              << std::endl;
  }

  // Save Z-operator expectation value to VectorMapND
  ND res_z{{0, z_expectation_val}};
  out_z_op_expects_.at(ii).at(jj) = res_z;

  // Save the counts to VectorMapNN in out_counts_ and raw map data in
  // out_raws
  populate_measure_counts_data(ii, jj, cudaq_counts.to_map());
}
} // namespace qb
