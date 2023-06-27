// Copyright (c) Quantum Brilliance Pty Ltd
#include "py_optimization.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/optimization/vqee/vqee.hpp"

namespace qb {
void bind_vqee(pybind11::module &opt_m) {
  namespace py = pybind11;
  // - - - - - - - - - - - - - - - - -  vqee - - - - - - - - - - - - - - - //
  py::module_ m_vqee = opt_m.def_submodule(
      "vqee",
      "Variational Quantum Eigensolver suite within optimization modules");

  py::class_<qb::vqee::vqe_iteration_data>(m_vqee, "vqe_iteration_data")
      .def(py::init<>())
      .def_readonly("energy", &qb::vqee::vqe_iteration_data::energy)
      .def_readonly("params", &qb::vqee::vqe_iteration_data::params);

  py::class_<qb::vqee::Params>(m_vqee, "Params")
      .def(py::init<>())
      .def_readwrite("circuitString", &qb::vqee::Params::circuitString)
      .def_readwrite("pauliString", &qb::vqee::Params::pauliString)
      .def_readwrite("acceleratorName", &qb::vqee::Params::acceleratorName)
      .def_readwrite("tolerance", &qb::vqee::Params::tolerance)
      .def_readwrite("nQubits", &qb::vqee::Params::nQubits)
      .def_readwrite("nShots", &qb::vqee::Params::nShots)
      .def_readwrite("maxIters", &qb::vqee::Params::maxIters)
      .def_readwrite("isDeterministic", &qb::vqee::Params::isDeterministic)
      .def_readonly("energies", &qb::vqee::Params::energies)
      .def_readonly("iterationData", &qb::vqee::Params::iterationData)
      .def_readwrite("enableVis", &qb::vqee::Params::enableVis)
      .def_readwrite("showTheta", &qb::vqee::Params::showTheta)
      .def_readwrite("limitThetaN", &qb::vqee::Params::limitThetaN)
      .def_readwrite("tail", &qb::vqee::Params::tail)
      .def_readwrite("plain", &qb::vqee::Params::plain)
      .def_readwrite("blocked", &qb::vqee::Params::blocked)
      .def_readonly("vis", &qb::vqee::Params::vis)
      .def_readwrite("optimalParameters", &qb::vqee::Params::theta)
      .def_readonly("optimalValue", &qb::vqee::Params::optimalValue);

  py::enum_<qb::vqee::JobID>(m_vqee, "JobID")
      .value("H2_explicit", qb::vqee::JobID::H2_explicit)
      .value("H1_HEA", qb::vqee::JobID::H1_HEA)
      .value("H2_UCCSD", qb::vqee::JobID::H2_UCCSD)
      .value("H2_ASWAP", qb::vqee::JobID::H2_ASWAP)
      .value("H5_UCCSD", qb::vqee::JobID::H5_UCCSD);

  m_vqee.def("makeJob", &qb::vqee::makeJob,
             "makeJob(JobID) -> vqee::Params: returns a predefined example "
             "job setup",
             py::arg("jobID"));

  m_vqee.def("pauliStringFromGeometry", &qb::vqee::pauliStringFromGeometry,
             "pauliStringFromGeometry(string, string) -> string: returns a "
             "Pauli string generated from molecule geometry using pyscf in "
             "sto-3g basis and Jordan Wigner transformation "
             "Pauli string",
             py::arg("geometry"), py::arg("basis"));

  py::enum_<qb::vqee::AnsatzID>(m_vqee, "AnsatzID")
      .value("HEA", qb::vqee::AnsatzID::HEA)
      .value("UCCSD", qb::vqee::AnsatzID::UCCSD)
      .value("ASWAP", qb::vqee::AnsatzID::ASWAP);

  // std::size_t setAnsatz(Params& params, const AnsatzID ansatzID, const int
  // nQubits, const int nDEP, const bool TRS=true);
  m_vqee.def("setAnsatz", &qb::vqee::setAnsatz,
             "setAnsatz(Params, AnsatzID, int, int, bool) -> int: sets Ansatz "
             "in params and returns number of variational parameters "
             "ansatz",
             py::arg("params"), py::arg("ansatzID"), py::arg("nQubits"),
             py::arg("nDEP"), py::arg("TRS"));

  py::class_<qb::vqee::VQEE>(m_vqee, "VQEE")
      .def(py::init<qb::vqee::Params &>())
      .def("run", &qb::vqee::VQEE::optimize, "solve VQE problem");
}
} // namespace qb