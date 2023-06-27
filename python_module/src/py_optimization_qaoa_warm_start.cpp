// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_optimization.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/optimization/qaoa/qaoa.hpp"

namespace qb {
void bind_qaoa_warm_start(pybind11::module &opt_m) {
  namespace py = pybind11;
  py::class_<qb::op::QaoaWarmStart>(opt_m, "qaoa_QaoaWarmStart")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qb::op::QaoaWarmStart::get_colnames,
                    &qb::op::QaoaWarmStart::set_colname,
                    qb::op::QaoaWarmStart::help_colnames_)
      .def_property("colnames", &qb::op::QaoaWarmStart::get_colnames,
                    &qb::op::QaoaWarmStart::set_colnames,
                    qb::op::QaoaWarmStart::help_colnames_)
      .def_property("rowname", &qb::op::QaoaWarmStart::get_rownames,
                    &qb::op::QaoaWarmStart::set_rowname,
                    qb::op::QaoaWarmStart::help_rownames_)
      .def_property("rownames", &qb::op::QaoaWarmStart::get_rownames,
                    &qb::op::QaoaWarmStart::set_rownames,
                    qb::op::QaoaWarmStart::help_rownames_)
      .def_property("theta", &qb::op::QaoaWarmStart::get_thetas,
                    &qb::op::QaoaWarmStart::set_theta,
                    qb::op::QaoaWarmStart::help_thetas_)
      .def_property("thetas", &qb::op::QaoaWarmStart::get_thetas,
                    &qb::op::QaoaWarmStart::set_thetas,
                    qb::op::QaoaWarmStart::help_thetas_)
      .def_property("acc", &qb::op::QaoaWarmStart::get_accs,
                    &qb::op::QaoaWarmStart::set_acc,
                    qb::op::QaoaWarmStart::help_accs_)
      .def_property("accs", &qb::op::QaoaWarmStart::get_accs,
                    &qb::op::QaoaWarmStart::set_accs,
                    qb::op::QaoaWarmStart::help_accs_)
      .def_property("good_cut", &qb::op::QaoaWarmStart::get_good_cuts,
                    &qb::op::QaoaWarmStart::set_good_cut,
                    qb::op::QaoaWarmStart::help_good_cuts_)
      .def_property("good_cuts", &qb::op::QaoaWarmStart::get_good_cuts,
                    &qb::op::QaoaWarmStart::set_good_cuts,
                    qb::op::QaoaWarmStart::help_good_cuts_)
      .def_property("ham", &qb::op::QaoaWarmStart::get_hams,
                    &qb::op::QaoaWarmStart::set_ham,
                    qb::op::QaoaWarmStart::help_hams_)
      .def_property("hams", &qb::op::QaoaWarmStart::get_hams,
                    &qb::op::QaoaWarmStart::set_hams,
                    qb::op::QaoaWarmStart::help_hams_)
      .def_property("qaoa_step", &qb::op::QaoaWarmStart::get_qaoa_steps,
                    &qb::op::QaoaWarmStart::set_qaoa_step,
                    qb::op::QaoaWarmStart::help_qaoa_steps_)
      .def_property("qaoa_steps", &qb::op::QaoaWarmStart::get_qaoa_steps,
                    &qb::op::QaoaWarmStart::set_qaoa_steps,
                    qb::op::QaoaWarmStart::help_qaoa_steps_)
      .def_property("qn", &qb::op::QaoaWarmStart::get_qns,
                    &qb::op::QaoaWarmStart::set_qn,
                    qb::op::QaoaWarmStart::help_qns_)
      .def_property("qns", &qb::op::QaoaWarmStart::get_qns,
                    &qb::op::QaoaWarmStart::set_qns,
                    qb::op::QaoaWarmStart::help_qns_)
      .def_property("rn", &qb::op::QaoaWarmStart::get_rns,
                    &qb::op::QaoaWarmStart::set_rn,
                    qb::op::QaoaWarmStart::help_rns_)
      .def_property("rns", &qb::op::QaoaWarmStart::get_rns,
                    &qb::op::QaoaWarmStart::set_rns,
                    qb::op::QaoaWarmStart::help_rns_)
      .def_property("sn", &qb::op::QaoaWarmStart::get_sns,
                    &qb::op::QaoaWarmStart::set_sn,
                    qb::op::QaoaWarmStart::help_sns_)
      .def_property("sns", &qb::op::QaoaWarmStart::get_sns,
                    &qb::op::QaoaWarmStart::set_sns,
                    qb::op::QaoaWarmStart::help_sns_)
      .def_property("noise", &qb::op::QaoaWarmStart::get_noises,
                    &qb::op::QaoaWarmStart::set_noise,
                    qb::op::QaoaWarmStart::help_noises_)
      .def_property("noises", &qb::op::QaoaWarmStart::get_noises,
                    &qb::op::QaoaWarmStart::set_noises,
                    qb::op::QaoaWarmStart::help_noises_)
      .def_property("extended_param",
                    &qb::op::QaoaWarmStart::get_extended_params,
                    &qb::op::QaoaWarmStart::set_extended_param,
                    qb::op::QaoaWarmStart::help_extended_params_)
      .def_property("extended_params",
                    &qb::op::QaoaWarmStart::get_extended_params,
                    &qb::op::QaoaWarmStart::set_extended_params,
                    qb::op::QaoaWarmStart::help_extended_params_)
      .def_property("method", &qb::op::QaoaWarmStart::get_methods,
                    &qb::op::QaoaWarmStart::set_method,
                    qb::op::QaoaWarmStart::help_methods_)
      .def_property("methods", &qb::op::QaoaWarmStart::get_methods,
                    &qb::op::QaoaWarmStart::set_methods,
                    qb::op::QaoaWarmStart::help_methods_)
      .def_property("grad", &qb::op::QaoaWarmStart::get_grads,
                    &qb::op::QaoaWarmStart::set_grad,
                    qb::op::QaoaWarmStart::help_grads_)
      .def_property("grads", &qb::op::QaoaWarmStart::get_grads,
                    &qb::op::QaoaWarmStart::set_grads,
                    qb::op::QaoaWarmStart::help_grads_)
      .def_property("gradient_strategy",
                    &qb::op::QaoaWarmStart::get_gradient_strategys,
                    &qb::op::QaoaWarmStart::set_gradient_strategy,
                    qb::op::QaoaWarmStart::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qb::op::QaoaWarmStart::get_gradient_strategys,
                    &qb::op::QaoaWarmStart::set_gradient_strategys,
                    qb::op::QaoaWarmStart::help_gradient_strategys_)
      .def_property("maxeval", &qb::op::QaoaWarmStart::get_maxevals,
                    &qb::op::QaoaWarmStart::set_maxeval,
                    qb::op::QaoaWarmStart::help_maxevals_)
      .def_property("maxevals", &qb::op::QaoaWarmStart::get_maxevals,
                    &qb::op::QaoaWarmStart::set_maxevals,
                    qb::op::QaoaWarmStart::help_maxevals_)
      .def_property("functol", &qb::op::QaoaWarmStart::get_functols,
                    &qb::op::QaoaWarmStart::set_functol,
                    qb::op::QaoaWarmStart::help_functols_)
      .def_property("functols", &qb::op::QaoaWarmStart::get_functols,
                    &qb::op::QaoaWarmStart::set_functols,
                    qb::op::QaoaWarmStart::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qb::op::QaoaWarmStart::get_optimum_energy_abstols,
                    &qb::op::QaoaWarmStart::set_optimum_energy_abstol,
                    qb::op::QaoaWarmStart::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qb::op::QaoaWarmStart::get_optimum_energy_abstols,
                    &qb::op::QaoaWarmStart::set_optimum_energy_abstols,
                    qb::op::QaoaWarmStart::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qb::op::QaoaWarmStart::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaWarmStart::set_optimum_energy_lowerbound,
                    qb::op::QaoaWarmStart::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qb::op::QaoaWarmStart::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaWarmStart::set_optimum_energy_lowerbounds,
                    qb::op::QaoaWarmStart::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate",
                    &qb::op::QaoaWarmStart::get_out_eigenstates,
                    &qb::op::QaoaWarmStart::set_out_eigenstate,
                    qb::op::QaoaWarmStart::help_out_eigenstates_)
      .def_property("out_eigenstates",
                    &qb::op::QaoaWarmStart::get_out_eigenstates,
                    &qb::op::QaoaWarmStart::set_out_eigenstates,
                    qb::op::QaoaWarmStart::help_out_eigenstates_)
      .def_property("out_energy", &qb::op::QaoaWarmStart::get_out_energys,
                    &qb::op::QaoaWarmStart::set_out_energy,
                    qb::op::QaoaWarmStart::help_out_energys_)
      .def_property("out_energys", &qb::op::QaoaWarmStart::get_out_energys,
                    &qb::op::QaoaWarmStart::set_out_energys,
                    qb::op::QaoaWarmStart::help_out_energys_)
      .def_property("out_jacobian", &qb::op::QaoaWarmStart::get_out_jacobians,
                    &qb::op::QaoaWarmStart::set_out_jacobian,
                    qb::op::QaoaWarmStart::help_out_jacobians_)
      .def_property("out_jacobians", &qb::op::QaoaWarmStart::get_out_jacobians,
                    &qb::op::QaoaWarmStart::set_out_jacobians,
                    qb::op::QaoaWarmStart::help_out_jacobians_)
      .def_property("out_theta", &qb::op::QaoaWarmStart::get_out_thetas,
                    &qb::op::QaoaWarmStart::set_out_theta,
                    qb::op::QaoaWarmStart::help_out_thetas_)
      .def_property("out_thetas", &qb::op::QaoaWarmStart::get_out_thetas,
                    &qb::op::QaoaWarmStart::set_out_thetas,
                    qb::op::QaoaWarmStart::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qb::op::QaoaWarmStart::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaWarmStart::set_out_quantum_energy_calc_time,
                    qb::op::QaoaWarmStart::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qb::op::QaoaWarmStart::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaWarmStart::set_out_quantum_energy_calc_times,
                    qb::op::QaoaWarmStart::help_out_quantum_energy_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_time",
          &qb::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times,
          &qb::op::QaoaWarmStart::set_out_quantum_jacobian_calc_time,
          qb::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_times",
          &qb::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times,
          &qb::op::QaoaWarmStart::set_out_quantum_jacobian_calc_times,
          qb::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_"
                    "time",
                    &qb::op::QaoaWarmStart::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaWarmStart::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qb::op::QaoaWarmStart::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_"
                    "times",
                    &qb::op::QaoaWarmStart::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaWarmStart::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qb::op::QaoaWarmStart::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qb::op::QaoaBase::run),
           "Execute all declared experiments under "
           "all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qb::op::QaoaWarmStart::run),
           "runit(i,j) : Execute ansatz i, condition "
           "j")

      .def("__repr__", &qb::op::QaoaWarmStart::get_summary,
           "Print summary of qbos_op_qaoa settings");
}
} // namespace qb