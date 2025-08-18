// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include "qristal/core/extension_loader.hpp"

// dlopen
#include <dlfcn.h>

// STL
#include <stdexcept>


namespace qristal {

  // Local helper function for loading functions from extension libraries
  template<typename T>
  T load(const std::string function, void* lib, const std::string libname) {
    // Clear all errors
    dlerror();
    // Attempt to get the function pointer from the shared library
    T result = reinterpret_cast<T>(dlsym(lib, function.c_str()));
    // Check that nothing went wrong
    if (dlerror()) throw std::runtime_error("Failed to load " + function + " from " + libname + " library!");
    // OK; return pointer
    return result;
  }

  // Attempt to load and return pointer to the emulator plugin library
  void* emulator_library() {
    static bool first_run = true;
    static void* handle = NULL;

    if (first_run) {
      // Attempt to load emulator library
      static const char *EMULATOR_LIB_NAME = "libqristal_emulator.so";
      handle = dlopen(EMULATOR_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
      first_run = false;
    }
    return handle;
  }


  // Whether or not the emulator plugin has been compiled with GPU support
  bool emulator_supports_gpus() {
    static bool first_run = true;
    static bool supported = false;
    static void* emulator = emulator_library();

    if (emulator and first_run) {
      supported = load<bool(*)()>("emulator_built_with_cuda", emulator, "emulator")();
      first_run = false;
    }
    return supported;
  }


  // The number of GPUs available on the system for use with the emulator plugin
  size_t available_gpus() {
    static bool first_run = true;
    static size_t num = 0;
    static void* emulator = emulator_library();

    if (emulator and first_run) {
      num = load<size_t(*)()>("num_gpu_devices", emulator, "emulator")();
      first_run = false;
    }
    return num;
  }


  // Check if a proposed set of GPU device IDs is valid on the current system.
  void check_gpu_device_ids(const std::vector<size_t>& gpu_device_ids) {
    static bool first_run = true;
    static void* emulator = emulator_library();
    using func_type = void(*)(const std::vector<size_t>&);
    static func_type checker;

    if (emulator_supports_gpus()) {
      if (first_run) {
        checker = load<func_type>("check_gpu_device_ids", emulator, "emulator");
        first_run = false;
      }
      checker(gpu_device_ids);
    }
  }

}
