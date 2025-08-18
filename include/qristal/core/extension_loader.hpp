// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

// STL
#include <cstddef>
#include <vector>

namespace qristal {

  /// Attempt to load and return pointer to the emulator plugin library
  void* emulator_library();

  /// Whether or not the emulator plugin has been compiled with GPU support
  bool emulator_supports_gpus();

  /// The number of GPUs available on the system for use with the emulator plugin
  size_t available_gpus();

  /// Check if a proposed set of GPU device IDs is valid on the current system.
  void check_gpu_device_ids(const std::vector<size_t>& gpu_device_ids);

}
