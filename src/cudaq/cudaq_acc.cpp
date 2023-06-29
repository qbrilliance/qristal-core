// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/cudaq/cudaq_acc.hpp"
#include "cudaq/algorithms/sample.h"
#include "qb/core/cudaq/ir_converter.hpp"
#include "qb/core/cudaq/sim_pool.hpp"

namespace qb {
cudaq_acc::cudaq_acc(const std::string &backend_name)
    : m_backend(backend_name) {}
const std::string cudaq_acc::name() const { return "cudaq"; }
const std::string cudaq_acc::description() const {
  return "Wrapper to delegate XACC IR to CUDA Quantum backends.";
}
void cudaq_acc::initialize(const xacc::HeterogeneousMap &params) {
  if (!params.keyExists<int>("shots")) {
    throw std::runtime_error(
        "'shots' is required. Please provide the number of shots to run.");
  }
  updateConfiguration(params);
}
void cudaq_acc::updateConfiguration(const xacc::HeterogeneousMap &config) {
  if (config.keyExists<int>("shots")) {
    m_shots = config.get<int>("shots");
  }
}
const std::vector<std::string> cudaq_acc::configurationKeys() {
  return {"shots"};
}
void cudaq_acc::execute(
    std::shared_ptr<xacc::AcceleratorBuffer> buffer,
    const std::shared_ptr<xacc::CompositeInstruction> program) {
  qb::cudaq_ir_converter converter(program);
  auto &cudaq_builder = converter.get_cudaq_builder();
  assert(xacc::container::contains(
      cudaq_sim_pool::get_instance().available_simulators(), m_backend));
  // Set the simulator
  cudaq_sim_pool::get_instance().set_simulator(m_backend);
  auto cudaq_counts =
      cudaq::sample(m_shots, cudaq_builder, std::vector<double>{});
  for (const auto &[bits, count] : cudaq_counts) {
    buffer->appendMeasurement(bits, count);
  }
}

void cudaq_acc::execute(
    std::shared_ptr<xacc::AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
        CompositeInstructions) {
  for (auto &f : CompositeInstructions) {
    auto tmpBuffer =
        std::make_shared<xacc::AcceleratorBuffer>(f->name(), buffer->size());
    execute(tmpBuffer, f);
    buffer->appendChild(f->name(), tmpBuffer);
  }
}
} // namespace qb
