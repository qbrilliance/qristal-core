// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/python/py_optimization.hpp"
#include "qristal/core/python/py_stl_containers.hpp"
#include "qristal/core/optimization/qaoa/qaoa.hpp"

namespace qristal {
void bind_qaoa_warm_start(pybind11::module &opt_m) {
  namespace py = pybind11;
  py::class_<qristal::op::QaoaWarmStart>(opt_m, "qaoa_QaoaWarmStart")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname", &qristal::op::QaoaWarmStart::get_colnames,
                    &qristal::op::QaoaWarmStart::set_colname,
                    qristal::op::QaoaWarmStart::help_colnames_)
      .def_property("colnames", &qristal::op::QaoaWarmStart::get_colnames,
                    &qristal::op::QaoaWarmStart::set_colnames,
                    qristal::op::QaoaWarmStart::help_colnames_)
      .def_property("rowname", &qristal::op::QaoaWarmStart::get_rownames,
                    &qristal::op::QaoaWarmStart::set_rowname,
                    qristal::op::QaoaWarmStart::help_rownames_)
      .def_property("rownames", &qristal::op::QaoaWarmStart::get_rownames,
                    &qristal::op::QaoaWarmStart::set_rownames,
                    qristal::op::QaoaWarmStart::help_rownames_)
      .def_property("theta", &qristal::op::QaoaWarmStart::get_thetas,
                    &qristal::op::QaoaWarmStart::set_theta,
                    qristal::op::QaoaWarmStart::help_thetas_)
      .def_property("thetas", &qristal::op::QaoaWarmStart::get_thetas,
                    &qristal::op::QaoaWarmStart::set_thetas,
                    qristal::op::QaoaWarmStart::help_thetas_)
      .def_property("acc", &qristal::op::QaoaWarmStart::get_accs,
                    &qristal::op::QaoaWarmStart::set_acc,
                    qristal::op::QaoaWarmStart::help_accs_)
      .def_property("accs", &qristal::op::QaoaWarmStart::get_accs,
                    &qristal::op::QaoaWarmStart::set_accs,
                    qristal::op::QaoaWarmStart::help_accs_)
      .def_property("good_cut", &qristal::op::QaoaWarmStart::get_good_cuts,
                    &qristal::op::QaoaWarmStart::set_good_cut,
                    qristal::op::QaoaWarmStart::help_good_cuts_)
      .def_property("good_cuts", &qristal::op::QaoaWarmStart::get_good_cuts,
                    &qristal::op::QaoaWarmStart::set_good_cuts,
                    qristal::op::QaoaWarmStart::help_good_cuts_)
      .def_property("ham", &qristal::op::QaoaWarmStart::get_hams,
                    &qristal::op::QaoaWarmStart::set_ham,
                    qristal::op::QaoaWarmStart::help_hams_)
      .def_property("hams", &qristal::op::QaoaWarmStart::get_hams,
                    &qristal::op::QaoaWarmStart::set_hams,
                    qristal::op::QaoaWarmStart::help_hams_)
      .def_property("qaoa_step", &qristal::op::QaoaWarmStart::get_qaoa_steps,
                    &qristal::op::QaoaWarmStart::set_qaoa_step,
                    qristal::op::QaoaWarmStart::help_qaoa_steps_)
      .def_property("qaoa_steps", &qristal::op::QaoaWarmStart::get_qaoa_steps,
                    &qristal::op::QaoaWarmStart::set_qaoa_steps,
                    qristal::op::QaoaWarmStart::help_qaoa_steps_)
      .def_property("qn", &qristal::op::QaoaWarmStart::get_qns,
                    &qristal::op::QaoaWarmStart::set_qn,
                    qristal::op::QaoaWarmStart::help_qns_)
      .def_property("qns", &qristal::op::QaoaWarmStart::get_qns,
                    &qristal::op::QaoaWarmStart::set_qns,
                    qristal::op::QaoaWarmStart::help_qns_)
      .def_property("rn", &qristal::op::QaoaWarmStart::get_rns,
                    &qristal::op::QaoaWarmStart::set_rn,
                    qristal::op::QaoaWarmStart::help_rns_)
      .def_property("rns", &qristal::op::QaoaWarmStart::get_rns,
                    &qristal::op::QaoaWarmStart::set_rns,
                    qristal::op::QaoaWarmStart::help_rns_)
      .def_property("sn", &qristal::op::QaoaWarmStart::get_sns,
                    &qristal::op::QaoaWarmStart::set_sn,
                    qristal::op::QaoaWarmStart::help_sns_)
      .def_property("sns", &qristal::op::QaoaWarmStart::get_sns,
                    &qristal::op::QaoaWarmStart::set_sns,
                    qristal::op::QaoaWarmStart::help_sns_)
      .def_property("noise", &qristal::op::QaoaWarmStart::get_noises,
                    &qristal::op::QaoaWarmStart::set_noise,
                    qristal::op::QaoaWarmStart::help_noises_)
      .def_property("noises", &qristal::op::QaoaWarmStart::get_noises,
                    &qristal::op::QaoaWarmStart::set_noises,
                    qristal::op::QaoaWarmStart::help_noises_)
      .def_property("extended_param",
                    &qristal::op::QaoaWarmStart::get_extended_params,
                    &qristal::op::QaoaWarmStart::set_extended_param,
                    qristal::op::QaoaWarmStart::help_extended_params_)
      .def_property("extended_params",
                    &qristal::op::QaoaWarmStart::get_extended_params,
                    &qristal::op::QaoaWarmStart::set_extended_params,
                    qristal::op::QaoaWarmStart::help_extended_params_)
      .def_property("method", &qristal::op::QaoaWarmStart::get_methods,
                    &qristal::op::QaoaWarmStart::set_method,
                    qristal::op::QaoaWarmStart::help_methods_)
      .def_property("methods", &qristal::op::QaoaWarmStart::get_methods,
                    &qristal::op::QaoaWarmStart::set_methods,
                    qristal::op::QaoaWarmStart::help_methods_)
      .def_property("grad", &qristal::op::QaoaWarmStart::get_grads,
                    &qristal::op::QaoaWarmStart::set_grad,
                    qristal::op::QaoaWarmStart::help_grads_)
      .def_property("grads", &qristal::op::QaoaWarmStart::get_grads,
                    &qristal::op::QaoaWarmStart::set_grads,
                    qristal::op::QaoaWarmStart::help_grads_)
      .def_property("gradient_strategy",
                    &qristal::op::QaoaWarmStart::get_gradient_strategys,
                    &qristal::op::QaoaWarmStart::set_gradient_strategy,
                    qristal::op::QaoaWarmStart::help_gradient_strategys_)
      .def_property("gradient_strategys",
                    &qristal::op::QaoaWarmStart::get_gradient_strategys,
                    &qristal::op::QaoaWarmStart::set_gradient_strategys,
                    qristal::op::QaoaWarmStart::help_gradient_strategys_)
      .def_property("maxeval", &qristal::op::QaoaWarmStart::get_maxevals,
                    &qristal::op::QaoaWarmStart::set_maxeval,
                    qristal::op::QaoaWarmStart::help_maxevals_)
      .def_property("maxevals", &qristal::op::QaoaWarmStart::get_maxevals,
                    &qristal::op::QaoaWarmStart::set_maxevals,
                    qristal::op::QaoaWarmStart::help_maxevals_)
      .def_property("functol", &qristal::op::QaoaWarmStart::get_functols,
                    &qristal::op::QaoaWarmStart::set_functol,
                    qristal::op::QaoaWarmStart::help_functols_)
      .def_property("functols", &qristal::op::QaoaWarmStart::get_functols,
                    &qristal::op::QaoaWarmStart::set_functols,
                    qristal::op::QaoaWarmStart::help_functols_)
      .def_property("optimum_energy_abstol",
                    &qristal::op::QaoaWarmStart::get_optimum_energy_abstols,
                    &qristal::op::QaoaWarmStart::set_optimum_energy_abstol,
                    qristal::op::QaoaWarmStart::help_optimum_energy_abstols_)
      .def_property("optimum_energy_abstols",
                    &qristal::op::QaoaWarmStart::get_optimum_energy_abstols,
                    &qristal::op::QaoaWarmStart::set_optimum_energy_abstols,
                    qristal::op::QaoaWarmStart::help_optimum_energy_abstols_)
      .def_property("optimum_energy_lowerbound",
                    &qristal::op::QaoaWarmStart::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaWarmStart::set_optimum_energy_lowerbound,
                    qristal::op::QaoaWarmStart::help_optimum_energy_lowerbounds_)
      .def_property("optimum_energy_lowerbounds",
                    &qristal::op::QaoaWarmStart::get_optimum_energy_lowerbounds,
                    &qristal::op::QaoaWarmStart::set_optimum_energy_lowerbounds,
                    qristal::op::QaoaWarmStart::help_optimum_energy_lowerbounds_)
      .def_property("out_eigenstate",
                    &qristal::op::QaoaWarmStart::get_out_eigenstates,
                    &qristal::op::QaoaWarmStart::set_out_eigenstate,
                    qristal::op::QaoaWarmStart::help_out_eigenstates_)
      .def_property("out_eigenstates",
                    &qristal::op::QaoaWarmStart::get_out_eigenstates,
                    &qristal::op::QaoaWarmStart::set_out_eigenstates,
                    qristal::op::QaoaWarmStart::help_out_eigenstates_)
      .def_property("out_energy", &qristal::op::QaoaWarmStart::get_out_energys,
                    &qristal::op::QaoaWarmStart::set_out_energy,
                    qristal::op::QaoaWarmStart::help_out_energys_)
      .def_property("out_energys", &qristal::op::QaoaWarmStart::get_out_energys,
                    &qristal::op::QaoaWarmStart::set_out_energys,
                    qristal::op::QaoaWarmStart::help_out_energys_)
      .def_property("out_jacobian", &qristal::op::QaoaWarmStart::get_out_jacobians,
                    &qristal::op::QaoaWarmStart::set_out_jacobian,
                    qristal::op::QaoaWarmStart::help_out_jacobians_)
      .def_property("out_jacobians", &qristal::op::QaoaWarmStart::get_out_jacobians,
                    &qristal::op::QaoaWarmStart::set_out_jacobians,
                    qristal::op::QaoaWarmStart::help_out_jacobians_)
      .def_property("out_theta", &qristal::op::QaoaWarmStart::get_out_thetas,
                    &qristal::op::QaoaWarmStart::set_out_theta,
                    qristal::op::QaoaWarmStart::help_out_thetas_)
      .def_property("out_thetas", &qristal::op::QaoaWarmStart::get_out_thetas,
                    &qristal::op::QaoaWarmStart::set_out_thetas,
                    qristal::op::QaoaWarmStart::help_out_thetas_)
      .def_property("out_quantum_energy_calc_time",
                    &qristal::op::QaoaWarmStart::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaWarmStart::set_out_quantum_energy_calc_time,
                    qristal::op::QaoaWarmStart::help_out_quantum_energy_calc_times_)
      .def_property("out_quantum_energy_calc_times",
                    &qristal::op::QaoaWarmStart::get_out_quantum_energy_calc_times,
                    &qristal::op::QaoaWarmStart::set_out_quantum_energy_calc_times,
                    qristal::op::QaoaWarmStart::help_out_quantum_energy_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_time",
          &qristal::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times,
          &qristal::op::QaoaWarmStart::set_out_quantum_jacobian_calc_time,
          qristal::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_)
      .def_property(
          "out_quantum_jacobian_calc_times",
          &qristal::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times,
          &qristal::op::QaoaWarmStart::set_out_quantum_jacobian_calc_times,
          qristal::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_"
                    "time",
                    &qristal::op::QaoaWarmStart::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaWarmStart::
                        set_out_classical_energy_jacobian_total_calc_time,
                    qristal::op::QaoaWarmStart::
                        help_out_classical_energy_jacobian_total_calc_times_)
      .def_property("out_classical_energy_jacobian_total_calc_"
                    "times",
                    &qristal::op::QaoaWarmStart::
                        get_out_classical_energy_jacobian_total_calc_times,
                    &qristal::op::QaoaWarmStart::
                        set_out_classical_energy_jacobian_total_calc_times,
                    qristal::op::QaoaWarmStart::
                        help_out_classical_energy_jacobian_total_calc_times_)

      .def("run", py::overload_cast<>(&qristal::op::QaoaBase::run),
           "Execute all declared experiments under "
           "all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(
               &qristal::op::QaoaWarmStart::run),
           "runit(i,j) : Execute ansatz i, condition "
           "j")

      .def("__repr__", &qristal::op::QaoaWarmStart::get_summary,
           "Print summary of qristal_op_qaoa settings");
}
}
