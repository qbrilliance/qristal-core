// Copyright (c) Quantum Brilliance Pty Ltd

#include "qb/core/python/py_circuit_builder.hpp"
#include "qb/core/python/py_job_handle.hpp"
#include "qb/core/python/py_middleware.hpp"
#include "qb/core/python/py_noise_model.hpp"
#include "qb/core/python/py_optimization.hpp"
#include "qb/core/python/py_session.hpp"
#include "qb/core/python/py_stl_containers.hpp"
#include "xacc.hpp"
#include <pybind11/iostream.h>

#ifdef WITH_CUDAQ
#include "qb/core/cudaq/sim_pool.hpp"
#endif

namespace py = pybind11;
/// Python binding for the Qristal's core module
PYBIND11_MODULE(core, m) {
  m.doc() = "pybind11 for QB SDK";
  // Initialize XACC framework during module import.
  xacc::Initialize();
  xacc::setIsPyApi();
#ifdef WITH_CUDAQ
  // Initialize CUDAQ runtime
  qb::cudaq_sim_pool::get_instance().init_cudaq_runtime();
#endif
  // Properly redirect C++ std::cout -> Python sys.stdout
  py::add_ostream_redirect(m);
  // Basic containers
  qb::bind_opaque_containers(m);
  // Async. job handle
  qb::bind_job_handle(m);
  // Noise modelling types
  qb::bind_noise_model(m);
  // Placement passes
  qb::bind_placement_passes(m);
  // Circuit optimization passes
  qb::bind_circuit_opt_passes(m);
  // Circuit builder
  qb::bind_circuit_builder(m);
  // Session class
  qb::bind_session(m);
  // Optimization modules: nested under core.optimization sub-module
  py::module_ m_opt =
      m.def_submodule("optimization", "Optimization modules within qb_core");
  // VQEE
  qb::bind_vqee(m_opt);
  // Simple QAOA
  qb::bind_qaoa_simple(m_opt);
  // Recursive QAOA
  qb::bind_qaoa_recursive(m_opt);
  // Warm-start QAOA
  qb::bind_qaoa_warm_start(m_opt);
}
