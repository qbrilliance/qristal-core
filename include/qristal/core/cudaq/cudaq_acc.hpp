// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include "Accelerator.hpp"
#include "qristal/core/utils.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include <cudaq.h>

namespace qristal {
struct cudaqNoiseStruct {
  std::optional<std::shared_ptr<NoiseModel>> qristal_noise_model_to_emulator = std::nullopt;
};
extern cudaqNoiseStruct cudaqNoise;

/// A xacc::Accelerator wrapper for offloading XACC IR execution to CUDA Quantum
/// simulator backends by converting XACC IR to Quake then QIR.
class cudaq_acc : public xacc::Accelerator {
  /// Name of the CUDAQ backend to use
  std::string m_backend;
  /// Number of measurement shots
  int m_shots;
  /// Initial bond dimension
  int m_initial_bond_dim;
  /// Initial Kraus dimension
  int m_initial_kraus_dim;
  /// Maximum bond dimension
  int m_max_bond_dim;
  /// Maximum Kraus dimension
  int m_max_kraus_dim;
  /// Singular values absolute cutoff threshold
  double m_abs_svd_cutoff;
  /// Singular values relative cutoff threshold
  double m_rel_svd_cutoff;
  /// Measurement sampling method
  std::string m_measure_sample_method; 
  /// CudaQ's struct containing sampling function's members: number of shots and noise model
  cudaq::sample_options sample_ops;
  /// GPU device IDs
  std::vector<size_t> m_gpu_device_id; 

public:
  /// Constructor
  cudaq_acc(const std::string &backend_name);
  /// Name of this xacc::Accelerator implementation
  virtual const std::string name() const override;
  /// Description of this xacc::Accelerator implementation
  virtual const std::string description() const override;
  /// Initialize this xacc::Accelerator with runtime configurations.
  virtual void initialize(const xacc::HeterogeneousMap &params) override;
  /// Update runtime configurations of this xacc::Accelerator and set the cudaQ environment variables
  virtual void updateConfiguration(const xacc::HeterogeneousMap &config) override;
  /// Free the CudaQ environment variables set in updateConfigurations()
  void freeEnvVars();
  /// List of configuration keys that this xacc::Accelerator will look for.
  virtual const std::vector<std::string> configurationKeys() override;
  /// Execute a single circuit and persist the measurement results to the
  /// buffer.
  virtual void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                       const std::shared_ptr<xacc::CompositeInstruction>
                           CompositeInstruction) override;
  /// Execute a list of circuits and persist the measurement results to the
  /// buffer.
  virtual void
  execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
              CompositeInstructions) override;

  /// Helper function to convert the CudaQ environment variables to const char*
  template <typename T>
  void setEnvVar(T param, const char* env_var_name) {
    std::string param_str;
    if constexpr (std::is_same_v<T, int>) {
      param_str = std::to_string(param);
    } else if constexpr (std::is_same_v<T, double>) {
      param_str = double_to_string(param, 16);
    } else if constexpr (std::is_same_v<T, std::string>) {
      param_str = param;
    }
    const char *param_char = param_str.c_str();
    setenv(env_var_name, param_char, true);

    if constexpr (std::is_same_v<T, std::vector<size_t>>) {
      std::stringstream param_str_vec;
      std::copy(param.begin(), param.end(), std::ostream_iterator<int>(param_str_vec, " "));
      const std::string tmp = param_str_vec.str();
      const char *param_char = tmp.c_str();
      setenv(env_var_name, param_char, true);
    }
  }
};
}
