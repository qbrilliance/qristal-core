// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_optimization.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/optimization/qml/qml.hpp"

namespace qb {
void bind_qml(pybind11::module &opt_m) {
  namespace py = pybind11;

  py::enum_<qb::qml::DefaultAnsatzes>(opt_m, "defaultAnsatzes")
      .value("qrlRDBMS", qb::qml::DefaultAnsatzes::qrlRDBMS);

  py::class_<qb::qml::ParamCirc, qb::CircuitBuilder>(opt_m, "ParamCirc")
      .def(py::init<size_t, qb::qml::DefaultAnsatzes, size_t,
                    qb::VectorString>())
      .def(py::init<size_t>())
      .def("numInputs", &qb::qml::ParamCirc::getNumInputs)
      .def("numParams", &qb::qml::ParamCirc::getNumParams)
      .def("numQubits", &qb::qml::ParamCirc::getNumQubits)
      .def("numAnsatzRepetitions_",
           &qb::qml::ParamCirc::getNumAnsatzRepetitions)
      .def("rx", &qb::qml::ParamCirc::RX)
      .def("ry", &qb::qml::ParamCirc::RY)
      .def("rz", &qb::qml::ParamCirc::RZ)
      .def("u1", &qb::qml::ParamCirc::U1)
      .def("cphase", &qb::qml::ParamCirc::CPhase)
      .def("reupload", &qb::qml::ParamCirc::reupload);

  py::class_<qb::qml::QMLExecutor>(opt_m, "QMLExecutor")
      .def(py::init<qb::qml::ParamCirc &, std::vector<double>,
                    std::vector<double>>())
      .def_property("circuit", &qb::qml::QMLExecutor::getCircuit,
                    &qb::qml::QMLExecutor::setCircuit)
      .def_property("inputParams", &qb::qml::QMLExecutor::getInputParams,
                    &qb::qml::QMLExecutor::setInputParams)
      .def_property("weights", &qb::qml::QMLExecutor::getWeights,
                    &qb::qml::QMLExecutor::setWeights)
      .def_readwrite("acc", &qb::qml::QMLExecutor::acc)
      .def("run", &qb::qml::QMLExecutor::run)
      .def("getStats", &qb::qml::QMLExecutor::getStats)
      .def("runGradients", &qb::qml::QMLExecutor::runGradients)
      .def("getGradients", &qb::qml::QMLExecutor::getStatGradients);
}
} // namespace qb