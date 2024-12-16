// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/cudaq/cudaq_acc.hpp"
#include "cudaq/algorithms/sample.h"
#include "qristal/core/cudaq/ir_converter.hpp"
#include "qristal/core/cudaq/sim_pool.hpp"
#include <cudaq.h>

namespace qristal {
cudaqNoiseStruct cudaqNoise;

cudaq_acc::cudaq_acc(const std::string &backend_name)
    : m_backend(backend_name) {}
const std::string cudaq_acc::name() const { return "cudaq"; }

const std::string cudaq_acc::description() const {
  return "Wrapper to delegate XACC IR to CUDA Quantum backends.";
}

void cudaq_acc::initialize(const xacc::HeterogeneousMap &params) {
  if (!params.keyExists<int>("shots")) {
    throw std::runtime_error("'shots' is required. Please provide the number of shots to run.");
  }
  updateConfiguration(params);
}

void cudaq_acc::updateConfiguration(const xacc::HeterogeneousMap &config) {
  if (config.keyExists<int>("shots")) {
    sample_ops.shots = config.get<int>("shots");
  }

  if (config.pointerLikeExists<NoiseModel>("noise-model")) {
    NoiseModel *nm = config.getPointerLike<NoiseModel>("noise-model");
    cudaqNoise.qristal_noise_model_to_emulator = std::make_shared<NoiseModel>(*nm);
  }
  if (config.keyExists<int>("initial-bond-dim")) {
    m_initial_bond_dim = config.get<int>("initial-bond-dim");
    setEnvVar(m_initial_bond_dim, "CUDAQ_INI_BOND");
  }
  if (config.keyExists<int>("initial-kraus-dim")) {
    m_initial_kraus_dim = config.get<int>("initial-kraus-dim");
    setEnvVar(m_initial_kraus_dim, "CUDAQ_INI_KRAUS");
  }
  if (config.keyExists<int>("max-bond-dim")) {
    m_max_bond_dim = config.get<int>("max-bond-dim");
    setEnvVar(m_max_bond_dim, "CUDAQ_MAX_BOND");
  }
  if (config.keyExists<int>("max-kraus-dim")) {
    m_max_kraus_dim = config.get<int>("max-kraus-dim");
    setEnvVar(m_max_kraus_dim, "CUDAQ_MAX_KRAUS");
  }
  if (config.keyExists<double>("abs-truncation-threshold")) {
    m_abs_svd_cutoff = config.get<double>("abs-truncation-threshold");
    setEnvVar(m_abs_svd_cutoff, "CUDAQ_ABS_CUTOFF");
  }
  if (config.keyExists<double>("rel-truncation-threshold")) {
    m_rel_svd_cutoff = config.get<double>("rel-truncation-threshold");
    setEnvVar(m_rel_svd_cutoff, "CUDAQ_RELATIVE_CUTOFF");
  }
  if (config.stringExists("measurement-sampling-sequential")) {
    m_measure_sample_sequential = config.getString("measurement-sampling-sequential");
    setEnvVar(m_measure_sample_sequential, "CUDAQ_MEASURE_SAMPLING_SEQ");
  }
}

void cudaq_acc::freeEnvVars() {
  unsetenv("CUDAQ_INI_BOND");
  unsetenv("CUDAQ_INI_KRAUS");
  unsetenv("CUDAQ_MAX_BOND");
  unsetenv("CUDAQ_MAX_KRAUS");
  unsetenv("CUDAQ_ABS_CUTOFF");
  unsetenv("CUDAQ_RELATIVE_CUTOFF");
  unsetenv("CUDAQ_MEASURE_SAMPLING_SEQ");
}

const std::vector<std::string> cudaq_acc::configurationKeys() {
  return {"shots", "initial-bond-dim", "initial-kraus-dim", "max-bond-dim", "max-kraus-dim",
          "abs-truncation-threshold", "rel-truncation-threshold", "measurement-sampling-sequential",
          "noise-model"};
}

void cudaq_acc::execute(
    std::shared_ptr<xacc::AcceleratorBuffer> buffer,
    const std::shared_ptr<xacc::CompositeInstruction> program) {
  qristal::cudaq_ir_converter converter(program);
  auto &cudaq_builder = converter.get_cudaq_builder();
  assert(xacc::container::contains(
      cudaq_sim_pool::get_instance().available_simulators(), m_backend));
  // Set the simulator
  cudaq_sim_pool::get_instance().set_simulator(m_backend);
  auto cudaq_counts =
      cudaq::sample(sample_ops, cudaq_builder, std::vector<double>{});
  freeEnvVars();
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
}
