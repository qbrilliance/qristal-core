// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/python/py_circuit_builder.hpp>
#include <qristal/core/python/py_job_handle.hpp>
#include <qristal/core/python/py_middleware.hpp>
#include <qristal/core/python/py_noise_model.hpp>
#include <qristal/core/python/py_optimization.hpp>
#include <qristal/core/python/py_session.hpp>
#include <qristal/core/python/py_stl_containers.hpp>
#include <qristal/core/python/py_benchmark.hpp>
#include <xacc.hpp>
#include <pybind11/iostream.h>

#ifdef WITH_CUDAQ
#include <qristal/core/cudaq/sim_pool.hpp>
#endif

namespace py = pybind11;
/// Python binding for the Qristal's core module
PYBIND11_MODULE(core, m) {
  m.doc() = "pybind11 for Qristal";
  // Initialize XACC framework during module import.
  xacc::Initialize();
  xacc::setIsPyApi();
#ifdef WITH_CUDAQ
  // Initialize CUDAQ runtime
  qristal::cudaq_sim_pool::get_instance().init_cudaq_runtime();
#endif
  // Properly redirect C++ std::cout -> Python sys.stdout
  py::add_ostream_redirect(m);
  // Basic containers
  qristal::bind_opaque_containers(m);
  // Async. job handle
  qristal::bind_job_handle(m);
  // Noise modelling types
  qristal::bind_noise_model(m);
  // Placement passes
  qristal::bind_placement_passes(m);
  // Circuit optimization passes
  qristal::bind_circuit_opt_passes(m);
  // Circuit builder
  qristal::bind_circuit_builder(m);
  // Session class
  qristal::bind_session(m);
  // Optimization modules: nested under core.optimization sub-module
  py::module_ m_opt = m.def_submodule("optimization", "Optimization modules within qristal.core");
  // VQEE
  qristal::bind_vqee(m_opt);

  // Benchmarking modules; nested under core.benchmark sub-module
  py::module_ m_benchmark = m.def_submodule("benchmark", "Benchmarking modules within qristal.core");
  //general
  qristal::bind_Task(m_benchmark); //Task
  qristal::bind_Pauli(m_benchmark); //Pauli
  qristal::bind_BlochSphereUnitState(m_benchmark); //BlochSphereUnitState
  //workflows
  qristal::bind_SPAMBenchmark(m_benchmark); //SPAMBenchmark
  qristal::bind_RotationSweep(m_benchmark); //RotationSweep
  qristal::bind_SimpleCircuitExecution(m_benchmark); //SimpleCircuitExecution
  qristal::bind_PyGSTiBenchmark(m_benchmark); //PyGSTiBenchmark
  qristal::bind_QuantumStateTomography(m_benchmark); //QuantumStateTomography
  qristal::bind_QuantumProcessTomography(m_benchmark); //QuantumProcessTomography
  //metrics
  qristal::bind_CircuitFidelity(m_benchmark); //CircuitFidelity
  qristal::bind_PyGSTiResults(m_benchmark); //PyGSTiResults
  qristal::bind_ConfusionMatrix(m_benchmark); //ConfusionMatrix
  qristal::bind_QuantumStateDensity(m_benchmark); //QuantumStateDensity
  qristal::bind_QuantumProcessMatrix(m_benchmark); //QuantumProcessMatrix
  qristal::bind_QuantumStateFidelity(m_benchmark); //QuantumStateFidelity
  qristal::bind_QuantumProcessFidelity(m_benchmark); //QuantumProcessFidelity
}
