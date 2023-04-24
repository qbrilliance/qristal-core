// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once
#include "qb/core/cudaq/sim_pool.hpp"
#include <filesystem>
#include <iostream>
#include <link.h>
#include <ranges>
#include <regex>
#include <string.h>
#include "common/PluginUtils.h"
#include "nvqir/CircuitSimulator.h"
namespace {
// Hook to configure runtime CUDAQ NVQIR backend.
// Implemented in the CUDAQ library.
extern "C" {
void __nvqir__setCircuitSimulator(nvqir::CircuitSimulator *);
}
} // namespace
namespace qb {

/// Getter for the instance; makes this class a threadsafe singleton
cudaq_sim_pool &cudaq_sim_pool::get_instance() {
  static cudaq_sim_pool instance;
  return instance;
}

/// Constructor
cudaq_sim_pool::cudaq_sim_pool() {
  std::string cudaq_lib_path;
  const auto is_cudaq_lib = [](struct dl_phdr_info *info, size_t size,
                               void *found_path) -> int {
    if (std::string(info->dlpi_name).find("libcudaq") != std::string::npos) {
      std::filesystem::path cudaq_lib_full_path(info->dlpi_name);
      auto casted = static_cast<std::string *>(found_path);
      *casted = cudaq_lib_full_path.parent_path().string();
      return 1;
    }

    return 0;
  };
  dl_iterate_phdr(is_cudaq_lib, &cudaq_lib_path);
  if (!cudaq_lib_path.empty()) {
    // Find all the simulator libs
    for (auto const &dir_entry :
         std::filesystem::directory_iterator{cudaq_lib_path}) {
      const std::string file_name = dir_entry.path().stem();
      static const std::string SIM_LIB_NAME_PREFIX = "libnvqir-";
      if (file_name.rfind(SIM_LIB_NAME_PREFIX, 0) == 0) {
        std::string sim_name = file_name.substr(SIM_LIB_NAME_PREFIX.size());
        sim_name = std::regex_replace(sim_name, std::regex("-"), "_");
        sim_name_to_lib[sim_name] = dir_entry.path().string();
      }
    }
  }
}

std::vector<std::string> cudaq_sim_pool::available_simulators() const {
  auto kv = std::views::keys(sim_name_to_lib);
  std::vector<std::string> keys{kv.begin(), kv.end()};
  return keys;
}

void cudaq_sim_pool::set_simulator(const std::string &name) {
  const auto iter = sim_name_to_lib.find(name);
  if (iter == sim_name_to_lib.end()) {
    std::stringstream error_msg_ss;
    error_msg_ss << "The requested CUDAQ simulator '" << name
                 << "' is invalid.\n Available CUDAQ simulators are: \n";
    for (const auto &sim_name : std::views::keys(sim_name_to_lib)) {
      error_msg_ss << "  - " << sim_name << "\n";
    }
    error_msg_ss
        << "Please check your input or CUDAQ installation (e.g., did you build "
           "CUDAQ with CUSTATEVEC support?).";
    throw std::runtime_error(error_msg_ss.str());
  }

  if (active_sim != name) {
    const auto sim_instance_iter = sim_name_to_sim_ptr.find(name);
    if (sim_instance_iter != sim_name_to_sim_ptr.end()) {
      // Use the cached instance
      __nvqir__setCircuitSimulator(sim_instance_iter->second);
    } else {
      // Load the shared library and retrieve an instance

      const std::string &sim_lib = iter->second;
      auto *handle = dlopen(sim_lib.c_str(), RTLD_GLOBAL | RTLD_NOW);
      if (!handle) {
        char *error_msg = dlerror();
        throw std::runtime_error(
            "Failed to load CUDAQ NVQIR backend library: " +
            (error_msg ? std::string(error_msg) : ""));
      }

      // Clear all errors
      dlerror();
      const std::string get_sim_instance_fn = "getCircuitSimulator_" + name;
      // Load the simulator
      using func_type = nvqir::CircuitSimulator *();
      auto *get_sim_instance = reinterpret_cast<func_type *>(
          dlsym(handle, get_sim_instance_fn.c_str()));
      char *error_msg = dlerror();
      // Encounter an error:
      if (error_msg) {
        throw std::runtime_error("Failed to load function pointer to '" +
                                 get_sim_instance_fn +
                                 "': " + std::string(error_msg));
      }

      auto *simulator = get_sim_instance();
      __nvqir__setCircuitSimulator(simulator);
      // Cache it so that we don't need to load it again
      sim_name_to_sim_ptr[name] = simulator;
    }
    active_sim = name;
  }
}

/// Destructor
cudaq_sim_pool::~cudaq_sim_pool() { sim_name_to_sim_ptr.clear(); }
} // namespace qb