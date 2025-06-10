#include <qristal/core/python/py_stl_containers.hpp>
#include <pybind11/stl_bind.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <ranges>

#include <iostream>

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

namespace qristal {

  void bind_opaque_containers(pybind11::module &m) {

    namespace py = pybind11;
    py::bind_vector<std::vector<std::string>>(m, "VectorString");
    py::bind_vector<std::vector<size_t>>(m, "VectorSize_t");
    py::bind_vector<std::vector<std::vector<size_t>>>(m, "TableSize_t");
    py::bind_vector<std::vector<std::vector<std::string>>>(m, "TableString");
    py::bind_vector<std::vector<std::vector<bool>>>(m, "TableBool");
    py::bind_map<std::map<int, double>>(m, "MapIntDouble");
    py::bind_map<std::map<int, std::complex<double>>>(m, "MapIntComplex");
    py::bind_vector<std::vector<std::map<int, double>>>(m, "VectorMapIntDouble");
    py::bind_vector<std::vector<std::map<int, std::complex<double>>>>(m, "VectorMapIntComplex");
    py::bind_vector<std::vector<std::vector<std::map<int, std::complex<double>>>>>(m, "TableMapIntComplex");
    py::bind_vector<std::vector<std::vector<std::map<int, double>>>>(m, "TableMapIntDouble");

    // Additional binding overloads.

    // These define print statements (__str__) and an operator[] (__getitem__) that works with a generic Python array.
    // The latter is needed because the key types are opaque, so they are not implicitly converted to Python array-like types.

    py::bind_map<std::map<std::vector<size_t>, double>>(m, "MapVectorSize_tDouble")
        .def(py::init<>())
        .def("__str__",
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
        .def("__str__",
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

    py::bind_vector<std::vector<bool>>(m, "VectorBool")
        .def(py::init<>())
        .def("__str__",
             // Printing places the highest-indexed qubit first.
             [](const std::vector<bool>& self) {
               std::ostringstream out;
               for (const auto& x: std::ranges::views::reverse(self)) out << std::to_string((int)x);
               return out.str();

             });

    py::class_<std::map<std::vector<bool>, int>>(m, "MapVectorBoolInt")
        .def(py::init<>())
        .def("__len__",
             [](const std::map<std::vector<bool>, int>& self) {
               return self.size();
             })
        .def("__iter__",
             [](std::map<std::vector<bool>, int>& self) {
               return py::make_key_iterator(self.begin(), self.end());
             }, py::keep_alive<0, 1>() /* Essential: keep view alive while iterator exists */
            )
        .def("__str__",
             // Printing places the highest-indexed qubit first.
             [](const std::map<std::vector<bool>, int>& self) {
               std::ostringstream out;
               for (const auto& [bits, count]: self) {
                 for (const auto& x: std::ranges::views::reverse(bits)) out << std::to_string((int)x);
                 out << ": " << count << std::endl;
               }
               std::string result = out.str();
               result.pop_back();
               return result;
             })
        .def("__getitem__",
             // Get if present, throw error if not
             [](std::map<std::vector<bool>, int>& self, const std::vector<bool>& key) {
               const auto iter = self.find(key);
               if (iter != self.end()) return iter->second;
               else throw std::out_of_range("Bitvector not found.");
             })
        .def("__getitem__",
             // Get if present, throw error if not
             [](std::map<std::vector<bool>, int>& self, const py::array_t<bool>& key) {
               const auto iter = self.find(py_array_to_std_vec(key));
               if (iter != self.end()) return iter->second;
               else throw std::out_of_range("Bitvector not found.");
             })
        .def("__setitem__",
             [](std::map<std::vector<bool>, int>& self, const std::vector<bool>& key, int& value) {
               return self[key] = value;
             })
        .def("__setitem__",
             [](std::map<std::vector<bool>, int>& self, const py::array_t<bool>& key, int& value) {
               return self[py_array_to_std_vec(key)] = value;
             })
        .def("__contains__",
             [](const std::map<std::vector<bool>, int>& self, const std::vector<bool>& key) {
               return self.find(key) != self.end();
             })
        .def("__contains__",
             [](const std::map<std::vector<bool>, int>& self, const py::array_t<bool>& key) {
               return self.find(py_array_to_std_vec(key)) != self.end();
             })
        .def("total_counts",
             [](const std::map<std::vector<bool>, int>& self) {
               return std::accumulate(self.begin(), self.end(), 0, [](auto sum, auto& e) { return sum+e.second; });
             });

    py::bind_vector<std::vector<std::map<std::vector<bool>, int>>>(m, "VectorMapVectorBoolInt");
    py::bind_vector<std::vector<std::vector<std::map<std::vector<bool>, int>>>>(m, "TableMapVectorBoolInt");

  }

  pybind11::array_t<int> std_vec_to_py_array(const std::vector<int> &input) {
    pybind11::array_t<int> vec(input.size());
    auto r = vec.mutable_unchecked<1>();
    for (int i = 0; i < input.size(); ++i) {
      r(i) = input[i];
    }
    return vec;
  }

}
