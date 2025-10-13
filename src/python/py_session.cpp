// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/python/py_job_handle.hpp>
#include <qristal/core/python/py_stl_containers.hpp>
#include <qristal/core/python/py_session.hpp>
#include <qristal/core/python/py_help_strings.hpp>
#include <qristal/core/session.hpp>
#include <qristal/core/jensen_shannon.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/thread_pool.hpp>
#include <pybind11/eigen.h>

namespace qristal {

  void bind_session(pybind11::module &m) {

    namespace py = pybind11;

    m.def("jensen_shannon", &jensen_shannon, "Calculate Jensen-Shannon divergence");

    py::class_<session> py_session(m, "session");

    py::enum_<circuit_language>(m, "circuit_language")
      .value("XASM", circuit_language::XASM)
      .value("Quil", circuit_language::Quil)
      .value("OpenQASM", circuit_language::OpenQASM)
      .export_values();

    py_session.def(py::init<const bool>())
              .def(py::init())
              .def_readwrite("infile", &session::infile)
              .def_readwrite("instring", &session::instring)
              .def_readwrite("include_qb", &session::include_qb)
              .def_readwrite("circuit_parameters", &session::circuit_parameters)
              .def_readwrite("calc_gradients", &session::calc_gradients)
              .def_readwrite("calc_all_bitstring_counts", &session::calc_all_bitstring_counts)
              .def_readwrite("remote_backend_database_path", &session::remote_backend_database_path)
              .def_readwrite("acc", &session::acc)
              .def_readwrite("aer_sim_type", &session::aer_sim_type)
              .def_readwrite("aer_omp_threads", &session::aer_omp_threads)
              .def_readwrite("random_circuit_depth", &session::random_circuit_depth)
              .def_readwrite("noplacement", &session::noplacement)
              .def_readwrite("placement", &session::placement)
              .def_readwrite("nooptimise", &session::nooptimise)
              .def_readwrite("circuit_opts", &session::circuit_opts)
              .def_readwrite("execute_circuit", &session::execute_circuit)
              .def_readwrite("noise", &session::noise)
              .def_readwrite("calc_state_vec", &session::calc_state_vec)
              .def_readwrite("output_oqm_enabled", &session::output_oqm_enabled)
              .def_readwrite("notiming", &session::notiming)
              .def_readwrite("qn", &session::qn)
              .def_readwrite("sn", &session::sn)
              .def_readwrite("initial_bond_dimension", &session::initial_bond_dimension)
              .def_readwrite("max_bond_dimension", &session::max_bond_dimension)
              .def_readwrite("initial_kraus_dimension", &session::initial_kraus_dimension)
              .def_readwrite("max_kraus_dimension", &session::max_kraus_dimension)
              .def_readwrite("svd_cutoff", &session::svd_cutoff)
              .def_readwrite("rel_svd_cutoff", &session::rel_svd_cutoff)
              .def_readwrite("measure_sample_method", &session::measure_sample_method)
              .def_readwrite("svd_type", &session::svd_type)
              .def_readwrite("svdj_tol", &session::svdj_tol)
              .def_readwrite("svdj_max_sweeps", &session::svdj_max_sweeps)
              .def_readwrite("debug", &session::debug)
              .def_readwrite("seed", &session::seed)
              .def_readwrite("noise_mitigation", &session::noise_mitigation)
              .def_readwrite("input_language", &session::input_language)
              .def_readwrite("SPAM_correction_matrix", &session::SPAM_correction_matrix)
              .def_property_readonly("results", &session::results, help::results)
              .def_property_readonly("results_native", &session::results_native, help::results_native)
              .def_property_readonly("state_vec", &session::state_vec, help::state_vec)
              .def_property_readonly("all_bitstring_probabilities", &session::all_bitstring_probabilities, help::all_bitstring_probabilities)
              .def_property_readonly("all_bitstring_counts", &session::all_bitstring_counts, help::all_bitstring_counts)
              .def_property_readonly("all_bitstring_probability_gradients", &session::all_bitstring_probability_gradients, help::all_bitstring_probability_gradients)
              .def_property_readonly("transpiled_circuit", &session::transpiled_circuit, help::transpiled_circuit)
              .def_property_readonly("qobj", &session::qobj, help::qobj)
              .def_property_readonly("qbjson", &session::qbjson, help::qbjson)
              .def_property_readonly("one_qubit_gate_depths", &session::one_qubit_gate_depths, help::one_qubit_gate_depths)
              .def_property_readonly("two_qubit_gate_depths", &session::two_qubit_gate_depths, help::two_qubit_gate_depths)
              .def_property_readonly("timing_estimates", &session::timing_estimates, help::timing_estimates)
              .def_property_readonly("z_op_expectation", &session::z_op_expectation, help::z_op_expectation)

              .def_property("gpu_device_ids",
                    [&](qristal::session& s) { return py::array_t<size_t>(s.gpu_device_ids); },
                    [&](qristal::session& s, const py::array_t<size_t>& ids) { s.gpu_device_ids = py_array_to_std_vec(ids); },
                    qristal::help::gpu_device_ids)

              .def_property("SPAM_confusion_matrix", &session::get_SPAM_confusion_matrix, &session::set_SPAM_confusion_matrix)

              .def_property(
                  "irtarget",
                  [&](qristal::session& s) { return qristal::CircuitBuilder(s.irtarget); },
                  [&](qristal::session& s, qristal::CircuitBuilder &circuit) { s.irtarget = circuit.get(); },
                  "Circuit object to be executed.")

              .def_property(
                  "noise_model",
                  [&](qristal::session& s) { return *(s.noise_model); },
                  [&](qristal::session& s, qristal::NoiseModel& nm) { s.noise_model = std::make_shared<qristal::NoiseModel>(nm); },
                  "Circuit object to be executed.")

              .def_property(
                  "num_threads",
                  [&](session &s) { return thread_pool::get_num_threads(); },
                  [&](session &s, int i) { thread_pool::set_num_threads(i); },
                  "num_threads: The number of threads in the Qristal thread pool")

              .def("run_with_SPAM", py::overload_cast<size_t>(&session::run_with_SPAM),
                   "Automatically execute a SPAM measurement and enable automatic SPAM correction. Then automatically exexute run().")
              .def("bitstring_index", py::overload_cast<const std::vector<bool>&>(&session::bitstring_index), help::bitstring_index)
              .def("bitstring_index",
                   [&](session &s, const py::array_t<bool>& key) {
                     return s.bitstring_index(py_array_to_std_vec(key));
                   },
                   help::bitstring_index)
              .def("draw_shot", py::overload_cast<>(&session::draw_shot),
                   "draw_shot : Draw a single shot from the saved results of circuit i, condition j.")
              .def("run",
                  [&](session &s) {
                    std::shared_ptr<async_job_handle> handle = s.run();
                    if (handle) handle->wait_for_completion();
                  },
                  "Execute quantum circuit synchronously (blocking).")
              .def("run_async",
                  [&](session &s) {
                    auto handle = std::make_shared<JobHandle>();
                    // Allow accelerators to acquire the GIL for themselves from a
                    // different thread
                    handle->post_async(s);
                    return handle;
                  },
                  "Execute quantum circuit asynchronously (non-blocking).");

  }
}
