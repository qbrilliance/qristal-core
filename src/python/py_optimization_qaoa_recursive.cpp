// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/python/py_optimization.hpp"
#include "qristal/core/python/py_stl_containers.hpp"
#include "qristal/core/optimization/qaoa/qaoa.hpp"

namespace qristal {
void bind_qaoa_recursive(pybind11::module &opt_m) {
  namespace py = pybind11;
  py::class_<qristal::op::QaoaRecursive>(opt_m, "qaoa_QaoaRecursive")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qristal::op::QaoaRecursive::get_colnames,
                    &qristal::op::QaoaRecursive::set_colname,
                    qristal::op::QaoaRecursive::help_colnames_)
      .def_property("colnames", &qristal::op::QaoaRecursive::get_colnames,
                    &qristal::op::QaoaRecursive::set_colnames,
                    qristal::op::QaoaRecursive::help_colnames_)
      .def_property("rowname", &qristal::op::QaoaRecursive::get_rownames,
                    &qristal::op::QaoaRecursive::set_rowname,
                    qristal::op::QaoaRecursive::help_rownames_)
      .def_property("rownames", &qristal::op::QaoaRecursive::get_rownames,
                    &qristal::op::QaoaRecursive::set_rownames,
                    qristal::op::QaoaRecursive::help_rownames_)
      .def_property("acc", &qristal::op::QaoaRecursive::get_accs,
                    &qristal::op::QaoaRecursive::set_acc,
                    qristal::op::QaoaRecursive::help_accs_)
      .def_property("accs", &qristal::op::QaoaRecursive::get_accs,
                    &qristal::op::QaoaRecursive::set_accs,
                    qristal::op::QaoaRecursive::help_accs_)
      .def_property("ham", &qristal::op::QaoaRecursive::get_hams,
                    &qristal::op::QaoaRecursive::set_ham,
                    qristal::op::QaoaRecursive::help_hams_)
      .def_property("hams", &qristal::op::QaoaRecursive::get_hams,
                    &qristal::op::QaoaRecursive::set_hams,
                    qristal::op::QaoaRecursive::help_hams_)
      .def_property("qaoa_step", &qristal::op::QaoaRecursive::get_qaoa_steps,
                    &qristal::op::QaoaRecursive::set_qaoa_step,
                    qristal::op::QaoaRecursive::help_qaoa_steps_)
      .def_property("qaoa_steps", &qristal::op::QaoaRecursive::get_qaoa_steps,
                    &qristal::op::QaoaRecursive::set_qaoa_steps,
                    qristal::op::QaoaRecursive::help_qaoa_steps_)
      .def_property("n_c", &qristal::op::QaoaRecursive::get_n_cs,
                    &qristal::op::QaoaRecursive::set_n_c,
                    qristal::op::QaoaRecursive::help_n_cs_)
      .def_property("n_cs", &qristal::op::QaoaRecursive::get_n_cs,
                    &qristal::op::QaoaRecursive::set_n_cs,
                    qristal::op::QaoaRecursive::help_n_cs_)
      .def_property("qn", &qristal::op::QaoaRecursive::get_qns,
                    &qristal::op::QaoaRecursive::set_qn,
                    qristal::op::QaoaRecursive::help_qns_)
      .def_property("qns", &qristal::op::QaoaRecursive::get_qns,
                    &qristal::op::QaoaRecursive::set_qns,
                    qristal::op::QaoaRecursive::help_qns_)
      .def_property("rn", &qristal::op::QaoaRecursive::get_rns,
                    &qristal::op::QaoaRecursive::set_rn,
                    qristal::op::QaoaRecursive::help_rns_)
      .def_property("rns", &qristal::op::QaoaRecursive::get_rns,
                    &qristal::op::QaoaRecursive::set_rns,
                    qristal::op::QaoaRecursive::help_rns_)
      .def_property("sn", &qristal::op::QaoaRecursive::get_sns,
                    &qristal::op::QaoaRecursive::set_sn,
                    qristal::op::QaoaRecursive::help_sns_)
      .def_property("sns", &qristal::op::QaoaRecursive::get_sns,
                    &qristal::op::QaoaRecursive::set_sns,
                    qristal::op::QaoaRecursive::help_sns_)
      .def_property("noise", &qristal::op::QaoaRecursive::get_noises,
                    &qristal::op::QaoaRecursive::set_noise,
                    qristal::op::QaoaRecursive::help_noises_)
      .def_property("noises", &qristal::op::QaoaRecursive::get_noises,
                    &qristal::op::QaoaRecursive::set_noises,
                    qristal::op::QaoaRecursive::help_noises_)
      .def_property("extended_param",
                    &qristal::op::QaoaRecursive::get_extended_params,
                    &qristal::op::QaoaRecursive::set_extended_param,
                    qristal::op::QaoaRecursive::help_extended_params_)
      .def_property("extended_params",
                    &qristal::op::QaoaRecursive::get_extended_params,
                    &qristal::op::QaoaRecursive::set_extended_params,
                    qristal::op::QaoaRecursive::help_extended_params_)
      .def_property("method", &qristal::op::QaoaRecursive::get_methods,
                    &qristal::op::QaoaRecursive::set_method,
                    qristal::op::QaoaRecursive::help_methods_)
      .def_property("methods", &qristal::op::QaoaRecursive::get_methods,
                    &qristal::op::QaoaRecursive::set_methods,
                    qristal::op::QaoaRecursive::help_methods_)
      .def_property("grad", &qristal::op::QaoaRecursive::get_grads,
                    &qristal::op::QaoaRecursive::set_grad,
                    qristal::op::QaoaRecursive::help_grads_)
      .def_property("grads", &qristal::op::QaoaRecursive::get_grads,
                    &qristal::op::QaoaRecursive::set_grads,
                    qristal::op::QaoaRecursive::help_grads_)
      .def_property("gradient_strategy",
                    &qristal::op::QaoaRecursive::get_gradient_strategys,
                    &qristal::op::QaoaRecursive::set_gradient_strategy,
                    qristal::op::QaoaRecursive::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qristal::op::QaoaRecursive::get_gradient_strategys,
                    &qristal::op::QaoaRecursive::set_gradient_strategys,
                    qristal::op::QaoaRecursive::help_gradient_strategys_)
      .def_property("maxeval", &qristal::op::QaoaRecursive::get_maxevals,
                    &qristal::op::QaoaRecursive::set_maxeval,
                    qristal::op::QaoaRecursive::help_maxevals_)
      .def_property("maxevals", &qristal::op::QaoaRecursive::get_maxevals,
                    &qristal::op::QaoaRecursive::set_maxevals,
                    qristal::op::QaoaRecursive::help_maxevals_)
      .def_property("functol", &qristal::op::QaoaRecursive::get_functols,
                    &qristal::op::QaoaRecursive::set_functol,
                    qristal::op::QaoaRecursive::help_functols_)
      .def_property("functols", &qristal::op::QaoaRecursive::get_functols,
                    &qristal::op::QaoaRecursive::set_functols,
                    qristal::op::QaoaRecursive::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qristal::op::QaoaRecursive::get_optimum_energy_abstols,
                    &qristal::op::QaoaRecursive::set_optimum_energy_abstol,
                    qristal::op::QaoaRecursive::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qristal::op::QaoaRecursive::get_optimum_energy_abstols,
                    &qristal::op::QaoaRecursive::set_optimum_energy_abstols,
                    qristal::op::QaoaRecursive::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qristal::op::QaoaRecursive::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaRecursive::set_optimum_energy_lowerbound,
                    qristal::op::QaoaRecursive::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qristal::op::QaoaRecursive::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaRecursive::set_optimum_energy_lowerbounds,
                    qristal::op::QaoaRecursive::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate",
                    &qristal::op::QaoaRecursive::get_out_eigenstates,
                    &qristal::op::QaoaRecursive::set_out_eigenstate,
                    qristal::op::QaoaRecursive::help_out_eigenstates_)
      .def_property("out_eigenstates",
                    &qristal::op::QaoaRecursive::get_out_eigenstates,
                    &qristal::op::QaoaRecursive::set_out_eigenstates,
                    qristal::op::QaoaRecursive::help_out_eigenstates_)
      .def_property("out_energy", &qristal::op::QaoaRecursive::get_out_energys,
                    &qristal::op::QaoaRecursive::set_out_energy,
                    qristal::op::QaoaRecursive::help_out_energys_)
      .def_property("out_energys", &qristal::op::QaoaRecursive::get_out_energys,
                    &qristal::op::QaoaRecursive::set_out_energys,
                    qristal::op::QaoaRecursive::help_out_energys_)
      .def_property("out_jacobian", &qristal::op::QaoaRecursive::get_out_jacobians,
                    &qristal::op::QaoaRecursive::set_out_jacobian,
                    qristal::op::QaoaRecursive::help_out_jacobians_)
      .def_property("out_jacobians", &qristal::op::QaoaRecursive::get_out_jacobians,
                    &qristal::op::QaoaRecursive::set_out_jacobians,
                    qristal::op::QaoaRecursive::help_out_jacobians_)
      .def_property("out_theta", &qristal::op::QaoaRecursive::get_out_thetas,
                    &qristal::op::QaoaRecursive::set_out_theta,
                    qristal::op::QaoaRecursive::help_out_thetas_)
      .def_property("out_thetas", &qristal::op::QaoaRecursive::get_out_thetas,
                    &qristal::op::QaoaRecursive::set_out_thetas,
                    qristal::op::QaoaRecursive::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qristal::op::QaoaRecursive::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaRecursive::set_out_quantum_energy_calc_time,
                    qristal::op::QaoaRecursive::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qristal::op::QaoaRecursive::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaRecursive::set_out_quantum_energy_calc_times,
                    qristal::op::QaoaRecursive::help_out_quantum_energy_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_time",
          &qristal::op::QaoaRecursive::get_out_quantum_jacobian_calc_times,
          &qristal::op::QaoaRecursive::set_out_quantum_jacobian_calc_time,
          qristal::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_times",
          &qristal::op::QaoaRecursive::get_out_quantum_jacobian_calc_times,
          &qristal::op::QaoaRecursive::set_out_quantum_jacobian_calc_times,
          qristal::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_time",
                    &qristal::op::QaoaRecursive::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaRecursive::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qristal::op::QaoaRecursive::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_times",
                    &qristal::op::QaoaRecursive::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaRecursive::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qristal::op::QaoaRecursive::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qristal::op::QaoaBase::run),
           "Execute all declared experiments under all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qristal::op::QaoaRecursive::run),
           "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", &qristal::op::QaoaRecursive::get_summary,
           "Print summary of qb_op_qaoa settings");
}
}
