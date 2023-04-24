// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
// Forward declare
namespace nvqir {
class CircuitSimulator;
}

namespace qb {
/// @brief Singleton util class holding/caching references to CUDAQ backend simulator instances.
// Rationale: 
// - CudaQ QIR backends are implemented in dynamic libs (libnvqir-<backend name>.so).
//   All have C-API hooks:  getCircuitSimulator() and getCircuitSimulator_<backend name>() to retrieve a pointer to the static instance of that backend.
//   (see NVQIR_REGISTER_SIMULATOR macro in nvqir/CircuitSimulator.h)
// - In static link-time use case, when only one of these backend libs, e.g., libnvqir-custatevec.so, could be linked to the executable,
//   and the CUDAQ runtime will load it via dlsym lookup in itself (dlopen(NULL)), see getUniquePluginInstance() util function.
// - In qb::session use case, we want to be able to dynamically select one of these backends; hence 
//   following the same procedure in getUniquePluginInstance but referring to the appropriate libnvqir-<backend name>.so when doing dlopen.
// - The CUDAQ runtime backend is set by the C-API: __nvqir__setCircuitSimulator()
// - This pool will cache pointers to the static instances that it has retrieved before,
//   as well as tracking the active CUDAQ backend simulator (i.e., no need to set it again if it is already the active one).
// - Adapted from CUDAQ Python binding implementation (cuda-quantum/python)
class cudaq_sim_pool {
public:
  /// @brief Returns the names of the available CUDAQ simulators
  std::vector<std::string> available_simulators() const;

  /// @brief Sets the 'active' CUDAQ simulator backend.
  /// This will throw if this is not a valid name (i.e., one of
  /// `available_simulators`)
  void set_simulator(const std::string &name);
  /// Getter for the instance; makes this class a threadsafe singleton
  static cudaq_sim_pool &get_instance();

private:
  /// Constructor
  cudaq_sim_pool();

  /// Destructor
  ~cudaq_sim_pool();

private:
  std::unordered_map<std::string, std::string> sim_name_to_lib;
  // Sim instance pool (lazily populated)
  std::unordered_map<std::string, nvqir::CircuitSimulator *> sim_name_to_sim_ptr;
  std::string active_sim;
};

} // namespace qb