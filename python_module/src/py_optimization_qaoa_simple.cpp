// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_optimization.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/optimization/qaoa/qaoa.hpp"

namespace qb {
void bind_qaoa_simple(pybind11::module &opt_m) {
  namespace py = pybind11;

  py::class_<qb::op::QaoaSimple>(opt_m, "qaoa_QaoaSimple")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qb::op::QaoaSimple::get_colnames,
                    &qb::op::QaoaSimple::set_colname,
                    qb::op::QaoaSimple::help_colnames_)
      .def_property("colnames", &qb::op::QaoaSimple::get_colnames,
                    &qb::op::QaoaSimple::set_colnames,
                    qb::op::QaoaSimple::help_colnames_)
      .def_property("rowname", &qb::op::QaoaSimple::get_rownames,
                    &qb::op::QaoaSimple::set_rowname,
                    qb::op::QaoaSimple::help_rownames_)
      .def_property("rownames", &qb::op::QaoaSimple::get_rownames,
                    &qb::op::QaoaSimple::set_rownames,
                    qb::op::QaoaSimple::help_rownames_)
      .def_property("theta", &qb::op::QaoaSimple::get_thetas,
                    &qb::op::QaoaSimple::set_theta,
                    qb::op::QaoaSimple::help_thetas_)
      .def_property("thetas", &qb::op::QaoaSimple::get_thetas,
                    &qb::op::QaoaSimple::set_thetas,
                    qb::op::QaoaSimple::help_thetas_)
      .def_property("acc", &qb::op::QaoaSimple::get_accs,
                    &qb::op::QaoaSimple::set_acc,
                    qb::op::QaoaSimple::help_accs_)
      .def_property("accs", &qb::op::QaoaSimple::get_accs,
                    &qb::op::QaoaSimple::set_accs,
                    qb::op::QaoaSimple::help_accs_)
      .def_property("ham", &qb::op::QaoaSimple::get_hams,
                    &qb::op::QaoaSimple::set_ham,
                    qb::op::QaoaSimple::help_hams_)
      .def_property("hams", &qb::op::QaoaSimple::get_hams,
                    &qb::op::QaoaSimple::set_hams,
                    qb::op::QaoaSimple::help_hams_)
      .def_property("qaoa_step", &qb::op::QaoaSimple::get_qaoa_steps,
                    &qb::op::QaoaSimple::set_qaoa_step,
                    qb::op::QaoaSimple::help_qaoa_steps_)
      .def_property("qaoa_steps", &qb::op::QaoaSimple::get_qaoa_steps,
                    &qb::op::QaoaSimple::set_qaoa_steps,
                    qb::op::QaoaSimple::help_qaoa_steps_)
      .def_property("qn", &qb::op::QaoaSimple::get_qns,
                    &qb::op::QaoaSimple::set_qn, qb::op::QaoaSimple::help_qns_)
      .def_property("qns", &qb::op::QaoaSimple::get_qns,
                    &qb::op::QaoaSimple::set_qns, qb::op::QaoaSimple::help_qns_)
      .def_property("rn", &qb::op::QaoaSimple::get_rns,
                    &qb::op::QaoaSimple::set_rn, qb::op::QaoaSimple::help_rns_)
      .def_property("rns", &qb::op::QaoaSimple::get_rns,
                    &qb::op::QaoaSimple::set_rns, qb::op::QaoaSimple::help_rns_)
      .def_property("sn", &qb::op::QaoaSimple::get_sns,
                    &qb::op::QaoaSimple::set_sn, qb::op::QaoaSimple::help_sns_)
      .def_property("sns", &qb::op::QaoaSimple::get_sns,
                    &qb::op::QaoaSimple::set_sns, qb::op::QaoaSimple::help_sns_)
      .def_property("noise", &qb::op::QaoaSimple::get_noises,
                    &qb::op::QaoaSimple::set_noise,
                    qb::op::QaoaSimple::help_noises_)
      .def_property("noises", &qb::op::QaoaSimple::get_noises,
                    &qb::op::QaoaSimple::set_noises,
                    qb::op::QaoaSimple::help_noises_)
      .def_property("extended_param", &qb::op::QaoaSimple::get_extended_params,
                    &qb::op::QaoaSimple::set_extended_param,
                    qb::op::QaoaSimple::help_extended_params_)
      .def_property("extended_params", &qb::op::QaoaSimple::get_extended_params,
                    &qb::op::QaoaSimple::set_extended_params,
                    qb::op::QaoaSimple::help_extended_params_)
      .def_property("method", &qb::op::QaoaSimple::get_methods,
                    &qb::op::QaoaSimple::set_method,
                    qb::op::QaoaSimple::help_methods_)
      .def_property("methods", &qb::op::QaoaSimple::get_methods,
                    &qb::op::QaoaSimple::set_methods,
                    qb::op::QaoaSimple::help_methods_)
      .def_property("grad", &qb::op::QaoaSimple::get_grads,
                    &qb::op::QaoaSimple::set_grad,
                    qb::op::QaoaSimple::help_grads_)
      .def_property("grads", &qb::op::QaoaSimple::get_grads,
                    &qb::op::QaoaSimple::set_grads,
                    qb::op::QaoaSimple::help_grads_)
      .def_property("gradient_strategy",
                    &qb::op::QaoaSimple::get_gradient_strategys,
                    &qb::op::QaoaSimple::set_gradient_strategy,
                    qb::op::QaoaSimple::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qb::op::QaoaSimple::get_gradient_strategys,
                    &qb::op::QaoaSimple::set_gradient_strategys,
                    qb::op::QaoaSimple::help_gradient_strategys_)
      .def_property("maxeval", &qb::op::QaoaSimple::get_maxevals,
                    &qb::op::QaoaSimple::set_maxeval,
                    qb::op::QaoaSimple::help_maxevals_)
      .def_property("maxevals", &qb::op::QaoaSimple::get_maxevals,
                    &qb::op::QaoaSimple::set_maxevals,
                    qb::op::QaoaSimple::help_maxevals_)
      .def_property("functol", &qb::op::QaoaSimple::get_functols,
                    &qb::op::QaoaSimple::set_functol,
                    qb::op::QaoaSimple::help_functols_)
      .def_property("functols", &qb::op::QaoaSimple::get_functols,
                    &qb::op::QaoaSimple::set_functols,
                    qb::op::QaoaSimple::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qb::op::QaoaSimple::get_optimum_energy_abstols,
                    &qb::op::QaoaSimple::set_optimum_energy_abstol,
                    qb::op::QaoaSimple::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qb::op::QaoaSimple::get_optimum_energy_abstols,
                    &qb::op::QaoaSimple::set_optimum_energy_abstols,
                    qb::op::QaoaSimple::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qb::op::QaoaSimple::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaSimple::set_optimum_energy_lowerbound,
                    qb::op::QaoaSimple::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qb::op::QaoaSimple::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaSimple::set_optimum_energy_lowerbounds,
                    qb::op::QaoaSimple::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate", &qb::op::QaoaSimple::get_out_eigenstates,
                    &qb::op::QaoaSimple::set_out_eigenstate,
                    qb::op::QaoaSimple::help_out_eigenstates_)
      .def_property("out_eigenstates", &qb::op::QaoaSimple::get_out_eigenstates,
                    &qb::op::QaoaSimple::set_out_eigenstates,
                    qb::op::QaoaSimple::help_out_eigenstates_)
      .def_property("out_energy", &qb::op::QaoaSimple::get_out_energys,
                    &qb::op::QaoaSimple::set_out_energy,
                    qb::op::QaoaSimple::help_out_energys_)
      .def_property("out_energys", &qb::op::QaoaSimple::get_out_energys,
                    &qb::op::QaoaSimple::set_out_energys,
                    qb::op::QaoaSimple::help_out_energys_)
      .def_property("out_jacobian", &qb::op::QaoaSimple::get_out_jacobians,
                    &qb::op::QaoaSimple::set_out_jacobian,
                    qb::op::QaoaSimple::help_out_jacobians_)
      .def_property("out_jacobians", &qb::op::QaoaSimple::get_out_jacobians,
                    &qb::op::QaoaSimple::set_out_jacobians,
                    qb::op::QaoaSimple::help_out_jacobians_)
      .def_property("out_theta", &qb::op::QaoaSimple::get_out_thetas,
                    &qb::op::QaoaSimple::set_out_theta,
                    qb::op::QaoaSimple::help_out_thetas_)
      .def_property("out_thetas", &qb::op::QaoaSimple::get_out_thetas,
                    &qb::op::QaoaSimple::set_out_thetas,
                    qb::op::QaoaSimple::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qb::op::QaoaSimple::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaSimple::set_out_quantum_energy_calc_time,
                    qb::op::QaoaSimple::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qb::op::QaoaSimple::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaSimple::set_out_quantum_energy_calc_times,
                    qb::op::QaoaSimple::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_jacobian_calc_time",
                    &qb::op::QaoaSimple::get_out_quantum_jacobian_calc_times,
                    &qb::op::QaoaSimple::set_out_quantum_jacobian_calc_time,
                    qb::op::QaoaSimple::help_out_quantum_jacobian_calc_times_)
      .def_property("out_quantum_jacobian_calc_times",
                    &qb::op::QaoaSimple::get_out_quantum_jacobian_calc_times,
                    &qb::op::QaoaSimple::set_out_quantum_jacobian_calc_times,
                    qb::op::QaoaSimple::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_time",
                    &qb::op::QaoaSimple::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaSimple::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qb::op::QaoaSimple::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_times",
                    &qb::op::QaoaSimple::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaSimple::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qb::op::QaoaSimple::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qb::op::QaoaBase::run),
           "Execute all declared experiments under all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qb::op::QaoaSimple::run),
           "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", &qb::op::QaoaSimple::get_summary,
           "Print summary of qb_op_qaoa settings");
}
} // namespace qb