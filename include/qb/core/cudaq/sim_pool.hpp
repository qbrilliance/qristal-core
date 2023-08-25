// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
// Forward declare
namespace nvqir {
class CircuitSimulator;
}

namespace qb {

/// @brief Easy loader for cudaq backends
void load_cudaq_backend(std::string name);

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

  /// @brief Manual init the CUDAQ runtime by loading its core libs with
  /// RTLD_GLOBAL.
  // This is needed for C++ -> Python bindings.
  // Rationale: Python loads native extensions (pybind11-based libs) with
  // RTLD_LOCAL, hence causing some problems for CUDAQ (e.g., the JIT engine
  // unable to find symbols from the NVQIR runtime lib, the runtime unable to
  // find its quantum platform via symbol lookup, etc.)
  // Note: this is equivalent to using `LD_PRELOAD`
  // or overriding Python dlopen behaviour with
  // `sys.setdlopenflags(os.RTLD_GLOBAL | os.RTLD_NOW)`.
  void init_cudaq_runtime();

private:
  /// Constructor
  cudaq_sim_pool();

  /// Destructor
  ~cudaq_sim_pool();

private:
  /// Simulator name to lib path
  std::unordered_map<std::string, std::string> sim_name_to_lib;
  /// Simulator instance pool (lazily populated)
  std::unordered_map<std::string, nvqir::CircuitSimulator *> sim_name_to_sim_ptr;
  /// Name of the active simulator in the CUDAQ runtime
  std::string active_sim;
  /// Path to the nvqir (i.e., libnvqir.so) lib (core CUDAQ QIR runtime implementation)
  std::string nvqir_lib_path;
  /// Path to the CUDAQ platform lib (i.e., libcudaq-platform-default.so)
  std::string platform_lib_path;
  /// Path to the CUDAQ lib (libcudaq.so)
  std::string cudaq_rt_lib_path;
};

} // namespace qb
