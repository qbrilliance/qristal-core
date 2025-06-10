// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/python/py_optimization.hpp>
#include <qristal/core/python/py_stl_containers.hpp>
#include <qristal/core/optimization/vqee/vqee.hpp>

namespace qristal {
void bind_vqee(pybind11::module &opt_m) {
  namespace py = pybind11;
  // - - - - - - - - - - - - - - - - -  vqee - - - - - - - - - - - - - - - //
  py::module_ m_vqee = opt_m.def_submodule(
      "vqee",
      "Variational Quantum Eigensolver suite within optimization modules");

  py::class_<qristal::vqee::vqe_iteration_data>(m_vqee, "vqe_iteration_data")
      .def(py::init<>())
      .def_readonly("energy", &qristal::vqee::vqe_iteration_data::energy)
      .def_readonly("params", &qristal::vqee::vqe_iteration_data::params);

  py::class_<qristal::vqee::Params>(m_vqee, "Params")
      .def(py::init<>())
      .def_readwrite("circuitString", &qristal::vqee::Params::circuitString)
      .def_readwrite("pauliString", &qristal::vqee::Params::pauliString)
      .def_readwrite("acceleratorName", &qristal::vqee::Params::acceleratorName)
      .def_readwrite("tolerance", &qristal::vqee::Params::tolerance)
      .def_readwrite("nQubits", &qristal::vqee::Params::nQubits)
      .def_readwrite("nShots", &qristal::vqee::Params::nShots)
      .def_readwrite("maxIters", &qristal::vqee::Params::maxIters)
      .def_readwrite("algorithm", &qristal::vqee::Params::algorithm)
      .def_readwrite("extraOptions", &qristal::vqee::Params::extraOptions)
      .def_readwrite("isDeterministic", &qristal::vqee::Params::isDeterministic)
      .def_readonly("energies", &qristal::vqee::Params::energies)
      .def_readonly("iterationData", &qristal::vqee::Params::iterationData)
      .def_readwrite("enableVis", &qristal::vqee::Params::enableVis)
      .def_readwrite("showTheta", &qristal::vqee::Params::showTheta)
      .def_readwrite("limitThetaN", &qristal::vqee::Params::limitThetaN)
      .def_readwrite("tail", &qristal::vqee::Params::tail)
      .def_readwrite("plain", &qristal::vqee::Params::plain)
      .def_readwrite("blocked", &qristal::vqee::Params::blocked)
      .def_readonly("vis", &qristal::vqee::Params::vis)
      .def_readwrite("optimalParameters", &qristal::vqee::Params::theta)
      .def_readonly("optimalValue", &qristal::vqee::Params::optimalValue);

  py::enum_<qristal::vqee::JobID>(m_vqee, "JobID")
      .value("H2_explicit", qristal::vqee::JobID::H2_explicit)
      .value("H1_HEA", qristal::vqee::JobID::H1_HEA)
      .value("H2_UCCSD", qristal::vqee::JobID::H2_UCCSD)
      .value("H2_ASWAP", qristal::vqee::JobID::H2_ASWAP)
      .value("H5_UCCSD", qristal::vqee::JobID::H5_UCCSD);

  m_vqee.def("makeJob", &qristal::vqee::makeJob,
             "makeJob(JobID) -> vqee::Params: returns a predefined example "
             "job setup",
             py::arg("jobID"));

  m_vqee.def("pauliStringFromGeometry", &qristal::vqee::pauliStringFromGeometry,
             "pauliStringFromGeometry(string, string) -> string: returns a "
             "Pauli string generated from molecule geometry using pyscf in "
             "sto-3g basis and Jordan Wigner transformation "
             "Pauli string",
             py::arg("geometry"), py::arg("basis"));

  py::enum_<qristal::vqee::AnsatzID>(m_vqee, "AnsatzID")
      .value("HEA", qristal::vqee::AnsatzID::HEA)
      .value("UCCSD", qristal::vqee::AnsatzID::UCCSD)
      .value("ASWAP", qristal::vqee::AnsatzID::ASWAP);

  // std::size_t setAnsatz(Params& params, const AnsatzID ansatzID, const int
  // nQubits, const int nDEP, const bool TRS=true);
  m_vqee.def("setAnsatz", &qristal::vqee::setAnsatz,
             "setAnsatz(Params, AnsatzID, int, int, bool) -> int: sets Ansatz "
             "in params and returns number of variational parameters "
             "ansatz",
             py::arg("params"), py::arg("ansatzID"), py::arg("nQubits"),
             py::arg("nDEP"), py::arg("TRS"));

  py::class_<qristal::vqee::VQEE>(m_vqee, "VQEE")
      .def(py::init<qristal::vqee::Params &>())
      .def("run", &qristal::vqee::VQEE::optimize, "solve VQE problem");
}
}
