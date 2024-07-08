#include "qb/core/python/py_stl_containers.hpp"
#include "qb/core/typedefs.hpp"
#include <pybind11/stl_bind.h>
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

  void bind_opaque_containers(pybind11::module &m) {

    namespace py = pybind11;
    py::bind_vector<std::vector<std::string>>(m, "VectorString");
    py::bind_vector<std::vector<size_t>>(m, "VectorSize_t");
    py::bind_vector<std::vector<bool>>(m, "VectorBool");
    py::bind_vector<std::vector<std::vector<size_t>>>(m, "TableSize_t");
    py::bind_vector<std::vector<std::vector<std::string>>>(m, "TableString");
    py::bind_vector<std::vector<std::vector<bool>>>(m, "TableBool");
    py::bind_map<std::map<int, double>>(m, "MapIntDouble");
    py::bind_map<std::map<int, std::complex<double>>>(m, "MapIntComplex");
    py::bind_map<std::map<std::vector<bool>, int>>(m, "MapVectorBoolInt");
    py::bind_vector<std::vector<std::map<int, double>>>(m, "VectorMapIntDouble");
    py::bind_vector<std::vector<std::map<int, std::complex<double>>>>(m, "VectorMapIntComplex");
    py::bind_vector<std::vector<std::map<std::vector<bool>, int>>>(m, "VectorMapVectorBoolInt");
    py::bind_vector<std::vector<std::vector<std::map<int, std::complex<double>>>>>(m, "TableMapIntComplex");
    py::bind_vector<std::vector<std::vector<std::map<int, double>>>>(m, "TableMapIntDouble");
    py::bind_vector<std::vector<std::vector<std::map<std::vector<bool>, int>>>>(m, "TableMapVectorBoolInt");
     
    // Additional binding overloads for std::map<std::vector<size_t>, double>
    // so that operator[] getter/setter with a generic Python array will *just*
    // work.
    // Note: since we have an opaque type declaration for
    // std::vector<size_t>, Python could get confused and ask for an explicit
    // construction of `qb::VectorSize_t` type.
    py::bind_map<std::map<std::vector<size_t>, double>>(m, "MapVectorSize_tDouble")
        .def(py::init<>())
        .def("__repr__",
             [](const std::map<std::vector<size_t>, double> &self) {
               nlohmann::json jret = self;
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
        m, "MapStringMapVectorSize_tDouble")
        .def(py::init<>())
        .def("__repr__",
             [](const std::unordered_map<
                 std::string, std::map<std::vector<size_t>, double>> &self) {
               nlohmann::json jret = self;
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

  pybind11::array_t<int> std_vec_to_py_array(const std::vector<int> &input) {
    pybind11::array_t<int> vec(static_cast<int>(input.size()));
    auto r = vec.mutable_unchecked<1>();
    for (int i = 0; i < input.size(); ++i) {
      r(i) = input[i];
    }
    return vec;
  }

} // namespace qb
