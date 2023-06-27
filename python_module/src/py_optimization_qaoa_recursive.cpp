// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_optimization.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/optimization/qaoa/qaoa.hpp"

namespace qb {
void bind_qaoa_recursive(pybind11::module &opt_m) {
  namespace py = pybind11;
  py::class_<qb::op::QaoaRecursive>(opt_m, "qaoa_QaoaRecursive")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qb::op::QaoaRecursive::get_colnames,
                    &qb::op::QaoaRecursive::set_colname,
                    qb::op::QaoaRecursive::help_colnames_)
      .def_property("colnames", &qb::op::QaoaRecursive::get_colnames,
                    &qb::op::QaoaRecursive::set_colnames,
                    qb::op::QaoaRecursive::help_colnames_)
      .def_property("rowname", &qb::op::QaoaRecursive::get_rownames,
                    &qb::op::QaoaRecursive::set_rowname,
                    qb::op::QaoaRecursive::help_rownames_)
      .def_property("rownames", &qb::op::QaoaRecursive::get_rownames,
                    &qb::op::QaoaRecursive::set_rownames,
                    qb::op::QaoaRecursive::help_rownames_)
      .def_property("acc", &qb::op::QaoaRecursive::get_accs,
                    &qb::op::QaoaRecursive::set_acc,
                    qb::op::QaoaRecursive::help_accs_)
      .def_property("accs", &qb::op::QaoaRecursive::get_accs,
                    &qb::op::QaoaRecursive::set_accs,
                    qb::op::QaoaRecursive::help_accs_)
      .def_property("ham", &qb::op::QaoaRecursive::get_hams,
                    &qb::op::QaoaRecursive::set_ham,
                    qb::op::QaoaRecursive::help_hams_)
      .def_property("hams", &qb::op::QaoaRecursive::get_hams,
                    &qb::op::QaoaRecursive::set_hams,
                    qb::op::QaoaRecursive::help_hams_)
      .def_property("qaoa_step", &qb::op::QaoaRecursive::get_qaoa_steps,
                    &qb::op::QaoaRecursive::set_qaoa_step,
                    qb::op::QaoaRecursive::help_qaoa_steps_)
      .def_property("qaoa_steps", &qb::op::QaoaRecursive::get_qaoa_steps,
                    &qb::op::QaoaRecursive::set_qaoa_steps,
                    qb::op::QaoaRecursive::help_qaoa_steps_)
      .def_property("n_c", &qb::op::QaoaRecursive::get_n_cs,
                    &qb::op::QaoaRecursive::set_n_c,
                    qb::op::QaoaRecursive::help_n_cs_)
      .def_property("n_cs", &qb::op::QaoaRecursive::get_n_cs,
                    &qb::op::QaoaRecursive::set_n_cs,
                    qb::op::QaoaRecursive::help_n_cs_)
      .def_property("qn", &qb::op::QaoaRecursive::get_qns,
                    &qb::op::QaoaRecursive::set_qn,
                    qb::op::QaoaRecursive::help_qns_)
      .def_property("qns", &qb::op::QaoaRecursive::get_qns,
                    &qb::op::QaoaRecursive::set_qns,
                    qb::op::QaoaRecursive::help_qns_)
      .def_property("rn", &qb::op::QaoaRecursive::get_rns,
                    &qb::op::QaoaRecursive::set_rn,
                    qb::op::QaoaRecursive::help_rns_)
      .def_property("rns", &qb::op::QaoaRecursive::get_rns,
                    &qb::op::QaoaRecursive::set_rns,
                    qb::op::QaoaRecursive::help_rns_)
      .def_property("sn", &qb::op::QaoaRecursive::get_sns,
                    &qb::op::QaoaRecursive::set_sn,
                    qb::op::QaoaRecursive::help_sns_)
      .def_property("sns", &qb::op::QaoaRecursive::get_sns,
                    &qb::op::QaoaRecursive::set_sns,
                    qb::op::QaoaRecursive::help_sns_)
      .def_property("noise", &qb::op::QaoaRecursive::get_noises,
                    &qb::op::QaoaRecursive::set_noise,
                    qb::op::QaoaRecursive::help_noises_)
      .def_property("noises", &qb::op::QaoaRecursive::get_noises,
                    &qb::op::QaoaRecursive::set_noises,
                    qb::op::QaoaRecursive::help_noises_)
      .def_property("extended_param",
                    &qb::op::QaoaRecursive::get_extended_params,
                    &qb::op::QaoaRecursive::set_extended_param,
                    qb::op::QaoaRecursive::help_extended_params_)
      .def_property("extended_params",
                    &qb::op::QaoaRecursive::get_extended_params,
                    &qb::op::QaoaRecursive::set_extended_params,
                    qb::op::QaoaRecursive::help_extended_params_)
      .def_property("method", &qb::op::QaoaRecursive::get_methods,
                    &qb::op::QaoaRecursive::set_method,
                    qb::op::QaoaRecursive::help_methods_)
      .def_property("methods", &qb::op::QaoaRecursive::get_methods,
                    &qb::op::QaoaRecursive::set_methods,
                    qb::op::QaoaRecursive::help_methods_)
      .def_property("grad", &qb::op::QaoaRecursive::get_grads,
                    &qb::op::QaoaRecursive::set_grad,
                    qb::op::QaoaRecursive::help_grads_)
      .def_property("grads", &qb::op::QaoaRecursive::get_grads,
                    &qb::op::QaoaRecursive::set_grads,
                    qb::op::QaoaRecursive::help_grads_)
      .def_property("gradient_strategy",
                    &qb::op::QaoaRecursive::get_gradient_strategys,
                    &qb::op::QaoaRecursive::set_gradient_strategy,
                    qb::op::QaoaRecursive::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qb::op::QaoaRecursive::get_gradient_strategys,
                    &qb::op::QaoaRecursive::set_gradient_strategys,
                    qb::op::QaoaRecursive::help_gradient_strategys_)
      .def_property("maxeval", &qb::op::QaoaRecursive::get_maxevals,
                    &qb::op::QaoaRecursive::set_maxeval,
                    qb::op::QaoaRecursive::help_maxevals_)
      .def_property("maxevals", &qb::op::QaoaRecursive::get_maxevals,
                    &qb::op::QaoaRecursive::set_maxevals,
                    qb::op::QaoaRecursive::help_maxevals_)
      .def_property("functol", &qb::op::QaoaRecursive::get_functols,
                    &qb::op::QaoaRecursive::set_functol,
                    qb::op::QaoaRecursive::help_functols_)
      .def_property("functols", &qb::op::QaoaRecursive::get_functols,
                    &qb::op::QaoaRecursive::set_functols,
                    qb::op::QaoaRecursive::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qb::op::QaoaRecursive::get_optimum_energy_abstols,
                    &qb::op::QaoaRecursive::set_optimum_energy_abstol,
                    qb::op::QaoaRecursive::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qb::op::QaoaRecursive::get_optimum_energy_abstols,
                    &qb::op::QaoaRecursive::set_optimum_energy_abstols,
                    qb::op::QaoaRecursive::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qb::op::QaoaRecursive::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaRecursive::set_optimum_energy_lowerbound,
                    qb::op::QaoaRecursive::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qb::op::QaoaRecursive::get_optimum_energy_lowerbounds,
                    &qb::op::QaoaRecursive::set_optimum_energy_lowerbounds,
                    qb::op::QaoaRecursive::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate",
                    &qb::op::QaoaRecursive::get_out_eigenstates,
                    &qb::op::QaoaRecursive::set_out_eigenstate,
                    qb::op::QaoaRecursive::help_out_eigenstates_)
      .def_property("out_eigenstates",
                    &qb::op::QaoaRecursive::get_out_eigenstates,
                    &qb::op::QaoaRecursive::set_out_eigenstates,
                    qb::op::QaoaRecursive::help_out_eigenstates_)
      .def_property("out_energy", &qb::op::QaoaRecursive::get_out_energys,
                    &qb::op::QaoaRecursive::set_out_energy,
                    qb::op::QaoaRecursive::help_out_energys_)
      .def_property("out_energys", &qb::op::QaoaRecursive::get_out_energys,
                    &qb::op::QaoaRecursive::set_out_energys,
                    qb::op::QaoaRecursive::help_out_energys_)
      .def_property("out_jacobian", &qb::op::QaoaRecursive::get_out_jacobians,
                    &qb::op::QaoaRecursive::set_out_jacobian,
                    qb::op::QaoaRecursive::help_out_jacobians_)
      .def_property("out_jacobians", &qb::op::QaoaRecursive::get_out_jacobians,
                    &qb::op::QaoaRecursive::set_out_jacobians,
                    qb::op::QaoaRecursive::help_out_jacobians_)
      .def_property("out_theta", &qb::op::QaoaRecursive::get_out_thetas,
                    &qb::op::QaoaRecursive::set_out_theta,
                    qb::op::QaoaRecursive::help_out_thetas_)
      .def_property("out_thetas", &qb::op::QaoaRecursive::get_out_thetas,
                    &qb::op::QaoaRecursive::set_out_thetas,
                    qb::op::QaoaRecursive::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qb::op::QaoaRecursive::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaRecursive::set_out_quantum_energy_calc_time,
                    qb::op::QaoaRecursive::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qb::op::QaoaRecursive::get_out_quantum_energy_calc_times,
                    &qb::op::QaoaRecursive::set_out_quantum_energy_calc_times,
                    qb::op::QaoaRecursive::help_out_quantum_energy_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_time",
          &qb::op::QaoaRecursive::get_out_quantum_jacobian_calc_times,
          &qb::op::QaoaRecursive::set_out_quantum_jacobian_calc_time,
          qb::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_times",
          &qb::op::QaoaRecursive::get_out_quantum_jacobian_calc_times,
          &qb::op::QaoaRecursive::set_out_quantum_jacobian_calc_times,
          qb::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_time",
                    &qb::op::QaoaRecursive::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaRecursive::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qb::op::QaoaRecursive::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_times",
                    &qb::op::QaoaRecursive::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qb::op::QaoaRecursive::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qb::op::QaoaRecursive::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qb::op::QaoaBase::run),
           "Execute all declared experiments under all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qb::op::QaoaRecursive::run),
           "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", &qb::op::QaoaRecursive::get_summary,
           "Print summary of qb_op_qaoa settings");
}
} // namespace qb