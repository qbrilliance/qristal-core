// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_session.hpp"
#include "qb/core/session.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/thread_pool.hpp"
#include "py_job_handle.hpp"
#include "py_stl_containers.hpp"
namespace qb {
void bind_session(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qb::session>(m, "session")
      .def(py::init<const std::string &>())
      .def(py::init<const bool>())
      .def(py::init())
      .def_property(
          "name_p", &qb::session::getName,
          py::overload_cast<const std::string &>(&qb::session::setName))
      .def_property(
          "names_p", &qb::session::getName,
          py::overload_cast<const VectorString &>(&qb::session::setName))
      .def_property("infile", &qb::session::get_infiles,
                    &qb::session::set_infile, qb::session::help_infiles_)
      .def_property("infiles", &qb::session::get_infiles,
                    &qb::session::set_infiles, qb::session::help_infiles_)
      .def_property("instring", &qb::session::get_instrings,
                    &qb::session::set_instring, qb::session::help_instrings_)
      .def_property("instrings", &qb::session::get_instrings,
                    &qb::session::set_instrings, qb::session::help_instrings_)
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
          qb::session::help_irtarget_ms_)
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
          qb::session::help_irtarget_ms_)
      .def_property("include_qb", &qb::session::get_include_qbs,
                    &qb::session::set_include_qb,
                    qb::session::help_include_qbs_)
      .def_property("include_qbs", &qb::session::get_include_qbs,
                    &qb::session::set_include_qbs,
                    qb::session::help_include_qbs_)
      .def_property("qpu_config", &qb::session::get_qpu_configs,
                    &qb::session::set_qpu_config,
                    qb::session::help_qpu_configs_)
      .def_property("qpu_configs", &qb::session::get_qpu_configs,
                    &qb::session::set_qpu_configs,
                    qb::session::help_qpu_configs_)
      .def_property("acc", &qb::session::get_accs, &qb::session::set_acc,
                    qb::session::help_accs_)
      .def_property("accs", &qb::session::get_accs, &qb::session::set_accs,
                    qb::session::help_accs_)
      .def_property("aws_verbatim", &qb::session::get_aws_verbatims,
                    &qb::session::set_aws_verbatim,
                    qb::session::help_aws_verbatims_)
      .def_property("aws_verbatims", &qb::session::get_aws_verbatims,
                    &qb::session::set_aws_verbatims,
                    qb::session::help_aws_verbatims_)
      .def_property("aws_format", &qb::session::get_aws_formats,
                    &qb::session::set_aws_format,
                    qb::session::help_aws_formats_)
      .def_property("aws_formats", &qb::session::get_aws_formats,
                    &qb::session::set_aws_formats,
                    qb::session::help_aws_formats_)
      .def_property("aws_device", &qb::session::get_aws_device_names,
                    &qb::session::set_aws_device_name,
                    qb::session::help_aws_device_names_)
      .def_property("aws_devices", &qb::session::get_aws_device_names,
                    &qb::session::set_aws_device_names,
                    qb::session::help_aws_device_names_)
      .def_property("aws_s3", &qb::session::get_aws_s3s,
                    &qb::session::set_aws_s3, qb::session::help_aws_s3s_)
      .def_property("aws_s3s", &qb::session::get_aws_s3s,
                    &qb::session::set_aws_s3s, qb::session::help_aws_s3s_)
      .def_property("aws_s3_path", &qb::session::get_aws_s3_paths,
                    &qb::session::set_aws_s3_path,
                    qb::session::help_aws_s3_paths_)
      .def_property("aws_s3_paths", &qb::session::get_aws_s3_paths,
                    &qb::session::set_aws_s3_paths,
                    qb::session::help_aws_s3_paths_)
      .def_property("aer_sim_type", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_type,
                    qb::session::help_aer_sim_types_)
      .def_property("aer_sim_types", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_types,
                    qb::session::help_aer_sim_types_)
      .def_property("random", &qb::session::get_randoms,
                    &qb::session::set_random, qb::session::help_randoms_)
      .def_property("randoms", &qb::session::get_randoms,
                    &qb::session::set_randoms, qb::session::help_randoms_)
      .def_property("xasm", &qb::session::get_xasms, &qb::session::set_xasm,
                    qb::session::help_xasms_)
      .def_property("xasms", &qb::session::get_xasms, &qb::session::set_xasms,
                    qb::session::help_xasms_)
      .def_property("quil1", &qb::session::get_quil1s, &qb::session::set_quil1,
                    qb::session::help_quil1s_)
      .def_property("quil1s", &qb::session::get_quil1s,
                    &qb::session::set_quil1s, qb::session::help_quil1s_)
      .def_property("noplacement", &qb::session::get_noplacements,
                    &qb::session::set_noplacement,
                    qb::session::help_noplacements_)
      .def_property("noplacements", &qb::session::get_noplacements,
                    &qb::session::set_noplacements,
                    qb::session::help_noplacements_)
      .def_property("placement", &qb::session::get_placements,
                    &qb::session::set_placement, qb::session::help_placements_)
      .def_property("placements", &qb::session::get_placements,
                    &qb::session::set_placements, qb::session::help_placements_)
      .def_property("nooptimise", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimise,
                    qb::session::help_nooptimises_)
      .def_property("nooptimises", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimises,
                    qb::session::help_nooptimises_)
      .def_property("circuit_optimization", &qb::session::get_circuit_opts,
                    &qb::session::set_circuit_opt,
                    qb::session::help_circuit_opts_)
      .def_property("circuit_optimizations", &qb::session::get_circuit_opts,
                    &qb::session::set_circuit_opts,
                    qb::session::help_circuit_opts_)
      .def_property("nosim", &qb::session::get_nosims, &qb::session::set_nosim,
                    qb::session::help_nosims_)
      .def_property("nosims", &qb::session::get_nosims,
                    &qb::session::set_nosims, qb::session::help_nosims_)
      .def_property("noise", &qb::session::get_noises, &qb::session::set_noise,
                    qb::session::help_noises_)
      .def_property("noises", &qb::session::get_noises,
                    &qb::session::set_noises, qb::session::help_noises_)
      .def_property("noise_model", &qb::session::get_noise_models,
                    &qb::session::set_noise_model,
                    qb::session::help_noise_models_)
      .def_property("noise_models", &qb::session::get_noise_models,
                    &qb::session::set_noise_models,
                    qb::session::help_noise_models_)
      .def_property("noise_mitigation", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigation,
                    qb::session::help_noise_mitigations_)
      .def_property("noise_mitigations", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigations,
                    qb::session::help_noise_mitigations_)
      .def_property("notiming", &qb::session::get_notimings,
                    &qb::session::set_notiming, qb::session::help_notimings_)
      .def_property("notimings", &qb::session::get_notimings,
                    &qb::session::set_notimings, qb::session::help_notimings_)
      .def_property("output_oqm_enabled", &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enabled,
                    qb::session::help_output_oqm_enableds_)
      .def_property("output_oqm_enableds",
                    &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enableds,
                    qb::session::help_output_oqm_enableds_)
      .def_property("log_enabled", &qb::session::get_log_enableds,
                    &qb::session::set_log_enabled,
                    qb::session::help_log_enableds_)
      .def_property("log_enableds", &qb::session::get_log_enableds,
                    &qb::session::set_log_enableds,
                    qb::session::help_log_enableds_)
      .def_property("qn", &qb::session::get_qns, &qb::session::set_qn,
                    qb::session::help_qns_)
      .def_property("qns", &qb::session::get_qns, &qb::session::set_qns,
                    qb::session::help_qns_)
      .def_property("rn", &qb::session::get_rns, &qb::session::set_rn,
                    qb::session::help_rns_)
      .def_property("rns", &qb::session::get_rns, &qb::session::set_rns,
                    qb::session::help_rns_)
      .def_property("sn", &qb::session::get_sns, &qb::session::set_sn,
                    qb::session::help_sns_)
      .def_property("sns", &qb::session::get_sns, &qb::session::set_sns,
                    qb::session::help_sns_)
      .def_property("beta", &qb::session::get_betas, &qb::session::set_beta,
                    qb::session::help_betas_)
      .def_property("betas", &qb::session::get_betas, &qb::session::set_betas,
                    qb::session::help_betas_)
      .def_property("theta", &qb::session::get_thetas, &qb::session::set_theta,
                    qb::session::help_thetas_)
      .def_property("thetas", &qb::session::get_thetas,
                    &qb::session::set_thetas, qb::session::help_thetas_)
      .def_property("svd_cutoff", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoff,
                    qb::session::help_svd_cutoffs_)
      .def_property("svd_cutoffs", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoffs,
                    qb::session::help_svd_cutoffs_)
      .def_property("max_bond_dimension", &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimension,
                    qb::session::help_max_bond_dimensions_)
      .def_property("max_bond_dimensions",
                    &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimensions,
                    qb::session::help_max_bond_dimensions_)
      .def_property("output_amplitude", &qb::session::get_output_amplitudes,
                    &qb::session::set_output_amplitude,
                    qb::session::help_output_amplitudes_)
      .def_property("output_amplitudes", &qb::session::get_output_amplitudes,
                    &qb::session::set_output_amplitudes,
                    qb::session::help_output_amplitudes_)
      .def_property_readonly("out_raw", &qb::session::get_out_raws,
                             qb::session::help_out_raws_)
      .def_property_readonly("out_raws", &qb::session::get_out_raws,
                             qb::session::help_out_raws_)
      .def_property_readonly("out_count", &qb::session::get_out_counts,
                             qb::session::help_out_counts_)
      .def_property_readonly("out_counts", &qb::session::get_out_counts,
                             qb::session::help_out_counts_)
      .def_property_readonly("out_divergence",
                             &qb::session::get_out_divergences,
                             qb::session::help_out_divergences_)
      .def_property_readonly("out_divergences",
                             &qb::session::get_out_divergences,
                             qb::session::help_out_divergences_)
      .def_property_readonly("out_transpiled_circuit",
                             &qb::session::get_out_transpiled_circuits,
                             qb::session::help_out_transpiled_circuits_)
      .def_property_readonly("out_transpiled_circuits",
                             &qb::session::get_out_transpiled_circuits,
                             qb::session::help_out_transpiled_circuits_)
      .def_property_readonly("out_qobj", &qb::session::get_out_qobjs,
                             qb::session::help_out_qobjs_)
      .def_property_readonly("out_qobjs", &qb::session::get_out_qobjs,
                             qb::session::help_out_qobjs_)
      .def_property_readonly("out_qbjson", &qb::session::get_out_qbjsons,
                             qb::session::help_out_qbjsons_)
      .def_property_readonly("out_qbjsons", &qb::session::get_out_qbjsons,
                             qb::session::help_out_qbjsons_)
      .def_property_readonly("out_single_qubit_gate_qty",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::session::help_out_single_qubit_gate_qtys_)
      .def_property_readonly("out_single_qubit_gate_qtys",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::session::help_out_single_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qty",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::session::help_out_double_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qtys",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::session::help_out_double_qubit_gate_qtys_)

      .def_property_readonly(
          "out_total_init_maxgate_readout_time",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::session::help_out_total_init_maxgate_readout_times_)
      .def_property_readonly(
          "out_total_init_maxgate_readout_times",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::session::help_out_total_init_maxgate_readout_times_)

      .def_property_readonly("out_z_op_expect",
                             &qb::session::get_out_z_op_expects,
                             qb::session::help_out_z_op_expects_)
      .def_property_readonly("out_z_op_expects",
                             &qb::session::get_out_z_op_expects,
                             qb::session::help_out_z_op_expects_)

      .def_property("debug", &qb::session::get_debug, &qb::session::set_debug,
                    qb::session::help_debug_)

      .def_property(
          "num_threads",
          [&](qb::session &s) { return qb::thread_pool::get_num_threads(); },
          [&](qb::session &s, int i) { qb::thread_pool::set_num_threads(i); },
          "num_threads: The number of threads in the QB SDK thread pool")

      .def_property("seed", &qb::session::get_seeds, &qb::session::set_seed,
                    qb::session::help_seeds_)
      .def_property("seeds", &qb::session::get_seeds, &qb::session::set_seeds,
                    qb::session::help_seeds_)

      .def("__repr__", &qb::session::get_summary,
           "Print summary of session settings")
      .def("run", py::overload_cast<>(&qb::session::run),
           "Execute all declared quantum circuits under all conditions")
      .def("runit",
           py::overload_cast<const size_t, const size_t>(&qb::session::run),
           "runit(i,j) : Execute circuit i, condition j")
      .def("divergence", py::overload_cast<>(&qb::session::get_jensen_shannon),
           "Calculate Jensen-Shannon divergence")
      .def("qb12", py::overload_cast<>(&qb::session::qb12),
           "Quantum Brilliance 12-qubit defaults")
      .def("aws32dm1", py::overload_cast<>(&qb::session::aws32dm1),
           "AWS Braket DM1, 32 async workers")
      .def("aws32sv1", py::overload_cast<>(&qb::session::aws32sv1),
           "AWS Braket SV1, 32 async workers")
      .def("aws8tn1", py::overload_cast<>(&qb::session::aws8tn1),
           "AWS Braket TN1, 8 async workers")
      .def("set_contrasts",
           py::overload_cast<const double &, const double &, const double &>(
               &qb::session::set_contrasts),
           "QB hardware contrast thresholds: init, qubit[0] final readout, "
           "qubit[1] final readout")
      .def("reset_contrasts",
           py::overload_cast<>(&qb::session::reset_contrasts),
           "QB hardware contrast thresholds reset")
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