#include "py_stl_containers.hpp"
#include "qb/core/typedefs.hpp"
#include <nlohmann/json.hpp>

// JSON conversion routines
namespace std {
void from_json(const nlohmann::json &js, std::complex<double> &elem) {
  if (js.is_array()) {
    throw std::invalid_argument(std::string("JSON: invalid complex element."));
  } else {
    std::complex<double> ret{js["r"].get<double>(), js["i"].get<double>()};
    elem = ret;
  }
}

void to_json(nlohmann::json &j, const std::complex<double> &elem) {
  j = nlohmann::json{{"r", elem.real()}, {"i", elem.imag()}};
}

void from_json(const nlohmann::json &js,
               std::vector<std::complex<double>> &vec) {
  std::vector<std::complex<double>> ret;
  if (js.is_array()) {
    for (auto &elt : js)
      ret.push_back(elt);
    vec = ret;
  } else {
    throw std::invalid_argument(std::string("JSON: invalid complex vector."));
  }
}
} // namespace std
namespace qb {
pybind11::array_t<int> std_vec_to_py_array(const std::vector<int> &input) {
  pybind11::array_t<int> vec(static_cast<int>(input.size()));
  auto r = vec.mutable_unchecked<1>();
  for (int i = 0; i < input.size(); ++i) {
    r(i) = input[i];
  }
  return vec;
}
void bind_opaque_containers(pybind11::module &m) {
  namespace py = pybind11;
  py::bind_vector<Bool>(m, "Bool").def("__repr__", [](const Bool &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorBool>(m, "VectorBool")
      .def(
          "append",
          [](VectorBool &a, const bool &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of Bool")
      .def("__repr__", [](const VectorBool &a) {
        json jret = a;
        return jret.dump();
      });

  py::bind_vector<String>(m, "String").def("__repr__", [](const String &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorString>(m, "VectorString")
      .def(
          "append",
          [](VectorString &a, const std::string &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of String")
      .def("__repr__", [](const VectorString &a) {
        json jret = a;
        return jret.dump();
      });

  py::bind_vector<N>(m, "N").def("__repr__", [](const N &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorN>(m, "VectorN")
      .def(
          "append",
          [](VectorN &a, const size_t &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of N")
      .def("__repr__", [](const VectorN &a) {
        json jret = a;
        return jret.dump();
      });

  py::bind_map<NC>(m, "NC").def("__repr__", [](const NC &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<MapNC>(m, "MapNC").def("__repr__", [](const MapNC &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorMapNC>(m, "VectorMapNC")
      .def(
          "append",
          [](VectorMapNC &a, const std::map<int, std::complex<double>> &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of MapNC")
      .def("__repr__", [](const VectorMapNC &a) {
        json jret = a;
        return jret.dump();
      });

  py::bind_map<ND>(m, "ND").def("__repr__", [](const ND &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<MapND>(m, "MapND").def("__repr__", [](const MapND &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorMapND>(m, "VectorMapND")
      .def(
          "append",
          [](VectorMapND &a, const std::map<int, double> &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of MapND")
      .def("__repr__", [](const VectorMapND &a) {
        json jret = a;
        return jret.dump();
      });

  py::bind_map<NN>(m, "NN").def("__repr__", [](const NN &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<MapNN>(m, "MapNN").def("__repr__", [](const MapNN &a) {
    json jret = a;
    return jret.dump();
  });
  py::bind_vector<VectorMapNN>(m, "VectorMapNN")
      .def(
          "append",
          [](VectorMapNN &a, const std::map<int, int> &value) {
            auto aa = a.begin();
            aa->push_back(value);
          },
          py::arg("x"), "Add an element to the end of MapNN")
      .def("__repr__", [](const VectorMapNN &a) {
        json jret = a;
        return jret.dump();
      });

  // Additional binding overloads for std::map<std::vector<size_t>, double>
  // so that operator[] getter/setter with a generic Python array will *just*
  // work.
  // Note: since we have an opaque type declaration for
  // std::vector<size_t>, Python could get confused and ask for an explicit
  // construction of `qb::N` type.
  py::bind_map<std::map<std::vector<size_t>, double>>(m, "IntVecToFloatMap")
      .def(py::init<>())
      .def("__repr__",
           [](const std::map<std::vector<size_t>, double> &self) {
             json jret = self;
             return jret.dump();
           })
      .def("__getitem__",
           // Get (if preset) or initialize to 0.0 (if not yet set)
           // Use generic py::array_t argument type to accept generic Python
           // integer arrays.
           [](std::map<std::vector<size_t>, double> &self,
              const py::array_t<size_t> &key) {
             const auto iter = self.find(py_array_to_std_vec(key));
             if (iter != self.end()) {
               return iter->second;
             } else {
               self[py_array_to_std_vec(key)] = 0.0;
               return 0.0;
             }
           })
      .def("__setitem__", [](std::map<std::vector<size_t>, double> &self,
                             const py::array_t<size_t> &key, double &value) {
        return self[py_array_to_std_vec(key)] = value;
      });

  py::bind_map<
      std::unordered_map<std::string, std::map<std::vector<size_t>, double>>>(
      m, "DictStringToIntVecToFloatMap")
      .def(py::init<>())
      .def("__repr__",
           [](const std::unordered_map<
               std::string, std::map<std::vector<size_t>, double>> &self) {
             json jret = self;
             return jret.dump();
           })
      // Special overload to handle empty dict initialization of a map entry.
      // e.g., my_map["abc"] = {}
      // Note: Python dict is a hash map, hence, dict(std::vector<size_t> ->
      // double) is not constructable in Python (list is not 'hashable').
      // Therefore, we don't expect to handle a non-empty dict here.
      .def("__setitem__",
           [](std::unordered_map<std::string,
                                 std::map<std::vector<size_t>, double>> &self,
              const std::string &key, const py::dict &value) {
             if (value.empty()) {
               self[key] = {};
             } else {
               throw py::type_error("incompatible function arguments");
             }
           })
      .def("__setitem__",
           [](std::unordered_map<std::string,
                                 std::map<std::vector<size_t>, double>> &self,
              const std::string &key,
              const std::map<std::vector<size_t>, double> &value) {
             self[key] = value;
           });
}
} // namespace qb