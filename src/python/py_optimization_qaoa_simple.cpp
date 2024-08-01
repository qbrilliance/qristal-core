// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/python/py_optimization.hpp"
#include "qristal/core/python/py_stl_containers.hpp"
#include "qristal/core/optimization/qaoa/qaoa.hpp"

namespace qristal {
void bind_qaoa_simple(pybind11::module &opt_m) {
  namespace py = pybind11;

  py::class_<qristal::op::QaoaSimple>(opt_m, "qaoa_QaoaSimple")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qristal::op::QaoaSimple::get_colnames,
                    &qristal::op::QaoaSimple::set_colname,
                    qristal::op::QaoaSimple::help_colnames_)
      .def_property("colnames", &qristal::op::QaoaSimple::get_colnames,
                    &qristal::op::QaoaSimple::set_colnames,
                    qristal::op::QaoaSimple::help_colnames_)
      .def_property("rowname", &qristal::op::QaoaSimple::get_rownames,
                    &qristal::op::QaoaSimple::set_rowname,
                    qristal::op::QaoaSimple::help_rownames_)
      .def_property("rownames", &qristal::op::QaoaSimple::get_rownames,
                    &qristal::op::QaoaSimple::set_rownames,
                    qristal::op::QaoaSimple::help_rownames_)
      .def_property("theta", &qristal::op::QaoaSimple::get_thetas,
                    &qristal::op::QaoaSimple::set_theta,
                    qristal::op::QaoaSimple::help_thetas_)
      .def_property("thetas", &qristal::op::QaoaSimple::get_thetas,
                    &qristal::op::QaoaSimple::set_thetas,
                    qristal::op::QaoaSimple::help_thetas_)
      .def_property("acc", &qristal::op::QaoaSimple::get_accs,
                    &qristal::op::QaoaSimple::set_acc,
                    qristal::op::QaoaSimple::help_accs_)
      .def_property("accs", &qristal::op::QaoaSimple::get_accs,
                    &qristal::op::QaoaSimple::set_accs,
                    qristal::op::QaoaSimple::help_accs_)
      .def_property("ham", &qristal::op::QaoaSimple::get_hams,
                    &qristal::op::QaoaSimple::set_ham,
                    qristal::op::QaoaSimple::help_hams_)
      .def_property("hams", &qristal::op::QaoaSimple::get_hams,
                    &qristal::op::QaoaSimple::set_hams,
                    qristal::op::QaoaSimple::help_hams_)
      .def_property("qaoa_step", &qristal::op::QaoaSimple::get_qaoa_steps,
                    &qristal::op::QaoaSimple::set_qaoa_step,
                    qristal::op::QaoaSimple::help_qaoa_steps_)
      .def_property("qaoa_steps", &qristal::op::QaoaSimple::get_qaoa_steps,
                    &qristal::op::QaoaSimple::set_qaoa_steps,
                    qristal::op::QaoaSimple::help_qaoa_steps_)
      .def_property("qn", &qristal::op::QaoaSimple::get_qns,
                    &qristal::op::QaoaSimple::set_qn, qristal::op::QaoaSimple::help_qns_)
      .def_property("qns", &qristal::op::QaoaSimple::get_qns,
                    &qristal::op::QaoaSimple::set_qns, qristal::op::QaoaSimple::help_qns_)
      .def_property("rn", &qristal::op::QaoaSimple::get_rns,
                    &qristal::op::QaoaSimple::set_rn, qristal::op::QaoaSimple::help_rns_)
      .def_property("rns", &qristal::op::QaoaSimple::get_rns,
                    &qristal::op::QaoaSimple::set_rns, qristal::op::QaoaSimple::help_rns_)
      .def_property("sn", &qristal::op::QaoaSimple::get_sns,
                    &qristal::op::QaoaSimple::set_sn, qristal::op::QaoaSimple::help_sns_)
      .def_property("sns", &qristal::op::QaoaSimple::get_sns,
                    &qristal::op::QaoaSimple::set_sns, qristal::op::QaoaSimple::help_sns_)
      .def_property("noise", &qristal::op::QaoaSimple::get_noises,
                    &qristal::op::QaoaSimple::set_noise,
                    qristal::op::QaoaSimple::help_noises_)
      .def_property("noises", &qristal::op::QaoaSimple::get_noises,
                    &qristal::op::QaoaSimple::set_noises,
                    qristal::op::QaoaSimple::help_noises_)
      .def_property("extended_param", &qristal::op::QaoaSimple::get_extended_params,
                    &qristal::op::QaoaSimple::set_extended_param,
                    qristal::op::QaoaSimple::help_extended_params_)
      .def_property("extended_params", &qristal::op::QaoaSimple::get_extended_params,
                    &qristal::op::QaoaSimple::set_extended_params,
                    qristal::op::QaoaSimple::help_extended_params_)
      .def_property("method", &qristal::op::QaoaSimple::get_methods,
                    &qristal::op::QaoaSimple::set_method,
                    qristal::op::QaoaSimple::help_methods_)
      .def_property("methods", &qristal::op::QaoaSimple::get_methods,
                    &qristal::op::QaoaSimple::set_methods,
                    qristal::op::QaoaSimple::help_methods_)
      .def_property("grad", &qristal::op::QaoaSimple::get_grads,
                    &qristal::op::QaoaSimple::set_grad,
                    qristal::op::QaoaSimple::help_grads_)
      .def_property("grads", &qristal::op::QaoaSimple::get_grads,
                    &qristal::op::QaoaSimple::set_grads,
                    qristal::op::QaoaSimple::help_grads_)
      .def_property("gradient_strategy",
                    &qristal::op::QaoaSimple::get_gradient_strategys,
                    &qristal::op::QaoaSimple::set_gradient_strategy,
                    qristal::op::QaoaSimple::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qristal::op::QaoaSimple::get_gradient_strategys,
                    &qristal::op::QaoaSimple::set_gradient_strategys,
                    qristal::op::QaoaSimple::help_gradient_strategys_)
      .def_property("maxeval", &qristal::op::QaoaSimple::get_maxevals,
                    &qristal::op::QaoaSimple::set_maxeval,
                    qristal::op::QaoaSimple::help_maxevals_)
      .def_property("maxevals", &qristal::op::QaoaSimple::get_maxevals,
                    &qristal::op::QaoaSimple::set_maxevals,
                    qristal::op::QaoaSimple::help_maxevals_)
      .def_property("functol", &qristal::op::QaoaSimple::get_functols,
                    &qristal::op::QaoaSimple::set_functol,
                    qristal::op::QaoaSimple::help_functols_)
      .def_property("functols", &qristal::op::QaoaSimple::get_functols,
                    &qristal::op::QaoaSimple::set_functols,
                    qristal::op::QaoaSimple::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qristal::op::QaoaSimple::get_optimum_energy_abstols,
                    &qristal::op::QaoaSimple::set_optimum_energy_abstol,
                    qristal::op::QaoaSimple::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qristal::op::QaoaSimple::get_optimum_energy_abstols,
                    &qristal::op::QaoaSimple::set_optimum_energy_abstols,
                    qristal::op::QaoaSimple::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qristal::op::QaoaSimple::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaSimple::set_optimum_energy_lowerbound,
                    qristal::op::QaoaSimple::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qristal::op::QaoaSimple::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaSimple::set_optimum_energy_lowerbounds,
                    qristal::op::QaoaSimple::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate", &qristal::op::QaoaSimple::get_out_eigenstates,
                    &qristal::op::QaoaSimple::set_out_eigenstate,
                    qristal::op::QaoaSimple::help_out_eigenstates_)
      .def_property("out_eigenstates", &qristal::op::QaoaSimple::get_out_eigenstates,
                    &qristal::op::QaoaSimple::set_out_eigenstates,
                    qristal::op::QaoaSimple::help_out_eigenstates_)
      .def_property("out_energy", &qristal::op::QaoaSimple::get_out_energys,
                    &qristal::op::QaoaSimple::set_out_energy,
                    qristal::op::QaoaSimple::help_out_energys_)
      .def_property("out_energys", &qristal::op::QaoaSimple::get_out_energys,
                    &qristal::op::QaoaSimple::set_out_energys,
                    qristal::op::QaoaSimple::help_out_energys_)
      .def_property("out_jacobian", &qristal::op::QaoaSimple::get_out_jacobians,
                    &qristal::op::QaoaSimple::set_out_jacobian,
                    qristal::op::QaoaSimple::help_out_jacobians_)
      .def_property("out_jacobians", &qristal::op::QaoaSimple::get_out_jacobians,
                    &qristal::op::QaoaSimple::set_out_jacobians,
                    qristal::op::QaoaSimple::help_out_jacobians_)
      .def_property("out_theta", &qristal::op::QaoaSimple::get_out_thetas,
                    &qristal::op::QaoaSimple::set_out_theta,
                    qristal::op::QaoaSimple::help_out_thetas_)
      .def_property("out_thetas", &qristal::op::QaoaSimple::get_out_thetas,
                    &qristal::op::QaoaSimple::set_out_thetas,
                    qristal::op::QaoaSimple::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qristal::op::QaoaSimple::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaSimple::set_out_quantum_energy_calc_time,
                    qristal::op::QaoaSimple::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qristal::op::QaoaSimple::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaSimple::set_out_quantum_energy_calc_times,
                    qristal::op::QaoaSimple::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_jacobian_calc_time",
                    &qristal::op::QaoaSimple::get_out_quantum_jacobian_calc_times,
                    &qristal::op::QaoaSimple::set_out_quantum_jacobian_calc_time,
                    qristal::op::QaoaSimple::help_out_quantum_jacobian_calc_times_)
      .def_property("out_quantum_jacobian_calc_times",
                    &qristal::op::QaoaSimple::get_out_quantum_jacobian_calc_times,
                    &qristal::op::QaoaSimple::set_out_quantum_jacobian_calc_times,
                    qristal::op::QaoaSimple::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_time",
                    &qristal::op::QaoaSimple::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaSimple::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qristal::op::QaoaSimple::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_times",
                    &qristal::op::QaoaSimple::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaSimple::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qristal::op::QaoaSimple::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qristal::op::QaoaBase::run),
           "Execute all declared experiments under all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qristal::op::QaoaSimple::run),
           "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", &qristal::op::QaoaSimple::get_summary,
           "Print summary of qb_op_qaoa settings");
}
}
