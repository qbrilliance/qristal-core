// Copyright (c) Quantum Brilliance Pty Ltd

#include "qb/core/python/py_job_handle.hpp"
#include "qb/core/python/py_stl_containers.hpp"
#include "qb/core/python/py_session.hpp"
#include "qb/core/python/py_help_strings.hpp"
#include "qb/core/session.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/thread_pool.hpp"

namespace qb {
void bind_session(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qb::session>(m, "session")
      .def(py::init<const std::string &>())
      .def(py::init<const bool>())
      .def(py::init<const bool, const bool>())
      .def(py::init())
      .def_property(
          "name_p", &qb::session::getName,
          py::overload_cast<const std::string &>(&qb::session::setName))
      .def_property(
          "names_p", &qb::session::getName,
          py::overload_cast<const Table2d<std::string> &>(&qb::session::setName))
      .def_property("infile", &qb::session::get_infiles,
                    &qb::session::set_infile, qb::help::infiles_)
      .def_property("infiles", &qb::session::get_infiles,
                    &qb::session::set_infiles, qb::help::infiles_)
      .def_property("instring", &qb::session::get_instrings,
                    &qb::session::set_instring, qb::help::instrings_)
      .def_property("instrings", &qb::session::get_instrings,
                    &qb::session::set_instrings, qb::help::instrings_)
      .def_property(
          "ir_target",
          [&](qb::session &s) {
            std::vector<std::vector<qb::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qb::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qb::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qb::session &s, qb::CircuitBuilder &circuit) {
            s.set_irtarget_m(circuit.get());
          },
          qb::help::irtarget_ms_)
      .def_property(
          "ir_targets",
          [&](qb::session &s) {
            std::vector<std::vector<qb::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qb::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qb::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qb::session &s,
              std::vector<std::vector<qb::CircuitBuilder>> &circuits) {
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                circuits_get;
            for (std::vector<qb::CircuitBuilder> vec : circuits) {
              std::vector<std::shared_ptr<xacc::CompositeInstruction>> vec_get;
              for (qb::CircuitBuilder circuit : vec) {
                std::shared_ptr<xacc::CompositeInstruction> circuit_get =
                    circuit.get();
                vec_get.push_back(circuit_get);
              }
              circuits_get.push_back(vec_get);
            }
            s.set_irtarget_ms(circuits_get);
          },
          qb::help::irtarget_ms_)
      .def_property("include_qb", &qb::session::get_include_qbs,
                    &qb::session::set_include_qb,
                    qb::help::include_qbs_)
      .def_property("include_qbs", &qb::session::get_include_qbs,
                    &qb::session::set_include_qbs,
                    qb::help::include_qbs_)
      .def_property("remote_backend_database_path", &qb::session::get_remote_backend_database_path,
                    &qb::session::set_remote_backend_database_path,
                    qb::help::remote_backend_database_path_)
      .def_property("acc", &qb::session::get_accs, &qb::session::set_acc,
                    qb::help::accs_)
      .def_property("accs", &qb::session::get_accs, &qb::session::set_accs,
                    qb::help::accs_)
      .def_property("aer_sim_type", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_type,
                    qb::help::aer_sim_types_)
      .def_property("aer_sim_types", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_types,
                    qb::help::aer_sim_types_)
      .def_property("random", &qb::session::get_randoms,
                    &qb::session::set_random, qb::help::randoms_)
      .def_property("randoms", &qb::session::get_randoms,
                    &qb::session::set_randoms, qb::help::randoms_)
      .def_property("xasm", &qb::session::get_xasms, &qb::session::set_xasm,
                    qb::help::xasms_)
      .def_property("xasms", &qb::session::get_xasms, &qb::session::set_xasms,
                    qb::help::xasms_)
      .def_property("quil1", &qb::session::get_quil1s, &qb::session::set_quil1,
                    qb::help::quil1s_)
      .def_property("quil1s", &qb::session::get_quil1s,
                    &qb::session::set_quil1s, qb::help::quil1s_)
      .def_property("calc_jacobian", &qb::session::get_calc_jacobians,
                    &qb::session::set_calc_jacobian,
                    qb::help::calc_jacobians_)
      .def_property("calc_jacobians", &qb::session::get_calc_jacobians,
                    &qb::session::set_calc_jacobians,
                    qb::help::calc_jacobians_)
      .def_property("calc_out_counts", &qb::session::get_calc_out_counts,
                    &qb::session::set_calc_out_counts,
                    qb::help::calc_out_counts_)
      .def_property("calc_out_countss", &qb::session::get_calc_out_counts,
                    &qb::session::set_calc_out_countss,
                    qb::help::calc_out_counts_)
      .def_property("noplacement", &qb::session::get_noplacements,
                    &qb::session::set_noplacement,
                    qb::help::noplacements_)
      .def_property("noplacements", &qb::session::get_noplacements,
                    &qb::session::set_noplacements,
                    qb::help::noplacements_)
      .def_property("placement", &qb::session::get_placements,
                    &qb::session::set_placement, qb::help::placements_)
      .def_property("placements", &qb::session::get_placements,
                    &qb::session::set_placements, qb::help::placements_)
      .def_property("nooptimise", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimise,
                    qb::help::nooptimises_)
      .def_property("nooptimises", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimises,
                    qb::help::nooptimises_)
      .def_property("circuit_optimization", &qb::session::get_circuit_opts,
                    &qb::session::set_circuit_opt,
                    qb::help::circuit_opts_)
      .def_property("circuit_optimizations", &qb::session::get_circuit_opts,
                    &qb::session::set_circuit_opts,
                    qb::help::circuit_opts_)
      .def_property("nosim", &qb::session::get_nosims, &qb::session::set_nosim,
                    qb::help::nosims_)
      .def_property("nosims", &qb::session::get_nosims,
                    &qb::session::set_nosims, qb::help::nosims_)
      .def_property("noise", &qb::session::get_noises, &qb::session::set_noise,
                    qb::help::noises_)
      .def_property("noises", &qb::session::get_noises,
                    &qb::session::set_noises, qb::help::noises_)
      .def_property("noise_model", &qb::session::get_noise_models,
                    &qb::session::set_noise_model,
                    qb::help::noise_models_)
      .def_property("noise_models", &qb::session::get_noise_models,
                    &qb::session::set_noise_models,
                    qb::help::noise_models_)
      .def_property("noise_mitigation", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigation,
                    qb::help::noise_mitigations_)
      .def_property("noise_mitigations", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigations,
                    qb::help::noise_mitigations_)
      .def_property("notiming", &qb::session::get_notimings,
                    &qb::session::set_notiming, qb::help::notimings_)
      .def_property("notimings", &qb::session::get_notimings,
                    &qb::session::set_notimings, qb::help::notimings_)
      .def_property("output_oqm_enabled", &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enabled,
                    qb::help::output_oqm_enableds_)
      .def_property("output_oqm_enableds",
                    &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enableds,
                    qb::help::output_oqm_enableds_)
      .def_property("log_enabled", &qb::session::get_log_enableds,
                    &qb::session::set_log_enabled,
                    qb::help::log_enableds_)
      .def_property("log_enableds", &qb::session::get_log_enableds,
                    &qb::session::set_log_enableds,
                    qb::help::log_enableds_)
      .def_property("qn", &qb::session::get_qns, &qb::session::set_qn,
                    qb::help::qns_)
      .def_property("qns", &qb::session::get_qns, &qb::session::set_qns,
                    qb::help::qns_)
      .def_property("rn", &qb::session::get_rns, &qb::session::set_rn,
                    qb::help::rns_)
      .def_property("rns", &qb::session::get_rns, &qb::session::set_rns,
                    qb::help::rns_)
      .def_property("sn", &qb::session::get_sns, &qb::session::set_sn,
                    qb::help::sns_)
      .def_property("sns", &qb::session::get_sns, &qb::session::set_sns,
                    qb::help::sns_)
      .def_property("beta", &qb::session::get_betas, &qb::session::set_beta,
                    qb::help::betas_)
      .def_property("betas", &qb::session::get_betas, &qb::session::set_betas,
                    qb::help::betas_)
      .def_property("theta", &qb::session::get_thetas, &qb::session::set_theta,
                    qb::help::thetas_)
      .def_property("thetas", &qb::session::get_thetas,
                    &qb::session::set_thetas, qb::help::thetas_)
      .def_property("parameter_list", &qb::session::get_parameter_vectors,
                    &qb::session::set_parameter_vector,
                    qb::help::parameter_vectors_)
      .def_property("parameter_lists", &qb::session::get_parameter_vectors,
                    &qb::session::set_parameter_vectors,
                    qb::help::parameter_vectors_)
      .def_property("svd_cutoff", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoff,
                    qb::help::svd_cutoffs_)
      .def_property("svd_cutoffs", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoffs,
                    qb::help::svd_cutoffs_)
      .def_property("rel_svd_cutoff", &qb::session::get_rel_svd_cutoffs,
                    &qb::session::set_rel_svd_cutoff,
                    qb::help::rel_svd_cutoffs_)
      .def_property("rel_svd_cutoffs", &qb::session::get_rel_svd_cutoffs,
                    &qb::session::set_rel_svd_cutoffs,
                    qb::help::rel_svd_cutoffs_)
      .def_property("initial_bond_dimension", &qb::session::get_initial_bond_dimensions,
                    &qb::session::set_initial_bond_dimension,
                    qb::help::initial_bond_dimensions_)
      .def_property("initial_bond_dimensions",
                    &qb::session::get_initial_bond_dimensions,
                    &qb::session::set_initial_bond_dimensions,
                    qb::help::initial_bond_dimensions_)
      .def_property("initial_kraus_dimension", &qb::session::get_initial_kraus_dimensions,
                    &qb::session::set_initial_kraus_dimension,
                    qb::help::initial_kraus_dimensions_)
      .def_property("initial_kraus_dimensions",
                    &qb::session::get_initial_kraus_dimensions,
                    &qb::session::set_initial_kraus_dimensions,
                    qb::help::initial_kraus_dimensions_)
      .def_property("max_bond_dimension", &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimension,
                    qb::help::max_bond_dimensions_)
      .def_property("max_bond_dimensions",
                    &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimensions,
                    qb::help::max_bond_dimensions_)
      .def_property("max_kraus_dimension", &qb::session::get_max_kraus_dimensions,
                    &qb::session::set_max_kraus_dimension,
                    qb::help::max_kraus_dimensions_)
      .def_property("max_kraus_dimensions",
                    &qb::session::get_max_kraus_dimensions,
                    &qb::session::set_max_kraus_dimensions,
                    qb::help::max_kraus_dimensions_)
      .def_property("measure_sample_sequential", &qb::session::get_measure_sample_sequentials,
                    &qb::session::set_measure_sample_sequential,
                    qb::help::measure_sample_sequentials_)
      .def_property("measure_sample_sequentials",
                    &qb::session::get_measure_sample_sequentials,
                    &qb::session::set_measure_sample_sequentials,
                    qb::help::measure_sample_sequentials_)
      .def_property("expected_amplitudes", &qb::session::get_expected_amplitudes,
                    &qb::session::set_expected_amplitudes,
                    qb::help::expected_amplitudes_)
      .def_property("expected_amplitudess", &qb::session::get_expected_amplitudes,
                    &qb::session::set_expected_amplitudess,
                    qb::help::expected_amplitudes_)
      .def_property("get_state_vec", &qb::session::get_state_vec,
                    &qb::session::get_state_vec,
                    qb::help::state_vec_)
      .def_property("get_state_vec_raw",
          [&](qb::session &s) {
            std::vector<std::complex<double>> stateVecData;
            std::shared_ptr<std::vector<std::complex<double>>> stateVec = s.get_state_vec_raw();
            for (const auto x : *stateVec) {
              stateVecData.emplace_back(x);
            }
            return stateVecData;
          },
          &qb::session::get_state_vec,
          qb::help::state_vec_)
      .def_property_readonly("results", &qb::session::results,
                             qb::help::results_)
      .def_property_readonly("out_probs", &qb::session::get_out_probs,
                             qb::help::out_probs_)
      .def_property_readonly("out_counts", &qb::session::get_out_counts,
                             qb::help::out_counts_)
      .def_property_readonly("out_prob_jacobians", &qb::session::get_out_prob_jacobians,
                             qb::help::out_jacobians_)
      .def_property_readonly("out_divergence",
                             &qb::session::get_out_divergences,
                             qb::help::out_divergences_)
      .def_property_readonly("out_divergences",
                             &qb::session::get_out_divergences,
                             qb::help::out_divergences_)
      .def_property_readonly("out_transpiled_circuit",
                             &qb::session::get_out_transpiled_circuits,
                             qb::help::out_transpiled_circuits_)
      .def_property_readonly("out_transpiled_circuits",
                             &qb::session::get_out_transpiled_circuits,
                             qb::help::out_transpiled_circuits_)
      .def_property_readonly("out_qobj", &qb::session::get_out_qobjs,
                             qb::help::out_qobjs_)
      .def_property_readonly("out_qobjs", &qb::session::get_out_qobjs,
                             qb::help::out_qobjs_)
      .def_property_readonly("out_qbjson", &qb::session::get_out_qbjsons,
                             qb::help::out_qbjsons_)
      .def_property_readonly("out_qbjsons", &qb::session::get_out_qbjsons,
                             qb::help::out_qbjsons_)
      .def_property_readonly("out_single_qubit_gate_qty",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::help::out_single_qubit_gate_qtys_)
      .def_property_readonly("out_single_qubit_gate_qtys",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::help::out_single_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qty",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::help::out_double_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qtys",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::help::out_double_qubit_gate_qtys_)

      .def_property_readonly(
          "out_total_init_maxgate_readout_time",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::help::out_total_init_maxgate_readout_times_)
      .def_property_readonly(
          "out_total_init_maxgate_readout_times",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::help::out_total_init_maxgate_readout_times_)

      .def_property_readonly("out_z_op_expect",
                             &qb::session::get_out_z_op_expects,
                             qb::help::out_z_op_expects_)
      .def_property_readonly("out_z_op_expects",
                             &qb::session::get_out_z_op_expects,
                             qb::help::out_z_op_expects_)

      .def_property("debug", &qb::session::get_debug, &qb::session::set_debug,
                    qb::help::debug_)

      .def_property(
          "num_threads",
          [&](qb::session &s) { return qb::thread_pool::get_num_threads(); },
          [&](qb::session &s, int i) { qb::thread_pool::set_num_threads(i); },
          "num_threads: The number of threads in the QB SDK thread pool")

      .def_property("seed", &qb::session::get_seeds, &qb::session::set_seed,
                    qb::help::seeds_)
      .def_property("seeds", &qb::session::get_seeds, &qb::session::set_seeds,
                    qb::help::seeds_)

      .def("__repr__", &qb::session::get_summary,
           "Print summary of session settings")
      .def("run", py::overload_cast<>(&qb::session::run),
           "Execute all declared quantum circuits under all conditions")
      .def("runit",
           py::overload_cast<const size_t, const size_t>(&qb::session::run),
           "runit(i,j) : Execute circuit i, condition j")
      .def("bitstring_index",
           py::overload_cast<const std::vector<bool>&>(&qb::session::bitstring_index),
           qb::help::bitstring_index_)
      .def("bitstring_index",
           [&](qb::session &s, const py::array_t<bool>& key) {
             return s.bitstring_index(py_array_to_std_vec(key));
           },
           qb::help::bitstring_index_)
      .def("divergence", py::overload_cast<>(&qb::session::get_jensen_shannon),
           "Calculate Jensen-Shannon divergence")
      .def("init", py::overload_cast<>(&qb::session::init),
           "Quantum Brilliance 12-qubit defaults")
      .def("aws_setup", py::overload_cast<uint>(&qb::session::aws_setup),
           "AWS Braket Setup")
      .def("set_parallel_run_config", &qb::session::set_parallel_run_config,
           "Set the parallel execution configuration")
      .def(
          "run_async",
          [&](qb::session &s, int i, int j) {
            auto handle = std::make_shared<qb::JobHandle>();
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
          [&](qb::session &s, int i, int j) {
            auto handle = qb::JobHandle::getJobHandle(i, j);
            if (handle) {
              return handle->complete();
            } else {
              return true;
            }
          },
          "run_complete(i,j) : Check if the execution of circuit i, condition "
          "j has been completed.");
}
} // namespace qb
