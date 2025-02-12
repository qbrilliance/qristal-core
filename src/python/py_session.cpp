// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/python/py_job_handle.hpp"
#include "qristal/core/python/py_stl_containers.hpp"
#include "qristal/core/python/py_session.hpp"
#include "qristal/core/python/py_help_strings.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/thread_pool.hpp"
#include <pybind11/eigen.h>

namespace qristal {
void bind_session(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qristal::session>(m, "session")
      .def(py::init<const std::string &>())
      .def(py::init<const bool>())
      .def(py::init<const bool, const bool>())
      .def(py::init())
      .def_property(
          "name_p", &qristal::session::getName,
          py::overload_cast<const std::string &>(&qristal::session::setName))
      .def_property(
          "names_p", &qristal::session::getName,
          py::overload_cast<const Table2d<std::string> &>(&qristal::session::setName))
      .def_property("infile", &qristal::session::get_infiles,
                    &qristal::session::set_infile, qristal::help::infiles_)
      .def_property("infiles", &qristal::session::get_infiles,
                    &qristal::session::set_infiles, qristal::help::infiles_)
      .def_property("instring", &qristal::session::get_instrings,
                    &qristal::session::set_instring, qristal::help::instrings_)
      .def_property("instrings", &qristal::session::get_instrings,
                    &qristal::session::set_instrings, qristal::help::instrings_)
      .def_property(
          "ir_target",
          [&](qristal::session &s) {
            std::vector<std::vector<qristal::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qristal::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qristal::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qristal::session &s, qristal::CircuitBuilder &circuit) {
            s.set_irtarget_m(circuit.get());
          },
          qristal::help::irtarget_ms_)
      .def_property(
          "ir_targets",
          [&](qristal::session &s) {
            std::vector<std::vector<qristal::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qristal::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qristal::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qristal::session &s,
              std::vector<std::vector<qristal::CircuitBuilder>> &circuits) {
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                circuits_get;
            for (std::vector<qristal::CircuitBuilder> vec : circuits) {
              std::vector<std::shared_ptr<xacc::CompositeInstruction>> vec_get;
              for (qristal::CircuitBuilder circuit : vec) {
                std::shared_ptr<xacc::CompositeInstruction> circuit_get =
                    circuit.get();
                vec_get.push_back(circuit_get);
              }
              circuits_get.push_back(vec_get);
            }
            s.set_irtarget_ms(circuits_get);
          },
          qristal::help::irtarget_ms_)
      .def_property("include_qb", &qristal::session::get_include_qbs,
                    &qristal::session::set_include_qb,
                    qristal::help::include_qbs_)
      .def_property("include_qbs", &qristal::session::get_include_qbs,
                    &qristal::session::set_include_qbs,
                    qristal::help::include_qbs_)
      .def_property("remote_backend_database_path", &qristal::session::get_remote_backend_database_path,
                    &qristal::session::set_remote_backend_database_path,
                    qristal::help::remote_backend_database_path_)
      .def_property("acc", &qristal::session::get_accs, &qristal::session::set_acc,
                    qristal::help::accs_)
      .def_property("accs", &qristal::session::get_accs, &qristal::session::set_accs,
                    qristal::help::accs_)
      .def_property("aer_sim_type", &qristal::session::get_aer_sim_types,
                    &qristal::session::set_aer_sim_type,
                    qristal::help::aer_sim_types_)
      .def_property("aer_sim_types", &qristal::session::get_aer_sim_types,
                    &qristal::session::set_aer_sim_types,
                    qristal::help::aer_sim_types_)
      .def_property("random", &qristal::session::get_randoms,
                    &qristal::session::set_random, qristal::help::randoms_)
      .def_property("randoms", &qristal::session::get_randoms,
                    &qristal::session::set_randoms, qristal::help::randoms_)
      .def_property("xasm", &qristal::session::get_xasms, &qristal::session::set_xasm,
                    qristal::help::xasms_)
      .def_property("xasms", &qristal::session::get_xasms, &qristal::session::set_xasms,
                    qristal::help::xasms_)
      .def_property("quil1", &qristal::session::get_quil1s, &qristal::session::set_quil1,
                    qristal::help::quil1s_)
      .def_property("quil1s", &qristal::session::get_quil1s,
                    &qristal::session::set_quil1s, qristal::help::quil1s_)
      .def_property("calc_jacobian", &qristal::session::get_calc_jacobians,
                    &qristal::session::set_calc_jacobian,
                    qristal::help::calc_jacobians_)
      .def_property("calc_jacobians", &qristal::session::get_calc_jacobians,
                    &qristal::session::set_calc_jacobians,
                    qristal::help::calc_jacobians_)
      .def_property("calc_out_counts", &qristal::session::get_calc_out_counts,
                    &qristal::session::set_calc_out_counts,
                    qristal::help::calc_out_counts_)
      .def_property("calc_out_countss", &qristal::session::get_calc_out_counts,
                    &qristal::session::set_calc_out_countss,
                    qristal::help::calc_out_counts_)
      .def_property("noplacement", &qristal::session::get_noplacements,
                    &qristal::session::set_noplacement,
                    qristal::help::noplacements_)
      .def_property("noplacements", &qristal::session::get_noplacements,
                    &qristal::session::set_noplacements,
                    qristal::help::noplacements_)
      .def_property("placement", &qristal::session::get_placements,
                    &qristal::session::set_placement, qristal::help::placements_)
      .def_property("placements", &qristal::session::get_placements,
                    &qristal::session::set_placements, qristal::help::placements_)
      .def_property("nooptimise", &qristal::session::get_nooptimises,
                    &qristal::session::set_nooptimise,
                    qristal::help::nooptimises_)
      .def_property("nooptimises", &qristal::session::get_nooptimises,
                    &qristal::session::set_nooptimises,
                    qristal::help::nooptimises_)
      .def_property("circuit_optimization", &qristal::session::get_circuit_opts,
                    &qristal::session::set_circuit_opt,
                    qristal::help::circuit_opts_)
      .def_property("circuit_optimizations", &qristal::session::get_circuit_opts,
                    &qristal::session::set_circuit_opts,
                    qristal::help::circuit_opts_)
      .def_property("execute_circuit", &qristal::session::get_execute_circuits, &qristal::session::set_execute_circuit,
                    qristal::help::execute_circuits_)
      .def_property("execute_circuits", &qristal::session::get_execute_circuits,
                    &qristal::session::set_execute_circuits, qristal::help::execute_circuits_)
      .def_property("noise", &qristal::session::get_noises, &qristal::session::set_noise,
                    qristal::help::noises_)
      .def_property("noises", &qristal::session::get_noises,
                    &qristal::session::set_noises, qristal::help::noises_)
      .def_property("noise_model", &qristal::session::get_noise_models,
                    &qristal::session::set_noise_model,
                    qristal::help::noise_models_)
      .def_property("noise_models", &qristal::session::get_noise_models,
                    &qristal::session::set_noise_models,
                    qristal::help::noise_models_)
      .def_property("noise_mitigation", &qristal::session::get_noise_mitigations,
                    &qristal::session::set_noise_mitigation,
                    qristal::help::noise_mitigations_)
      .def_property("noise_mitigations", &qristal::session::get_noise_mitigations,
                    &qristal::session::set_noise_mitigations,
                    qristal::help::noise_mitigations_)
      .def_property("SPAM_confusion", &qristal::session::get_SPAM_correction_matrix,
                    &qristal::session::set_SPAM_confusion_matrix,
                    qristal::help::SPAM_confusion_)
      .def_property("notiming", &qristal::session::get_notimings,
                    &qristal::session::set_notiming, qristal::help::notimings_)
      .def_property("notimings", &qristal::session::get_notimings,
                    &qristal::session::set_notimings, qristal::help::notimings_)
      .def_property("output_oqm_enabled", &qristal::session::get_output_oqm_enableds,
                    &qristal::session::set_output_oqm_enabled,
                    qristal::help::output_oqm_enableds_)
      .def_property("output_oqm_enableds",
                    &qristal::session::get_output_oqm_enableds,
                    &qristal::session::set_output_oqm_enableds,
                    qristal::help::output_oqm_enableds_)
      .def_property("qn", &qristal::session::get_qns, &qristal::session::set_qn,
                    qristal::help::qns_)
      .def_property("qns", &qristal::session::get_qns, &qristal::session::set_qns,
                    qristal::help::qns_)
      .def_property("sn", &qristal::session::get_sns, &qristal::session::set_sn,
                    qristal::help::sns_)
      .def_property("sns", &qristal::session::get_sns, &qristal::session::set_sns,
                    qristal::help::sns_)
      .def_property("parameter_list", &qristal::session::get_parameter_vectors,
                    &qristal::session::set_parameter_vector,
                    qristal::help::parameter_vectors_)
      .def_property("parameter_lists", &qristal::session::get_parameter_vectors,
                    &qristal::session::set_parameter_vectors,
                    qristal::help::parameter_vectors_)
      .def_property("svd_cutoff", &qristal::session::get_svd_cutoffs,
                    &qristal::session::set_svd_cutoff,
                    qristal::help::svd_cutoffs_)
      .def_property("svd_cutoffs", &qristal::session::get_svd_cutoffs,
                    &qristal::session::set_svd_cutoffs,
                    qristal::help::svd_cutoffs_)
      .def_property("rel_svd_cutoff", &qristal::session::get_rel_svd_cutoffs,
                    &qristal::session::set_rel_svd_cutoff,
                    qristal::help::rel_svd_cutoffs_)
      .def_property("rel_svd_cutoffs", &qristal::session::get_rel_svd_cutoffs,
                    &qristal::session::set_rel_svd_cutoffs,
                    qristal::help::rel_svd_cutoffs_)
      .def_property("initial_bond_dimension", &qristal::session::get_initial_bond_dimensions,
                    &qristal::session::set_initial_bond_dimension,
                    qristal::help::initial_bond_dimensions_)
      .def_property("initial_bond_dimensions",
                    &qristal::session::get_initial_bond_dimensions,
                    &qristal::session::set_initial_bond_dimensions,
                    qristal::help::initial_bond_dimensions_)
      .def_property("initial_kraus_dimension", &qristal::session::get_initial_kraus_dimensions,
                    &qristal::session::set_initial_kraus_dimension,
                    qristal::help::initial_kraus_dimensions_)
      .def_property("initial_kraus_dimensions",
                    &qristal::session::get_initial_kraus_dimensions,
                    &qristal::session::set_initial_kraus_dimensions,
                    qristal::help::initial_kraus_dimensions_)
      .def_property("max_bond_dimension", &qristal::session::get_max_bond_dimensions,
                    &qristal::session::set_max_bond_dimension,
                    qristal::help::max_bond_dimensions_)
      .def_property("max_bond_dimensions",
                    &qristal::session::get_max_bond_dimensions,
                    &qristal::session::set_max_bond_dimensions,
                    qristal::help::max_bond_dimensions_)
      .def_property("max_kraus_dimension", &qristal::session::get_max_kraus_dimensions,
                    &qristal::session::set_max_kraus_dimension,
                    qristal::help::max_kraus_dimensions_)
      .def_property("max_kraus_dimensions",
                    &qristal::session::get_max_kraus_dimensions,
                    &qristal::session::set_max_kraus_dimensions,
                    qristal::help::max_kraus_dimensions_)
      .def_property("measure_sample_method", &qristal::session::get_measure_sample_methods,
                    &qristal::session::set_measure_sample_method,
                    qristal::help::measure_sample_methods_)
      .def_property("measure_sample_methods",
                    &qristal::session::get_measure_sample_methods,
                    &qristal::session::set_measure_sample_methods,
                    qristal::help::measure_sample_methods_)
      .def_property("expected_amplitudes", &qristal::session::get_expected_amplitudes,
                    &qristal::session::set_expected_amplitudes,
                    qristal::help::expected_amplitudes_)
      .def_property("expected_amplitudess", &qristal::session::get_expected_amplitudes,
                    &qristal::session::set_expected_amplitudess,
                    qristal::help::expected_amplitudes_)
      .def_property("get_state_vec", &qristal::session::get_state_vec,
                    &qristal::session::get_state_vec,
                    qristal::help::state_vec_)
      .def_property("get_state_vec_raw",
          [&](qristal::session &s) {
            std::vector<std::complex<double>> stateVecData;
            std::shared_ptr<std::vector<std::complex<double>>> stateVec = s.get_state_vec_raw();
            for (const auto x : *stateVec) {
              stateVecData.emplace_back(x);
            }
            return stateVecData;
          },
          &qristal::session::get_state_vec,
          qristal::help::state_vec_)
      .def_property_readonly("results", &qristal::session::results,
                             qristal::help::results_)
      .def_property_readonly("results_native", &qristal::session::results_native,
                             qristal::help::results_native_)
      .def_property_readonly("out_probs", &qristal::session::get_out_probs,
                             qristal::help::out_probs_)
      .def_property_readonly("out_counts", &qristal::session::get_out_counts,
                             qristal::help::out_counts_)
      .def_property_readonly("out_prob_jacobians", &qristal::session::get_out_prob_jacobians,
                             qristal::help::out_jacobians_)
      .def_property_readonly("out_divergence",
                             &qristal::session::get_out_divergences,
                             qristal::help::out_divergences_)
      .def_property_readonly("out_divergences",
                             &qristal::session::get_out_divergences,
                             qristal::help::out_divergences_)
      .def_property_readonly("out_transpiled_circuit",
                             &qristal::session::get_out_transpiled_circuits,
                             qristal::help::out_transpiled_circuits_)
      .def_property_readonly("out_transpiled_circuits",
                             &qristal::session::get_out_transpiled_circuits,
                             qristal::help::out_transpiled_circuits_)
      .def_property_readonly("out_qobj", &qristal::session::get_out_qobjs,
                             qristal::help::out_qobjs_)
      .def_property_readonly("out_qobjs", &qristal::session::get_out_qobjs,
                             qristal::help::out_qobjs_)
      .def_property_readonly("out_qbjson", &qristal::session::get_out_qbjsons,
                             qristal::help::out_qbjsons_)
      .def_property_readonly("out_qbjsons", &qristal::session::get_out_qbjsons,
                             qristal::help::out_qbjsons_)
      .def_property_readonly("out_single_qubit_gate_qty",
                             &qristal::session::get_out_single_qubit_gate_qtys,
                             qristal::help::out_single_qubit_gate_qtys_)
      .def_property_readonly("out_single_qubit_gate_qtys",
                             &qristal::session::get_out_single_qubit_gate_qtys,
                             qristal::help::out_single_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qty",
                             &qristal::session::get_out_double_qubit_gate_qtys,
                             qristal::help::out_double_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qtys",
                             &qristal::session::get_out_double_qubit_gate_qtys,
                             qristal::help::out_double_qubit_gate_qtys_)

      .def_property_readonly(
          "out_total_init_maxgate_readout_time",
          &qristal::session::get_out_total_init_maxgate_readout_times,
          qristal::help::out_total_init_maxgate_readout_times_)
      .def_property_readonly(
          "out_total_init_maxgate_readout_times",
          &qristal::session::get_out_total_init_maxgate_readout_times,
          qristal::help::out_total_init_maxgate_readout_times_)

      .def_property_readonly("out_z_op_expect",
                             &qristal::session::get_out_z_op_expects,
                             qristal::help::out_z_op_expects_)
      .def_property_readonly("out_z_op_expects",
                             &qristal::session::get_out_z_op_expects,
                             qristal::help::out_z_op_expects_)

      .def_property("debug", &qristal::session::get_debug, &qristal::session::set_debug,
                    qristal::help::debug_)

      .def_property(
          "num_threads",
          [&](qristal::session &s) { return qristal::thread_pool::get_num_threads(); },
          [&](qristal::session &s, int i) { qristal::thread_pool::set_num_threads(i); },
          "num_threads: The number of threads in the Qristal thread pool")

      .def_property("seed", &qristal::session::get_seeds, &qristal::session::set_seed,
                    qristal::help::seeds_)
      .def_property("seeds", &qristal::session::get_seeds, &qristal::session::set_seeds,
                    qristal::help::seeds_)

      .def("__repr__", &qristal::session::get_summary,
           "Print summary of session settings")
      .def("run", py::overload_cast<>(&qristal::session::run),
           "Execute all declared quantum circuits under all conditions")
      .def("run_with_SPAM", py::overload_cast<size_t>(&qristal::session::run_with_SPAM),
           "Automatically execute a SPAM measurement and enable automatic SPAM correction. Then automatically exexute run().")
      .def("runit",
           py::overload_cast<const size_t, const size_t>(&qristal::session::run),
           "runit(i,j) : Execute circuit i, condition j")
      .def("bitstring_index",
           py::overload_cast<const std::vector<bool>&>(&qristal::session::bitstring_index),
           qristal::help::bitstring_index_)
      .def("bitstring_index",
           [&](qristal::session &s, const py::array_t<bool>& key) {
             return s.bitstring_index(py_array_to_std_vec(key));
           },
           qristal::help::bitstring_index_)
      .def("draw_shot", py::overload_cast<const size_t, const size_t>(&qristal::session::draw_shot),
           "draw_shot(i,j) : Draw a single shot from the saved results of circuit i, condition j.")
      .def("divergence", py::overload_cast<>(&qristal::session::get_jensen_shannon),
           "Calculate Jensen-Shannon divergence")
      .def("init", py::overload_cast<>(&qristal::session::init),
           "Quantum Brilliance 12-qubit defaults")
      .def("aws_setup", py::overload_cast<uint>(&qristal::session::aws_setup),
           "AWS Braket Setup")
      .def("set_parallel_run_config", &qristal::session::set_parallel_run_config,
           "Set the parallel execution configuration")
      .def(
          "run_async",
          [&](qristal::session &s, int i, int j) {
            auto handle = std::make_shared<qristal::JobHandle>();
            // Allow accelerators to acquire the GIL for themselves from a
            // different thread
            pybind11::gil_scoped_release release;
            handle->post_async(s, i, j);
            return handle;
          },
          "run_async(i,j) : Launch the execution of circuit i, condition j "
          "asynchronously.")
      .def(
          "run_complete",
          [&](qristal::session &s, int i, int j) {
            auto handle = qristal::JobHandle::getJobHandle(i, j);
            if (handle) {
              return handle->complete();
            } else {
              return true;
            }
          },
          "run_complete(i,j) : Check if the execution of circuit i, condition "
          "j has been completed.");
}
}
