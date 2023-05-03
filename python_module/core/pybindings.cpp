// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include "qb/core/optimization/vqee/vqee.hpp"
#include "qb/core/optimization/qml/qml.hpp"
#include "qb/core/optimization/qaoa/qaoa.hpp"
#include "qb/core/optimization/qml/qml.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/remote_async_accelerator.hpp"
#include "qb/core/thread_pool.hpp"
#include "qb/core/circuit_builders/exponent.hpp"
#include "CompositeInstruction.hpp"
#include <nlohmann/json.hpp>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/iostream.h>
#include <algorithm>
#include <memory>

namespace py = pybind11;
using namespace qb;

// JSON conversion routines
namespace std {
void from_json(const json &js, std::complex<double> &elem) {
  if (js.is_array()) {
    throw std::invalid_argument(std::string("JSON: invalid complex element."));
  } else {
    std::complex<double> ret{js["r"].get<double>(), js["i"].get<double>()};
    std::cout << ret << std::endl;
    elem = ret;
  }
}

void to_json(json &j, const std::complex<double> &elem) {
  j = json{{"r", elem.real()}, {"i", elem.imag()}};
}

void from_json(const json &js, std::vector<std::complex<double>> &vec) {
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

namespace {
template <typename T> std::vector<T> py_array_to_std_vec(py::array_t<T> input) {
  py::buffer_info buf = input.request();
  if (buf.ndim != 1) {
    throw std::runtime_error("Number of dimensions must be one");
  }
  std::vector<T> result;
  result.reserve(buf.shape[0]);
  T *ptr = static_cast<T *>(buf.ptr);
  for (size_t idx = 0; idx < buf.shape[0]; ++idx) {
    result.emplace_back(ptr[idx]);
  }

  return result;
}

py::array_t<int> std_vec_to_py_array(const std::vector<int> &input) {
  py::array_t<int> vec(static_cast<int>(input.size()));
  auto r = vec.mutable_unchecked<1>();
  for (int i = 0; i < input.size(); ++i) {
    r(i) = input[i];
  }
  return vec;
}
} // namespace

using OracleFuncPyType = std::function<qb::CircuitBuilder(int)>;
using StatePrepFuncPyType = std::function<qb::CircuitBuilder(
    py::array_t<int>, py::array_t<int>, py::array_t<int>, py::array_t<int>,
    py::array_t<int>)>;

/**
 * Opaque STL in Python binding
 **/
PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<size_t>);
PYBIND11_MAKE_OPAQUE(std::vector<bool>);
PYBIND11_MAKE_OPAQUE(std::map<int, double>);
PYBIND11_MAKE_OPAQUE(std::map<int, std::complex<double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, std::complex<double>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<size_t>>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, double>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::string>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<bool>>);
PYBIND11_MAKE_OPAQUE(
    std::vector<std::vector<std::map<int, std::complex<double>>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<int, double>>>);

namespace qb {
/// Python interop job handle for async. execution.
/// Supports both true async. remote backends (e.g., AWS Braket) and
/// threading-based local backends (e.g., multiple instances of local
/// accelerators). (1) Remote backends (fully async.) will release the thread
/// (from threadpool) as soon as it finishes job submission. It returns a handle
/// to check for completion. (2) Local simulator/emulator instances will run on
/// different threads, i.e., the completion of thread execution indicates the
/// job completion.
class JobHandle : public std::enable_shared_from_this<JobHandle> {
private:
  /// Results from virtualized local simulator running on a dedicated thread.
  std::future<std::string> m_threadResult;
  /// Flag to indicate whether the execution thread is still running.
  /// For local simulators, this translates to the completion status of the job.
  bool m_thread_running = false;
  /// Row index to the job table
  int m_i;
  /// Column index to the job table
  int m_j;
  /// Name of the QPU that this job is assigned to.
  std::string m_qpuName;
  /// Non-owning reference to the session
  // !Important!: Within this JobHandle, only thread-safe methods of the session
  // class should be called.
  qb::session *m_qpqe;
  /// Instance of the QPU/Accelerator from the pool that this job is assigned
  /// to.
  std::shared_ptr<xacc::Accelerator> m_qpu;
  /// Async. job handle when the QPU is a remote Accelerator.
  /// Note: This will be null when the QPU is a local instance running on a
  /// dedicated thread.
  std::shared_ptr<async_job_handle> m_handle;
  /// Static map of all job handles.
  static inline std::map<std::pair<int, int>, std::shared_ptr<JobHandle>>
      JOB_HANDLE_REGISTRY;
  /// Mutex to guard access to the JOB_HANDLE_REGISTRY map.
  static inline std::mutex g_mutex;

public:
  /// Returns true if the job is completed.
  bool complete() const {
    if (m_handle) {
      // For remote accelerator (e.g. AWS Braket), use the handle to query the
      // job status.
      return m_handle->done();
    } else {
      // Otherwise, this job is running locally on a thread from the thread
      // pool. Returns the thread status.
      return !m_thread_running;
    }
  }

  std::string qpu_name() const { return m_qpuName; }

  /// Post the (i, j) job asynchronously to be executed on the virtualized QPU
  /// pool.
  void post_async(qb::session &s, int i, int j) {
    m_qpqe = &s;
    m_i = i;
    m_j = j;
    m_thread_running = true;
    // Add a functor to the thread pool to run the job.
    m_threadResult = thread_pool::submit(&JobHandle::run_async_internal, this);
    // Add this handle to the JOB_HANDLE_REGISTRY map.
    addJobHandle();
  }

  /// Retrieve the async execution result.
  /// Blocking if the job is not completed yet.
  std::string get_async_result() {
    if (m_handle) {
      // If this is a remote job, wait for its completion.
      m_handle->wait_for_completion();
      return m_qpqe->get_out_raws().at(m_i).at(m_j);
    } else {
      /// If this is a local simulation, wait for the simulation (on a thread)
      /// to complete.
      auto result = m_threadResult.get();
      return result;
    }
  }

  /// Terminate a job.
  void terminate() {
    if (complete()) {
      // Nothing to do if already completed.
      return;
    }

    if (m_handle) {
      // Cancel the remote job.
      // Note: a remote accelerator instance can have multiple jobs in-flight,
      // so the job cancellation must be associated with a job handle.
      m_handle->cancel();
    } else {
      // For local simulators, ask the accelerator to stop.
      if (m_qpu) {
        // Terminate the job if still running.
        m_qpu->cancel();
      }
    }

    // Remove the job handle from the list of in-flight jobs.
    removeJobHandle();
  }

  /// Retrieve the job handle for the (i,j) index.
  /// Return null if not found (e.g., not-yet posted or cancelled)
  static std::shared_ptr<JobHandle> getJobHandle(int i, int j) {
    const std::scoped_lock guard(g_mutex);
    auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(i, j));
    if (iter != JOB_HANDLE_REGISTRY.end()) {
      return iter->second;
    }
    return nullptr;
  }

private:
  /// Add this to the JOB_HANDLE_REGISTRY
  void addJobHandle() {
    const std::scoped_lock guard(g_mutex);
    JOB_HANDLE_REGISTRY[std::make_pair(m_i, m_j)] = shared_from_this();
  }

  /// Remove this from the JOB_HANDLE_REGISTRY
  void removeJobHandle() {
    const std::scoped_lock guard(g_mutex);
    auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(m_i, m_j));
    if (iter != JOB_HANDLE_REGISTRY.end()) {
      JOB_HANDLE_REGISTRY.erase(iter);
    }
  }

  /// Asynchronously run this job.
  // !IMPORTANT! This method will be called on a different thread (one from the
  // thread pool).
  std::string run_async_internal() {
    m_qpu = m_qpqe->get_executor().getNextAvailableQpu();
    auto async_handle = m_qpqe->run_async(m_i, m_j, m_qpu);
    m_qpuName = m_qpu->name();
    m_thread_running = false;
    m_qpqe->get_executor().release(std::move(m_qpu));
    /// If this is a remote accelerator, i.e., run_async returns a valid handle,
    /// cache it to m_handle.
    if (async_handle) {
      m_handle = async_handle;
      /// Returns a dummy result, not yet completed.
      return "";
    } else {
      /// If run_async executed synchronously on this thead, the result is
      /// available now.
      return m_qpqe->get_out_raws()[m_i][m_j];
    }
  };
};
} // namespace qb

/**
 * PYBIND11_MODULE(my-module-name, type_py::module_)
 **/
PYBIND11_MODULE(core, m) {
  m.doc() = "pybind11 for QB SDK";
  xacc::Initialize();
  xacc::setIsPyApi();

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

  py::class_<qb::JobHandle, std::shared_ptr<qb::JobHandle>>(m, "Handle")
      .def(py::init<>())
      .def("complete", &qb::JobHandle::complete,
           "Check if the job execution is complete.")
      .def("qpu_name", &qb::JobHandle::qpu_name,
           "Get the name of the QPU accelerator that executed this job.")
      .def("get", &qb::JobHandle::get_async_result, "Get the job result.")
      .def("terminate", &qb::JobHandle::terminate,
           "Terminate the running job.");

  py::class_<qb::session>(m, "session")
      .def(py::init<const std::string &>())
      .def(py::init<const bool>())
      .def(py::init())
      .def_property(
          "name_p", &qb::session::getName,
          py::overload_cast<const std::string &>(&qb::session::setName))
      .def_property(
          "names_p", &qb::session::getName,
          py::overload_cast<const VectorString &>(&qb::session::setName))
      .def_property("infile", &qb::session::get_infiles,
                    &qb::session::set_infile, qb::session::help_infiles_)
      .def_property("infiles", &qb::session::get_infiles,
                    &qb::session::set_infiles, qb::session::help_infiles_)
      .def_property("instring", &qb::session::get_instrings,
                    &qb::session::set_instring, qb::session::help_instrings_)
      .def_property("instrings", &qb::session::get_instrings,
                    &qb::session::set_instrings, qb::session::help_instrings_)
      .def_property(
          "ir_target",
          [&](qb::session &s) {
            std::vector<std::vector<qb::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qb::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qb::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qb::session &s, qb::CircuitBuilder &circuit) {
            s.set_irtarget_m(circuit.get());
          },
          qb::session::help_irtarget_ms_)
      .def_property(
          "ir_targets",
          [&](qb::session &s) {
            std::vector<std::vector<qb::CircuitBuilder>> circuits;
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                instructions = s.get_irtarget_ms();
            for (auto vec_instructions : instructions) {
              std::vector<qb::CircuitBuilder> vec;
              for (auto instruction : vec_instructions) {
                vec.push_back(qb::CircuitBuilder(instruction));
              }
              circuits.push_back(vec);
            }
            return circuits;
          },
          [&](qb::session &s,
              std::vector<std::vector<qb::CircuitBuilder>> &circuits) {
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                circuits_get;
            for (std::vector<qb::CircuitBuilder> vec : circuits) {
              std::vector<std::shared_ptr<xacc::CompositeInstruction>> vec_get;
              for (qb::CircuitBuilder circuit : vec) {
                std::shared_ptr<xacc::CompositeInstruction> circuit_get =
                    circuit.get();
                vec_get.push_back(circuit_get);
              }
              circuits_get.push_back(vec_get);
            }
            s.set_irtarget_ms(circuits_get);
          },
          qb::session::help_irtarget_ms_)
      .def_property("include_qb", &qb::session::get_include_qbs,
                    &qb::session::set_include_qb,
                    qb::session::help_include_qbs_)
      .def_property("include_qbs", &qb::session::get_include_qbs,
                    &qb::session::set_include_qbs,
                    qb::session::help_include_qbs_)
      .def_property("qpu_config", &qb::session::get_qpu_configs,
                    &qb::session::set_qpu_config,
                    qb::session::help_qpu_configs_)
      .def_property("qpu_configs", &qb::session::get_qpu_configs,
                    &qb::session::set_qpu_configs,
                    qb::session::help_qpu_configs_)
      .def_property("acc", &qb::session::get_accs, &qb::session::set_acc,
                    qb::session::help_accs_)
      .def_property("accs", &qb::session::get_accs, &qb::session::set_accs,
                    qb::session::help_accs_)
      .def_property("aws_verbatim", &qb::session::get_aws_verbatims,
                    &qb::session::set_aws_verbatim,
                    qb::session::help_aws_verbatims_)
      .def_property("aws_verbatims", &qb::session::get_aws_verbatims,
                    &qb::session::set_aws_verbatims,
                    qb::session::help_aws_verbatims_)
      .def_property("aws_format", &qb::session::get_aws_formats,
                    &qb::session::set_aws_format,
                    qb::session::help_aws_formats_)
      .def_property("aws_formats", &qb::session::get_aws_formats,
                    &qb::session::set_aws_formats,
                    qb::session::help_aws_formats_)
      .def_property("aws_device", &qb::session::get_aws_device_names,
                    &qb::session::set_aws_device_name,
                    qb::session::help_aws_device_names_)
      .def_property("aws_devices", &qb::session::get_aws_device_names,
                    &qb::session::set_aws_device_names,
                    qb::session::help_aws_device_names_)
      .def_property("aws_s3", &qb::session::get_aws_s3s,
                    &qb::session::set_aws_s3, qb::session::help_aws_s3s_)
      .def_property("aws_s3s", &qb::session::get_aws_s3s,
                    &qb::session::set_aws_s3s, qb::session::help_aws_s3s_)
      .def_property("aws_s3_path", &qb::session::get_aws_s3_paths,
                    &qb::session::set_aws_s3_path,
                    qb::session::help_aws_s3_paths_)
      .def_property("aws_s3_paths", &qb::session::get_aws_s3_paths,
                    &qb::session::set_aws_s3_paths,
                    qb::session::help_aws_s3_paths_)
      .def_property("aer_sim_type", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_type,
                    qb::session::help_aer_sim_types_)
      .def_property("aer_sim_types", &qb::session::get_aer_sim_types,
                    &qb::session::set_aer_sim_types,
                    qb::session::help_aer_sim_types_)
      .def_property("random", &qb::session::get_randoms,
                    &qb::session::set_random, qb::session::help_randoms_)
      .def_property("randoms", &qb::session::get_randoms,
                    &qb::session::set_randoms, qb::session::help_randoms_)
      .def_property("xasm", &qb::session::get_xasms, &qb::session::set_xasm,
                    qb::session::help_xasms_)
      .def_property("xasms", &qb::session::get_xasms, &qb::session::set_xasms,
                    qb::session::help_xasms_)
      .def_property("quil1", &qb::session::get_quil1s, &qb::session::set_quil1,
                    qb::session::help_quil1s_)
      .def_property("quil1s", &qb::session::get_quil1s,
                    &qb::session::set_quil1s, qb::session::help_quil1s_)
      .def_property("noplacement", &qb::session::get_noplacements,
                    &qb::session::set_noplacement,
                    qb::session::help_noplacements_)
      .def_property("noplacements", &qb::session::get_noplacements,
                    &qb::session::set_noplacements,
                    qb::session::help_noplacements_)
      .def_property("placement", &qb::session::get_placements,
                    &qb::session::set_placement, qb::session::help_placements_)
      .def_property("placements", &qb::session::get_placements,
                    &qb::session::set_placements, qb::session::help_placements_)
      .def_property("nooptimise", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimise,
                    qb::session::help_nooptimises_)
      .def_property("nooptimises", &qb::session::get_nooptimises,
                    &qb::session::set_nooptimises,
                    qb::session::help_nooptimises_)
      .def_property("nosim", &qb::session::get_nosims, &qb::session::set_nosim,
                    qb::session::help_nosims_)
      .def_property("nosims", &qb::session::get_nosims,
                    &qb::session::set_nosims, qb::session::help_nosims_)
      .def_property("noise", &qb::session::get_noises, &qb::session::set_noise,
                    qb::session::help_noises_)
      .def_property("noises", &qb::session::get_noises,
                    &qb::session::set_noises, qb::session::help_noises_)
      .def_property("noise_model", &qb::session::get_noise_models,
                    &qb::session::set_noise_model,
                    qb::session::help_noise_models_)
      .def_property("noise_models", &qb::session::get_noise_models,
                    &qb::session::set_noise_models,
                    qb::session::help_noise_models_)
      .def_property("noise_mitigation", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigation,
                    qb::session::help_noise_mitigations_)
      .def_property("noise_mitigations", &qb::session::get_noise_mitigations,
                    &qb::session::set_noise_mitigations,
                    qb::session::help_noise_mitigations_)
      .def_property("notiming", &qb::session::get_notimings,
                    &qb::session::set_notiming, qb::session::help_notimings_)
      .def_property("notimings", &qb::session::get_notimings,
                    &qb::session::set_notimings, qb::session::help_notimings_)
      .def_property("output_oqm_enabled", &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enabled,
                    qb::session::help_output_oqm_enableds_)
      .def_property("output_oqm_enableds",
                    &qb::session::get_output_oqm_enableds,
                    &qb::session::set_output_oqm_enableds,
                    qb::session::help_output_oqm_enableds_)
      .def_property("log_enabled", &qb::session::get_log_enableds,
                    &qb::session::set_log_enabled,
                    qb::session::help_log_enableds_)
      .def_property("log_enableds", &qb::session::get_log_enableds,
                    &qb::session::set_log_enableds,
                    qb::session::help_log_enableds_)
      .def_property("qn", &qb::session::get_qns, &qb::session::set_qn,
                    qb::session::help_qns_)
      .def_property("qns", &qb::session::get_qns, &qb::session::set_qns,
                    qb::session::help_qns_)
      .def_property("rn", &qb::session::get_rns, &qb::session::set_rn,
                    qb::session::help_rns_)
      .def_property("rns", &qb::session::get_rns, &qb::session::set_rns,
                    qb::session::help_rns_)
      .def_property("sn", &qb::session::get_sns, &qb::session::set_sn,
                    qb::session::help_sns_)
      .def_property("sns", &qb::session::get_sns, &qb::session::set_sns,
                    qb::session::help_sns_)
      .def_property("beta", &qb::session::get_betas, &qb::session::set_beta,
                    qb::session::help_betas_)
      .def_property("betas", &qb::session::get_betas, &qb::session::set_betas,
                    qb::session::help_betas_)
      .def_property("theta", &qb::session::get_thetas, &qb::session::set_theta,
                    qb::session::help_thetas_)
      .def_property("thetas", &qb::session::get_thetas,
                    &qb::session::set_thetas, qb::session::help_thetas_)
      .def_property("svd_cutoff", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoff,
                    qb::session::help_svd_cutoffs_)
      .def_property("svd_cutoffs", &qb::session::get_svd_cutoffs,
                    &qb::session::set_svd_cutoffs,
                    qb::session::help_svd_cutoffs_)
      .def_property("max_bond_dimension", &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimension,
                    qb::session::help_max_bond_dimensions_)
      .def_property("max_bond_dimensions",
                    &qb::session::get_max_bond_dimensions,
                    &qb::session::set_max_bond_dimensions,
                    qb::session::help_max_bond_dimensions_)
      .def_property("output_amplitude", &qb::session::get_output_amplitudes,
                    &qb::session::set_output_amplitude,
                    qb::session::help_output_amplitudes_)
      .def_property("output_amplitudes", &qb::session::get_output_amplitudes,
                    &qb::session::set_output_amplitudes,
                    qb::session::help_output_amplitudes_)
      .def_property_readonly("out_raw", &qb::session::get_out_raws,
                             qb::session::help_out_raws_)
      .def_property_readonly("out_raws", &qb::session::get_out_raws,
                             qb::session::help_out_raws_)
      .def_property_readonly("out_count", &qb::session::get_out_counts,
                             qb::session::help_out_counts_)
      .def_property_readonly("out_counts", &qb::session::get_out_counts,
                             qb::session::help_out_counts_)
      .def_property_readonly("out_divergence",
                             &qb::session::get_out_divergences,
                             qb::session::help_out_divergences_)
      .def_property_readonly("out_divergences",
                             &qb::session::get_out_divergences,
                             qb::session::help_out_divergences_)
      .def_property_readonly("out_transpiled_circuit",
                             &qb::session::get_out_transpiled_circuits,
                             qb::session::help_out_transpiled_circuits_)
      .def_property_readonly("out_transpiled_circuits",
                             &qb::session::get_out_transpiled_circuits,
                             qb::session::help_out_transpiled_circuits_)
      .def_property_readonly("out_qobj", &qb::session::get_out_qobjs,
                             qb::session::help_out_qobjs_)
      .def_property_readonly("out_qobjs", &qb::session::get_out_qobjs,
                             qb::session::help_out_qobjs_)
      .def_property_readonly("out_qbjson", &qb::session::get_out_qbjsons,
                             qb::session::help_out_qbjsons_)
      .def_property_readonly("out_qbjsons", &qb::session::get_out_qbjsons,
                             qb::session::help_out_qbjsons_)
      .def_property_readonly("out_single_qubit_gate_qty",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::session::help_out_single_qubit_gate_qtys_)
      .def_property_readonly("out_single_qubit_gate_qtys",
                             &qb::session::get_out_single_qubit_gate_qtys,
                             qb::session::help_out_single_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qty",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::session::help_out_double_qubit_gate_qtys_)
      .def_property_readonly("out_double_qubit_gate_qtys",
                             &qb::session::get_out_double_qubit_gate_qtys,
                             qb::session::help_out_double_qubit_gate_qtys_)

      .def_property_readonly(
          "out_total_init_maxgate_readout_time",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::session::help_out_total_init_maxgate_readout_times_)
      .def_property_readonly(
          "out_total_init_maxgate_readout_times",
          &qb::session::get_out_total_init_maxgate_readout_times,
          qb::session::help_out_total_init_maxgate_readout_times_)

      .def_property_readonly("out_z_op_expect",
                             &qb::session::get_out_z_op_expects,
                             qb::session::help_out_z_op_expects_)
      .def_property_readonly("out_z_op_expects",
                             &qb::session::get_out_z_op_expects,
                             qb::session::help_out_z_op_expects_)

      .def_property("debug", &qb::session::get_debug, &qb::session::set_debug,
                    qb::session::help_debug_)

      .def_property(
          "num_threads",
          [&](qb::session &s) { return qb::thread_pool::get_num_threads(); },
          [&](qb::session &s, int i) { qb::thread_pool::set_num_threads(i); },
          "num_threads: The number of threads in the QB SDK thread pool")

      .def_property("seed", &qb::session::get_seeds, &qb::session::set_seed,
                    qb::session::help_seeds_)
      .def_property("seeds", &qb::session::get_seeds, &qb::session::set_seeds,
                    qb::session::help_seeds_)

      .def("__repr__", &qb::session::get_summary,
           "Print summary of session settings")
      .def("run", py::overload_cast<>(&qb::session::run),
           "Execute all declared quantum circuits under all conditions")
      .def("runit",
           py::overload_cast<const size_t, const size_t >(&qb::session::run),
           "runit(i,j) : Execute circuit i, condition j")
      .def("divergence", py::overload_cast<>(&qb::session::get_jensen_shannon),
           "Calculate Jensen-Shannon divergence")
      .def("qb12", py::overload_cast<>(&qb::session::qb12),
           "Quantum Brilliance 12-qubit defaults")
      .def("aws32dm1", py::overload_cast<>(&qb::session::aws32dm1),
           "AWS Braket DM1, 32 async workers")
      .def("aws32sv1", py::overload_cast<>(&qb::session::aws32sv1),
           "AWS Braket SV1, 32 async workers")
      .def("aws8tn1", py::overload_cast<>(&qb::session::aws8tn1),
           "AWS Braket TN1, 8 async workers")
      .def("set_contrasts", py::overload_cast<const double &, const double &, const double &>(&qb::session::set_contrasts),
           "QB hardware contrast thresholds: init, qubit[0] final readout, qubit[1] final readout")
      .def("reset_contrasts", py::overload_cast<>(&qb::session::reset_contrasts),
           "QB hardware contrast thresholds reset")
      .def("set_parallel_run_config", &qb::session::set_parallel_run_config,
           "Set the parallel execution configuration")
      .def(
          "run_async",
          [&](qb::session &s, int i, int j) {
            auto handle = std::make_shared<qb::JobHandle>();
            // Allow accelerators to acquire the GIL for themselves from a
            // different thread
            pybind11::gil_scoped_release release;
            handle->post_async(s, i, j);
            return handle;
          },
          "run_async(i,j) : Launch the execution of circuit i, condition j "
          "asynchronously.")
      .def(
          "run_complete",
          [&](qb::session &s, int i, int j) {
            auto handle = qb::JobHandle::getJobHandle(i, j);
            if (handle) {
              return handle->complete();
            } else {
              return true;
            }
          },
          "run_complete(i,j) : Check if the execution of circuit i, condition "
          "j has been completed.");

  // Overloaded C++

  // m.def("add_", & session::add, "A function which adds 2 numbers",
  // py::arg("i") = 0, py::arg("j") = 0); m.def("cx_p", & session::tcx, "Test
  // complex doubles"); m.def("print_p", &
  // session::print_kv<int,std::complex<double>>, "Overloaded print method");
  // m.def("print_p", & session::print_vector_kv<int,std::complex<double>>,
  // "Overloaded print method"); m.def("print_vector_p", py::overload_cast<const
  // std::map<int,std::complex<double>>     &>(& session::print_kv), "Use C++ to
  // output container contents", py::arg("elems")); m.def("print_vector_p",
  // py::overload_cast<const std::vector<std::map<int,std::complex<double>>>
  // &>(& session::print_vector_kv), "Use C++ to output container contents",
  // py::arg("elems"));

  // std::vector<std::vector<double>> v_one;
  // v_one.push_back({2.3,3.5});
  // v_one.push_back({9.3});
  // v_one.push_back({0.1,0.6,0.1,0.1});
  // py::object myname = py::cast(v_one);
  // m.attr("name_") = myname;

  py::class_<qb::CircuitBuilder>(m, "Circuit")
      .def(py::init())
      .def("print", &qb::CircuitBuilder::print,
           "Print the quantum circuit that has been built")
      .def(
          "openqasm",
          [&](qb::CircuitBuilder &this_) {
            auto staq = xacc::getCompiler("staq");
            const auto openQASMSrc = staq->translate(this_.get());
            return openQASMSrc;
          },
          "Get the OpenQASM representation of the circuit.")
      .def("append",[&](qb::CircuitBuilder &this_, qb::CircuitBuilder other) {
          this_.append(other);
      }, py::arg("other"),
           "Append the 'other' quantum circuit to this circuit.")
      // Temporary interface to execute the circuit.
      // TODO: using s `run` once the QE-382 is implemented
      .def(
          "execute",
          [&](qb::CircuitBuilder &this_, const std::string &QPU, int NUM_SHOTS,
              int NUM_QUBITS) {
            auto acc = xacc::getAccelerator(QPU, {{"shots", NUM_SHOTS}});
            if (NUM_QUBITS < 0) {
              NUM_QUBITS = this_.get()->nPhysicalBits();
            }
            auto buffer = xacc::qalloc(NUM_QUBITS);
            acc->execute(buffer, this_.get());
            return buffer->toString();
          },
          py::arg("QPU") = "qpp", py::arg("NUM_SHOTS") = 1024,
          py::arg("NUM_QUBITS") = -1, R"(
              Run the circuit.

              This method is used to pass the circuit to an accelerator backend
              for execution.

              The optional parameters are:

              - **QPU** The accelerator name [string]
              - **NUM_SHOTS** The number of shots to use [int]
              - **NUM_QUBITS** The number of qubits required for the circuit [int]

          )")
      .def("h", [&](qb::CircuitBuilder &builder, int idx) {
          builder.H(idx);
      }, py::arg("idx"), R"(
    Hadamard gate
  
   This method adds a Hadamard (H) gate to the circuit. 
   
   Parameters:

   - **idx** the index of the qubit being acted on [int]

  )")
        .def("x", [&](qb::CircuitBuilder &builder, int idx) {
          builder.X(idx);
      }, py::arg("idx"), R"(
      Pauli-X gate

     This method adds a Pauli-X (X) gate to the circuit.

     The X gate is defined by its action on the basis states

    Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("y", [&](qb::CircuitBuilder &builder, int idx) {
          builder.Y(idx);
      }, py::arg("idx"), R"(
      Pauli-Y gate

     This method adds a Pauli-Y (Y) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("z", [&](qb::CircuitBuilder &builder, int idx) {
          builder.Z(idx);
      }, py::arg("idx"), R"(
      Pauli-Z gate

     This method adds a Pauli-Z (Z) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("t", [&](qb::CircuitBuilder &builder, int idx) {
          builder.T(idx);
      }, py::arg("idx"), R"(
      T gate

     This method adds a T gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("tdg", [&](qb::CircuitBuilder &builder, int idx) {
          builder.Tdg(idx);
      }, py::arg("idx"), R"(
      Tdg gate

     This method adds an inverse of the T gate (Tdg) to the circuit.

     The Tdg gate is defined by its action on the basis states

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("s", [&](qb::CircuitBuilder &builder, int idx) {
          builder.S(idx);
      }, py::arg("idx"), R"(
      S gate

     This method adds an S gate to the circuit.

     The S gate is defined by its action on the basis states

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("sdg", [&](qb::CircuitBuilder &builder, int idx) {
          builder.Sdg(idx);
      }, py::arg("idx"), R"(
      Sdg gate

     This method adds an inverse of the S gate (Sdg) to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
        .def("rx", [&](qb::CircuitBuilder &builder, int idx, double theta) {
          builder.RX(idx, theta);
      }, py::arg("idx"), py::arg("theta"), R"(
      RX gate

     This method adds an x-axis rotation (RX) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the x-axis [double]

    )")
        .def("ry", [&](qb::CircuitBuilder &builder, int idx, double theta) {
          builder.RY(idx, theta);
      }, py::arg("idx"), py::arg("theta"), R"(
      RY gate

     This method adds a y-axis rotation (RY) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the y-axis [double]

    )")
        .def("rz", [&](qb::CircuitBuilder &builder, int idx, double theta) {
          builder.RZ(idx, theta);
      }, py::arg("idx"), py::arg("theta"), R"(
      RZ gate

     This method adds a z-axis rotation (RZ) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the z-axis [double]

    )")
        .def("cnot", [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
          builder.CNOT(ctrl_idx, target_idx);
      }, py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CNOT gate

     This method adds a controlled-X (CNOT) gate to the circuit.

     The CNOT gate performs an X gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
        .def(
            "mcx",
            [&](qb::CircuitBuilder &builder, py::array_t<int> ctrl_inds,
                int target_idx) {
              builder.MCX(py_array_to_std_vec(ctrl_inds), target_idx);
            }, py::arg("ctrl_inds"), py::arg("target_idx"),
            R"(
      MCX gate

     This method adds a multi-controlled X (MCX) gate to the circuit.

     The MCX gate performs an X gate on the target qubit
     conditional on all control qubits being in the 1 state.

     Parameters:

     - **ctrl_inds** the indices of the control qubits [list of int]
     - **target_idx** the index of the target qubit [int]

    )")
        .def(
            "ccx",
            [&](qb::CircuitBuilder &builder, int ctrl_idx1, int ctrl_idx2,
                int target_idx) {
              builder.MCX({ctrl_idx1, ctrl_idx2}, target_idx);
            }, py::arg("ctrl_idx1"), py::arg("ctrl_idx2"), py::arg("target_idx"),
            R"(
      Toffoli gate

     This method adds a Toffoli gate (CCX) to the circuit.

     The CCX gate performs an X gate on the target qubit
     conditional on the two control qubits being in the 1 state.

     Parameters:

     - **ctrl_idx1** the index of the first control qubit [int]
     - **ctrl_idx2** the index of the second control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
        .def("swap", [&](qb::CircuitBuilder &builder, int q1, int q2) {
            builder.SWAP(q1, q2);
        }, py::arg("q1"), py::arg("q2"), R"(
      SWAP gate

     This method adds a SWAP gate to the circuit. The SWAP gate is used to swap the quantum state of two qubits.

     Parameters:

     - **q1** the index of the first qubit [int]
     - **q2** the index of the second qubit [int]

    )")
        .def("cphase", [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx, double theta) {
            builder.CPhase(ctrl_idx, target_idx, theta);
        }, py::arg("ctrl_idx"), py::arg("target_idx"), py::arg("theta"),
             R"(
      CPhase gate

     This method adds a controlled-U1 (CPhase) gate to the circuit.

     The CPHase gate performs a U1(theta) gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]
     - **theta** the value of the phase [double]

    )")
        .def("cz", [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
            builder.CZ(ctrl_idx, target_idx);
        }, py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CZ gate

     This method adds a controlled-Z (CZ) gate to the circuit.

     The CZ gate performs a Z gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
        .def("ch", [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
            builder.CH(ctrl_idx, target_idx);
        }, py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CH gate

     This method adds a controlled-H (CH) gate to the circuit.

     The CH gate performs an H gate on the target qubit
     conditional on the control qubit being in the 1 state.

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
        .def("u1", [&](qb::CircuitBuilder &builder, int idx, double theta) {
            builder.U1(idx, theta);
        }, py::arg("idx"), py::arg("theta"), R"(
      U1 gate

     This method adds a phase (U1) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the value of the phase [double]

    )")
        .def("u3", [&](qb::CircuitBuilder &builder, int idx, double theta, double phi, double lambda) {
            builder.U3(idx, theta, phi, lambda);
        }, py::arg("idx"), py::arg("theta"), py::arg("phi"), py::arg("lambda"), R"(
      U3 gate

     This method adds an arbitrary single qubit gate (U3) to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** [double]
     - **phi** [double]
     - **lambda** [double]

    )")
        .def("measure", [&](qb::CircuitBuilder &builder, int idx) {
            builder.Measure(idx);
        }, py::arg("idx"), R"(
      Measurement

     This method is used to indicate a qubit in the circuit should be
     measured.

     Parameters:

     - **idx** the index of the qubit to be measured [int]

    )")
        .def("measure_all", [&](qb::CircuitBuilder &builder, int NUM_QUBITS) {
            builder.MeasureAll(NUM_QUBITS);
        },
              py::arg("NUM_QUBITS") = -1,
             R"(
      Measure all qubits

     This method adds a measurement for all qubits involved in the circuit.

     Parameters:

     - **NUM_QUBITS** the number of qubits in the circuit [int] [optional, the default value of -1 becomes the output of the XACC nPhysicalBits method.]

    )")
        .def(
            "qft",
            [&](qb::CircuitBuilder &builder, py::array_t<int> inds) {
              builder.QFT(py_array_to_std_vec(inds));
            },
            py::arg("qubits"), R"(
      Quantum Fourier Transform

     This method adds the Quantum Fourier Transform (QFT) to the circuit.
     This is a quantum analogue of the discrete Fourier Transform.

     Parameters:

     - **qubits** the indices of the target qubits [list of int]

    )")
        .def(
            "iqft",
            [&](qb::CircuitBuilder &builder, py::array_t<int> inds) {
              builder.IQFT(py_array_to_std_vec(inds));
            },
            py::arg("qubits"), R"(
      Inverse Quantum Fourier Transform

     This method adds the inverse of the Quantum Fourier Transform (IQFT) to
     the circuit.

     Parameters:

     - **qubits** the indices of the target qubits [list of int]

    )")
        .def(
            "exponent",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_log,
                py::array_t<int> qubits_exponent, py::array_t<int>
                qubits_ancilla, int min_significance, bool is_LSB) {
              qb::Exponent build_exp;
              xacc::HeterogeneousMap map =
              {{"qubits_log",py_array_to_std_vec(qubits_log)},
              {"min_significance", min_significance}, {"is_LSB", is_LSB}}; if
              (qubits_exponent.size() > 0) {
                  map.insert("qubits_exponent", qubits_exponent);
              }
              if (qubits_ancilla.size() > 0) {
                  map.insert("qubits_ancilla", qubits_ancilla);
              }
              bool expand_ok = build_exp.expand(map);
              builder.append(build_exp);
              return expand_ok;
            },
            py::arg("qubits_log") = py::array_t<int>(),
            py::arg("qubits_exponent") = py::array_t<int>(),
            py::arg("qubits_ancilla") = py::array_t<int>(),
            py::arg("min_significance") = 1, py::arg("is_LSB") = true,
            R"(
                Exponent Base 2

                This method adds an exponent to the circuit. This is used to replace some value
                by its exponent base 2. 

                Parameters:

                - **qubits_log** the indices of the qubits encoding the original value [list of int]
                - **qubits_exponent** the indices of the qubits used to store the result [list of int]
                - **qubits_ancilla** the indices of the required ancilla qubits [list of int]
                - **min_significance** the accuracy cutoff [int]
                - **is_LSB** indicates LSB ordering is used [bool]

            )")
        .def(
            "qpe",
            [&](qb::CircuitBuilder &builder, py::object &oracle, int
            precision,
                py::array_t<int> trial_qubits,
                py::array_t<int> evaluation_qubits) {
              qb::CircuitBuilder *casted =
                  oracle.cast<qb::CircuitBuilder *>();
              assert(casted);
              builder.QPE(*casted, precision,
              py_array_to_std_vec(trial_qubits),
                          py_array_to_std_vec(evaluation_qubits));
            },
            py::arg("oracle"), py::arg("precision"),
            py::arg("trial_qubits") = py::array_t<int>(),
            py::arg("precision_qubits") = py::array_t<int>(),
            R"(
      Quantum Phase Estimation

     This method adds the Quantum Phase Estimation (QPE) sub-routine to the
     circuit.

     Given some unitary operator U and and eigenvector v of U, QPE is used to provide a k-bit approximation to
     the corresponding eigenvalue's phase, storing the result in an evaluation register whilst leaving the eigenvector
     unchanged.

     Parameters:

     - **oracle** The unitary operator U involved in the QPE routine [CircuitBuilder]
     - **precision** The number of bits k used to approximate the phase [int]
     - **trial_qubits** The indices of the qubits encoding the eigenvector of the unitary [list of int]
     - **precision_qubits** The indices of the qubits that will be used to store the approximate phase [list of int]

    )")
        .def(
            "canonical_ae",
            [&](qb::CircuitBuilder &builder, py::object &state_prep,
                py::object &grover_op, int precision, int
                num_state_prep_qubits, int num_trial_qubits, py::array_t<int>
                precision_qubits, py::array_t<int> trial_qubits, bool
                no_state_prep) {
              qb::CircuitBuilder *casted_state_prep =
                  state_prep.cast<qb::CircuitBuilder *>();
              assert(state_prep);

              qb::CircuitBuilder *casted_grover_op =
                  grover_op.cast<qb::CircuitBuilder *>();
              assert(casted_grover_op);
              builder.CanonicalAmplitudeEstimation(
                  *casted_state_prep, *casted_grover_op, precision,
                  num_state_prep_qubits, num_trial_qubits,
                  py_array_to_std_vec(precision_qubits),
                  py_array_to_std_vec(trial_qubits), no_state_prep);
            },
            py::arg("state_prep"), py::arg("grover_op"),
            py::arg("precision"), py::arg("num_state_prep_qubits"),
            py::arg("num_trial_qubits"), py::arg("precision_qubits") =
            py::array_t<int>(), py::arg("trial_qubits") = py::array_t<int>(),
            py::arg("no_state_prep") = false, R"(
      Canonical Amplitude Estimation

     This method adds the canonical version of Quantum Amplitude Estimation
     (QAE) to the circuit.

     Given a quantum state split into a good subspace and a bad subspace, 
     the QAE sub-routine provides a k-bit approximation to the amplitude of
     the good subspace, a.

     QAE works by using the Grovers operator Q, which amplifies the amplitude
     of the good subspace, as the unitary input to a Quantum Phase Estimation
     routine.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **grover_op** The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **no_state_prep** If true, assumes the state is already prepared in the appropriate register [bool]

    )")
        .def(
            "run_canonical_ae",
            [&](qb::CircuitBuilder &builder, py::object &state_prep,
                py::object &grover_op, int precision, int
                num_state_prep_qubits, int num_trial_qubits, py::array_t<int>
                precision_qubits, py::array_t<int> trial_qubits, py::str
                acc_name) {
              qb::CircuitBuilder *casted_state_prep =
                  state_prep.cast<qb::CircuitBuilder *>();
              assert(state_prep);

              qb::CircuitBuilder *casted_grover_op =
                  grover_op.cast<qb::CircuitBuilder *>();
              assert(casted_grover_op);
              return builder.RunCanonicalAmplitudeEstimation(
                  *casted_state_prep, *casted_grover_op, precision,
                  num_state_prep_qubits, num_trial_qubits,
                  py_array_to_std_vec(precision_qubits),
                  py_array_to_std_vec(trial_qubits), acc_name);
            },
            py::arg("state_prep"), py::arg("grover_op"),
            py::arg("precision"), py::arg("num_state_prep_qubits"),
            py::arg("num_trial_qubits"), py::arg("precision_qubits") =
            py::array_t<int>(), py::arg("trial_qubits") = py::array_t<int>(),
            py::arg("qpu") = "qpp", R"(
      Run Canonical Amplitude Estimation

     This method sets up and executes an instance of the canonical amplitude
     estimation circuit.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **grover_op** The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
        .def(
            "amcu",
            [&](qb::CircuitBuilder &builder, py::object &U,
                py::array_t<int> qubits_control,
                py::array_t<int> qubits_ancilla) {
              qb::CircuitBuilder *casted_U =
                  U.cast<qb::CircuitBuilder *>();

              return builder.MultiControlledUWithAncilla(
                  *casted_U,
                  py_array_to_std_vec(qubits_control),
                  py_array_to_std_vec(qubits_ancilla));
            },
            py::arg("U"), py::arg("qubits_control"),
            py::arg("qubits_ancilla"),
                R"(
      Multi Controlled Unitary With Ancilla

     This method decomposes a multi-controlled unitary into Toffoli gates
     and the unitary itself, with the use of ancilla qubits. With N control
     qubits there should be N-1 ancilla. The resulting instructions are added
     to the circuit (AMCU gate).

     Parameters:

     - **U** The unitary operation [CircuitBuilder]
     - **qubits_control** The indices of the control qubits [list of int]
     - **qubits_ancilla** The indices of the ancilla qubits [list of int]

    )")
        .def(
            "run_canonical_ae_with_oracle",
            [&](qb::CircuitBuilder &builder, py::object &state_prep,
                py::object &oracle, int precision, int num_state_prep_qubits,
                int num_trial_qubits, py::array_t<int> precision_qubits,
                py::array_t<int> trial_qubits, py::str acc_name) {
              qb::CircuitBuilder *casted_state_prep =
                  state_prep.cast<qb::CircuitBuilder *>();
              assert(state_prep);

              qb::CircuitBuilder *casted_oracle =
                  oracle.cast<qb::CircuitBuilder *>();
              assert(casted_oracle);
              return builder.RunCanonicalAmplitudeEstimationWithOracle(
                  *casted_state_prep, *casted_oracle, precision,
                  num_state_prep_qubits, num_trial_qubits,
                  py_array_to_std_vec(precision_qubits),
                  py_array_to_std_vec(trial_qubits), acc_name);
            },
            py::arg("state_prep"), py::arg("oracle"), py::arg("precision"),
            py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
            py::arg("precision_qubits") = py::array_t<int>(),
            py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") =
            "qpp", R"(
      Run Canonical Amplitude Estimation with Oracle

     This method sets up and executes an instance of the canonical amplitude
     estimation circuit, but instead of providing the grovers_op Q, we
     provide the oracle circuit O which marks the good elements.

     The Grovvers operator Q is then constructed within the method from O and
     the state_prep circuit A.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
        .def(
            "run_MLQAE",
            [&](qb::CircuitBuilder &builder, py::object &state_prep,
                py::object &oracle,
                std::function<int(std::string, int)> is_in_good_subspace,
                py::array_t<int> score_qubits, int total_num_qubits, int
                num_runs, int shots, py::str acc_name) {
              qb::CircuitBuilder *casted_state_prep =
                  state_prep.cast<qb::CircuitBuilder *>();
              assert(state_prep);

              qb::CircuitBuilder *casted_oracle =
                  oracle.cast<qb::CircuitBuilder *>();
              assert(casted_oracle);
              return builder.RunMLAmplitudeEstimation(
                  *casted_state_prep, *casted_oracle, is_in_good_subspace,
                  py_array_to_std_vec(score_qubits), total_num_qubits,
                  num_runs, shots, acc_name);
            },
            py::arg("state_prep"), py::arg("oracle"),
            py::arg("is_in_good_subspace"), py::arg("score_qubits"),
            py::arg("total_num_qubits"), py::arg("num_runs") = 4,
            py::arg("shots") = 100, py::arg("qpu") = "qpp",
            R"(
      Run Maximum-Likelihood Amplitude Estimation

     This method sets up and executes an instance of the maximum-likelihood
     amplitude estimation circuit.

     Given a state split into a good subspace and a bad subspace, MLQAE is an alternative to canonical QAE to find an estimate for the
     amplitude of the good subspace, a. It works by performing several runs
     of amplitude amplification with various iterations and recording the
     number of good shots measured. Given this data, it finds the value of
     a that maximises the likelihood function.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **is_in_good_subspace** A function that, given a measured bitstring and potentially some other input value, returns a 1 if the measurement is in the good subspace and a 0 otherwise. [func(str, int) -> int]
     - **score_qubits** The indices of the qubits that determine whether the state is in the good or bad subspace [list of int]
     - **total_num_qubits** The total number of qubits in the circuit [int]
     - **num_runs** The number of runs of amplitude amplification (~4-6 is usually sufficient)
     - **shots** The number of shots in each run [int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
        .def(
            "amplitude_amplification",
            [&](qb::CircuitBuilder &builder, py::object &oracle,
                py::object &state_prep, int power) {
              qb::CircuitBuilder *oracle_casted =
                  oracle.cast<qb::CircuitBuilder *>();
              assert(oracle_casted);
              qb::CircuitBuilder *state_prep_casted =
                  state_prep.cast<qb::CircuitBuilder *>();
              assert(state_prep_casted);
              builder.AmplitudeAmplification(*oracle_casted,
              *state_prep_casted,
                                             power);
            },
            py::arg("oracle"), py::arg("state_prep"), py::arg("power") = 1,
            R"(
      Amplitude Amplification

     This method adds a number of Grovers operators to the circuit.

     Grovers operators are used to amplify the amplitude of some desired
     subspace of your quantum state. 

     Parameters:

     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **power** The number of Grovers operators to append to the circuit [int]

     )")
        .def(
            "ripple_add",
            [&](qb::CircuitBuilder &builder, py::array_t<int> a,
                py::array_t<int> b, int c_in) {
              builder.RippleAdd(py_array_to_std_vec(a),
              py_array_to_std_vec(b),
                                c_in);
            },
            py::arg("a"), py::arg("b"), py::arg("carry_bit"),R"(
      Ripple Carry Adder

     This method adds a ripple carry adder to the circuit.

     The ripple carry adder is an efficient in-line addition operation
     with a carry-in bit. 

     Parameters:

     - **a** The qubit indices of the first register in the addition [list of int]
     - **b** The qubit indices of the second register in the addition. This is where the result of a+b will be stored [list of int]
     - **carry_bit** The index of the carry-in bit [int]

    )")
        .def(
            "comparator",
            [&](qb::CircuitBuilder &builder, int BestScore,
                int num_scoring_qubits, py::array_t<int> trial_score_qubits,
                int flag_qubit, py::array_t<int> best_score_qubits,
                py::array_t<int> ancilla_qubits, bool is_LSB,
                py::array_t<int> controls_on, py::array_t<int> controls_off)
                {
              builder.Comparator(BestScore, num_scoring_qubits,
                                 py_array_to_std_vec(trial_score_qubits),
                                 flag_qubit,
                                 py_array_to_std_vec(best_score_qubits),
                                 py_array_to_std_vec(ancilla_qubits), is_LSB,
                                 py_array_to_std_vec(controls_on),
                                 py_array_to_std_vec(controls_off));
            },
            py::arg("best_score"), py::arg("num_scoring_qubits"),
            py::arg("trial_score_qubits") = py::array_t<int>(),
            py::arg("flag_qubit") = -1,
            py::arg("best_score_qubits") = py::array_t<int>(),
            py::arg("ancilla_qubits") = py::array_t<int>(),
            py::arg("is_LSB") = true, py::arg("controls_on") =
            py::array_t<int>(), py::arg("controls_off") = py::array_t<int>(),
            R"(
      Comparator

     This method adds a quantum bit string comparator to the circuit.

     The quantum bit string comparator is used to compare the values of two
     bit string. If the trial score is greater than the best score, the flag
     qubit is flipped.

     Parameters:

     - **best_score** The score we are comparing strings to [int]
     - **num_scoring_qubits** The number of qubits used to encode the scores [int]
     - **trial_score_qubits** The indices of the qubits encoding the trial states [list of int]
     - **flag_qubit** The index of the flag qubit which is flipped whenever trial score > BestScore [int]
     - **best_score_qubits** The indices of the qubits encoding the BestScore value [list of int]
     - **ancilla_qubits** The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
        .def(
            "efficient_encoding",
            [&](qb::CircuitBuilder &builder,
                std::function<int(int)> scoring_function, int
                num_state_qubits, int num_scoring_qubits, py::array_t<int>
                state_qubits, py::array_t<int> scoring_qubits, bool is_LSB,
                bool use_ancilla, py::array_t<int> qubits_init_flags, int
                flag_integer) {
              builder.EfficientEncoding(
                  scoring_function, num_state_qubits, num_scoring_qubits,
                  py_array_to_std_vec(state_qubits),
                  py_array_to_std_vec(scoring_qubits), is_LSB, use_ancilla,
                  py_array_to_std_vec(qubits_init_flags), flag_integer);
            },
            py::arg("scoring_function"), py::arg("num_state_qubits"),
            py::arg("num_scoring_qubits"),
            py::arg("state_qubits") = py::array_t<int>(),
            py::arg("scoring_qubits") = py::array_t<int>(),
            py::arg("is_LSB") = true, py::arg("use_ancilla") = false,
            py::arg("qubits_init_flags") = py::array_t<int>(),
            py::arg("flag_integer") = 0, R"(
      Efficient Encoding

     This method adds an efficient encoding routine to the circuit.

     Given a lookup function f that assigns a score to each binary string,
     we entangle each string to its score. Rather than
     encoding states sequentially we cut down on the
     amount of X gates required by instead following the Gray code ordering
     of states.

     This module can optionally also flag strings of a certain value.

     Parameters:

     - **scoring_function** A function that inputs the integer value of a binary string and outputs its score [func(int) -> int]
     - **num_state_qubits** The number of qubits encoding the strings [int]
     - **num_scoring_qubits** The number of qubits encoding the scores [int]
     - **state_qubits** The indices of the qubits encoding the strings [list of int]
     - **scoring_qubits** The indices of the qubits encoding the scores [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **use_ancilla** Indicates that ancilla qubits can be used to decompose MCX gates [bool]
     - **qubits_init_flag** The indices of any flag qubits [list of int]
     - **flag_integer** The integer value of binary strings that should be flagged [int]

    )")
        .def(
            "equality_checker",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
                py::array_t<int> qubits_b, int flag, bool use_ancilla,
                py::array_t<int> qubits_ancilla, py::array_t<int>
                controls_on, py::array_t<int> controls_off) {
              builder.EqualityChecker(
                  py_array_to_std_vec(qubits_a),
                  py_array_to_std_vec(qubits_b), flag, use_ancilla,
                  py_array_to_std_vec(qubits_ancilla),
                  py_array_to_std_vec(controls_on),
                  py_array_to_std_vec(controls_off));
            },
            py::arg("qubits_a"), py::arg("qubits_b"), py::arg("flag"),
            py::arg("use_ancilla") = false,
            py::arg("qubits_ancilla") = py::array_t<int>(),
            py::arg("controls_on") = py::array_t<int>(),
            py::arg("controls_off") = py::array_t<int>(),
            R"(
      Equality Checker

     This method adds an equality checker to the circuit.

     Given two input bitstrings a and b the equality checker is
     used to flip a flag qubit whenever a=b.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **flag** the index of the flag qubit that gets flipped whenever a=b [int]
     - **use_ancilla** Indicates that ancilla qubits can be used to decompose MCX gates [bool]
     - **qubits_ancilla** The indices of the qubits to be used as ancilla qubits if use_ancilla=true [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
        .def(
            "controlled_swap",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
                py::array_t<int> qubits_b, py::array_t<int> flags_on,
                py::array_t<int> flags_off) {
              builder.ControlledSwap(
                  py_array_to_std_vec(qubits_a),
                  py_array_to_std_vec(qubits_b),
                  py_array_to_std_vec(flags_on),
                  py_array_to_std_vec(flags_off));
            },
            py::arg("qubits_a"), py::arg("qubits_b"),
            py::arg("flags_on") = py::array_t<int>(),
            py::arg("flags_off") = py::array_t<int>(),
            R"(
      Controlled SWAP

     This method adds a controlled SWAP to the circuit.

     Performs a SWAP operation on a and b if an only if the controls are
     satisfied.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **flags_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **flags_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
        .def(
            "controlled_ripple_carry_adder",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_adder,
                py::array_t<int> qubits_sum, int c_in, py::array_t<int>
                flags_on, py::array_t<int> flags_off, bool no_overflow) {
              builder.ControlledAddition(
                  py_array_to_std_vec(qubits_adder),
                  py_array_to_std_vec(qubits_sum), c_in,
                  py_array_to_std_vec(flags_on),
                  py_array_to_std_vec(flags_off), no_overflow);
            },
            py::arg("qubits_adder"), py::arg("qubits_sum"), py::arg("c_in"),
            py::arg("flags_on") = py::array_t<int>(),
            py::arg("flags_off") = py::array_t<int>(), py::arg("no_overflow")
            = false, R"(
      Controlled Addition

     This method adds a controlled ripple carry adder to the circuit.

     Performs a RippleAdd operation on adder_bits and sum_bits if and only
     if the controls are satisfied.

     Parameters:

     - **qubits_adder** the indices of the qubits encoding adder_bits [list of int]
     - **qubits_sum** the indices of the qubits encoding sum_bits [list of int]
     - **c_in** the index of the carry-in bit [int]
     - **flags_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **flags_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **no_overflow** Indicates that the total of the addition can be encoded on the same number of qubits as sum_bits without overflowing [bool]

    )")
        .def(
            "generalised_mcx",
            [&](qb::CircuitBuilder &builder, int target,
                py::array_t<int> controls_on, py::array_t<int> controls_off)
                {
              builder.GeneralisedMCX(target,
                  py_array_to_std_vec(controls_on),
                  py_array_to_std_vec(controls_off));
            },
            py::arg("target"), py::arg("controls_on") = py::array_t<int>(),
            py::arg("controls_off") = py::array_t<int>(),
            R"(
    Generalised MCX

   This method adds a generalised MCX gate to the circuit.

   By generalised MCX we mean that we allow the control qubits to be
   conditional on being off or conditional on being on.

   Parameters:

   - **target** The index of the target qubit [int]
   - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
   - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

  )")
        .def(
            "compare_beam_oracle",
            [&](qb::CircuitBuilder &builder, int q0, int q1, int q2,
                py::array_t<int> FA, py::array_t<int> FB, py::array_t<int>
                SA, py::array_t<int> SB, bool simplified) {
              builder.CompareBeamOracle(q0, q1, q2,
                  py_array_to_std_vec(FA),
                  py_array_to_std_vec(FB),
                  py_array_to_std_vec(SA),
                  py_array_to_std_vec(SB), simplified);
            },
            py::arg("q0"), py::arg("q1"), py::arg("q2"),
            py::arg("FA"),
            py::arg("FB"),
            py::arg("SA"),
            py::arg("SB") = py::array_t<int>(),
            py::arg("simplified") = true,
            R"(
      Compare Beam Oracle

     This method adds a compare beam oracle to the circuit.

     This method is required for the quantum decoder algorithm.
    )")
        .def(
            "inverse_circuit",
            [&](qb::CircuitBuilder &builder, py::object &circ) {
              qb::CircuitBuilder *casted_circ =
                  circ.cast<qb::CircuitBuilder *>();
              builder.InverseCircuit(*casted_circ);
            },
            py::arg("circ"),
            R"(
    Inverse Circuit

   This method adds the inverse of a circuit to the current circuit.

   Given some collection of unitary operations,

   U = U_NU_{N-1}...U_2U_1

   this method appends the inverse to the circuit:

   U^{-1} = U_1dg U_2dg...U_{N-1}dg U_Ndg

   This may be useful for un-computing ancilla or for constructing Grovers
   operators.

   Parameters:

   - **circ** The circuit whose inverse we want to add to the current circuit [CircuitBuilder]

  )")
        .def(
            "comparator_as_oracle",
            [&](qb::CircuitBuilder &builder, int BestScore,
                int num_scoring_qubits, py::array_t<int> trial_score_qubits,
                int flag_qubit, py::array_t<int> best_score_qubits,
                py::array_t<int> ancilla_qubits, bool is_LSB,
                py::array_t<int> controls_on, py::array_t<int> controls_off)
                {
              builder.Comparator_as_Oracle(
                  BestScore, num_scoring_qubits,
                  py_array_to_std_vec(trial_score_qubits), flag_qubit,
                  py_array_to_std_vec(best_score_qubits),
                  py_array_to_std_vec(ancilla_qubits), is_LSB,
                  py_array_to_std_vec(controls_on),
                  py_array_to_std_vec(controls_off));
            },
            py::arg("best_score"), py::arg("num_scoring_qubits"),
            py::arg("trial_score_qubits") = py::array_t<int>(),
            py::arg("flag_qubit") = -1,
            py::arg("best_score_qubits") = py::array_t<int>(),
            py::arg("ancilla_qubits") = py::array_t<int>(),
            py::arg("is_LSB") = true, py::arg("controls_on") =
            py::array_t<int>(), py::arg("controls_off") = py::array_t<int>(),
            R"(
      Comparator as Oracle

     This method adds a quantum bit string comparator oracle to the circuit.

     The quantum bit string comparator is used to add a negative phase to any
     trial state whose bit string value is greater than the state being
     compared to. In this way it can be used as an oracle in a Grovers
     operator that amplifies higher scoring strings. This may be useful in
     many search problems.

     Parameters:

     - **best_score** The score we are comparing strings to [int]
     - **num_scoring_qubits** The number of qubits used to encode the scores [int]
     - **trial_score_qubits** The indices of the qubits encoding the trial states [list of int]
     - **flag_qubit** The index of the flag qubit which acquires a negative phase whenever trial score > BestScore [int]
     - **best_score_qubits** The indices of the qubits encoding the BestScore value [list of int]
     - **ancilla_qubits** The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
        .def(
            "multiplication",
            [&](qb::CircuitBuilder &builder,
                py::array_t<int> qubits_a, py::array_t<int> qubits_b,
                py::array_t<int> qubits_result, int qubit_ancilla, bool
                is_LSB) {
              builder.Multiplication(
                  py_array_to_std_vec(qubits_a),
                  py_array_to_std_vec(qubits_b),
                  py_array_to_std_vec(qubits_result), qubit_ancilla, is_LSB);
            },
            py::arg("qubit_ancilla"),
            py::arg("qubits_a"), py::arg("qubits_b"),
            py::arg("qubits_result"), py::arg("is_LSB") = true,
            R"(
      Multiplication

     This method adds a Multiplication to the circuit.

     Given two inputs a and b, computes the product a*b and stores the result on a new register.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **qubits_result** the indices of the qubits that will ecode the multiplication result [list of int]
     - **qubits_ancilla** the index of the single required ancilla [int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )")
        .def(
            "controlled_multiplication",
            [&](qb::CircuitBuilder &builder,
                py::array_t<int> qubits_a, py::array_t<int> qubits_b,
                py::array_t<int> qubits_result,
                int qubit_ancilla, bool is_LSB, py::array_t<int> controls_on,
                py::array_t<int> controls_off) {
              builder.ControlledMultiplication(
                  py_array_to_std_vec(qubits_a),
                  py_array_to_std_vec(qubits_b),
                  py_array_to_std_vec(qubits_result),
                  qubit_ancilla,
                  is_LSB,
                  py_array_to_std_vec(controls_on),
                  py_array_to_std_vec(controls_off));
            },
            py::arg("qubit_ancilla"),
            py::arg("qubits_a"), py::arg("qubits_b"),
            py::arg("qubits_result"), py::arg("is_LSB") = true,
            py::arg("controls_on") = py::array_t<int>(),
            py::arg("controls_off") = py::array_t<int>(), R"(
      Controlled Multiplication

     This method adds a controlled Multiplication to the circuit.

     Performs a Multiplication operation on a and b if an only if the
     controls are satisfied.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **qubits_result** the indices of the qubits that will ecode the multiplication result [list of int]
     - **qubits_ancilla** the index of the single required ancilla [int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
        .def(
          "superposition_adder",
          [&](qb::CircuitBuilder &builder, int q0, int q1, int q2,
              py::array_t<int> qubits_flags, py::array_t<int> qubits_string,
              py::array_t<int> qubits_metric, py::object &ae_state_prep_circ,
              py::array_t<int> qubits_ancilla, py::array_t<int> qubits_beam_metric) {

            qb::CircuitBuilder *casted_ae_state_prep_circ =
                ae_state_prep_circ.cast<qb::CircuitBuilder *>();
            assert(casted_ae_state_prep_circ);

            builder.SuperpositionAdder(q0, q1, q2,
                py_array_to_std_vec(qubits_flags), py_array_to_std_vec(qubits_string),
                py_array_to_std_vec(qubits_metric), *casted_ae_state_prep_circ,
                py_array_to_std_vec(qubits_ancilla), py_array_to_std_vec(qubits_beam_metric));
          },
          py::arg("q0"), py::arg("q1"), py::arg("q2"),
          py::arg("qubits_flags") = py::array_t<int>(),
          py::arg("qubits_string") = py::array_t<int>(),
          py::arg("qubits_metric") = py::array_t<int>(),
          py::arg("ae_state_prep_circ"),
          py::arg("qubits_ancilla") = py::array_t<int>(),
          py::arg("qubits_beam_metric") = py::array_t<int>(), R"(
      Superposition adder

     This method adds a Superposition Adder to the circuit.

     Given a superposition state, this circuit computes the mean of the amplitudes of the superposition components.

     Parameters:

     - **q0** the index of the single required ancilla [int]
     - **q1** the index of the single required ancilla [int]
     - **q2** the index of the single required ancilla [int]
     - **qubits_flags** the indices of the flag qubits [list of int]
     - **qubits_string** the indices of the qubits encoding the string [list of int]
     - **qubits_metric** the indices of the qubits encoding the metric value corresponding to the string [list of int]
     - **ae_state_prep_circ** The circuit A used to prepare the input state [CircuitBuilder]
     - **qubits_ancilla** the indices of the required ancilla qubits [list of int]
     - **qubits_beam_metric** the indices of the qubits encoding class' metric [list of int]

    )")
        .def(
          "exponential_search",
          [&](qb::CircuitBuilder &builder, py::str method, py::object state_prep,
              OracleFuncPyType oracle_func, int best_score,
              const std::function<int(int)> f_score, int total_num_qubits,
              py::array_t<int> qubits_string, py::array_t<int> total_metric,
              py::str acc_name) {

            qb::OracleFuncCType oracle_converted =
                [&](int best_score) {
                  // Do conversion
                  auto conv = oracle_func(best_score);
                  return conv.get();
                };

            std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
            StatePrepFuncPyType state_prep_casted;
            qb::StatePrepFuncCType state_prep_func;
            try {
              qb::CircuitBuilder state_prep_casted =
                  state_prep.cast<qb::CircuitBuilder>();
              static_state_prep_circ = state_prep_casted.get();
              state_prep_func = [&](std::vector<int> a, std::vector<int> b,
                                    std::vector<int> c, std::vector<int> d, std::vector<int> e) {
                return static_state_prep_circ;
              };
            } catch (...) {
              state_prep_casted = state_prep.cast<StatePrepFuncPyType>();
              state_prep_func = [&](std::vector<int> qubits_string,
                                    std::vector<int> qubits_metric,
                                    std::vector<int> qubits_next_letter,
                                    std::vector<int> qubits_next_metric,
                                    std::vector<int> qubits_ancilla_adder) {
                // Do conversion
                auto conv =
                    state_prep_casted(std_vec_to_py_array(qubits_string),
                                      std_vec_to_py_array(qubits_metric),
                                      std_vec_to_py_array(qubits_next_letter),
                                      std_vec_to_py_array(qubits_next_metric),
                                      std_vec_to_py_array(qubits_ancilla_adder));
                return conv.get();
              };
            }

            return builder.ExponentialSearch(
                method, state_prep_func, oracle_converted, best_score, f_score,
                total_num_qubits,
                py_array_to_std_vec(qubits_string),
                py_array_to_std_vec(total_metric),
                acc_name);
          },
          py::arg("method"), py::arg("state_prep"), py::arg("oracle"),
          py::arg("best_score"), py::arg("f_score"), py::arg("total_num_qubits"),
          py::arg("qubits_string"), py::arg("total_metric"),
          py::arg("qpu") = "qpp",
            R"(
      Exponential Search

     This method sets up and executes the exponential search routine.

     Exponential search is a way to perform amplitude estimation when the
     size of the "good" subspace is unknown (so the number of Grovers
     operators to use is unknown).

     We implement three variants:
     - canonical exponential search is a specific "guess and check" method
     - MLQAE exponential search uses MLQAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators
     - CQAE exponential search uses canonical QAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators

     Parameters:

     - **method** indicates which method to use. Options are "canonical", "MLQAE", "CQAE" [string]
     - **state_prep** a function which produces the state prep circuit [StatePrepFuncCType]
     - **oracle** a function which produces the oracle circuit that marks the good subspace [OracleFuncCType]
     - **best_score** the current best score [int]
     - **f_score** a function that returns a 1 if the input binary string has value greater than the current best score and 0 otherwise [func(int)->int]
     - **total_num_qubits** total number of qubits [int]
     - **qubits_string** the indices of the qubits encoding the strings [list of int]
     - **total_metric** the indices of the qubits encoding the string scores after any required pre-processing of qubits_metric (required by decoder) [list of int]
     - **qpu** the name of the accelerator used to execute the algorithm [string]

     Returns: a better score if found, otherwise returns the current best
     score
    )")
        .def(
            "q_prime_unitary",
            [&](qb::CircuitBuilder &builder, int nb_qubits_ancilla_metric,
                int nb_qubits_ancilla_letter,
                int nb_qubits_next_letter_probabilities,
                int nb_qubits_next_letter) {
              builder.QPrime(nb_qubits_ancilla_metric,
              nb_qubits_ancilla_letter,
                             nb_qubits_next_letter_probabilities,
                             nb_qubits_next_letter);
            },
            py::arg("nb_qubits_ancilla_metric"),
            py::arg("nb_qubits_ancilla_letter"),
            py::arg("nb_qubits_next_letter_probabilities"),
            py::arg("nb_qubits_next_letter"), R"(
      Q' Unitary

     This method adds a Q' unitary to the circuit.

     Q' is a unitary required for the quantum decoder algorithm.
    )")
        .def(
            "subtraction",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_larger,
                py::array_t<int> qubits_smaller, bool is_LSB, int
                qubit_ancilla) {
              builder.Subtraction(py_array_to_std_vec(qubits_larger),
                                  py_array_to_std_vec(qubits_smaller),
                                  is_LSB, qubit_ancilla);
            },
            py::arg("qubits_larger"), py::arg("qubits_smaller"),
            py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1,
            R"(
      Subtraction

     This method adds a subtraction to the circuit.

     Given two inputs a and b, leaves b unchanged but maps a to the difference a-b, assuming a>b.

     Parameters:

     - **qubits_larger** the indices of the qubits encoding the larger value [list of int]
     - **qubits_smaller** the indices of the qubits encoding the smaller value [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **qubit_ancilla** the index of the required ancilla [list of int]

    )")
        .def(
            "controlled_subtraction",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_larger,
                py::array_t<int> qubits_smaller, py::array_t<int>
                controls_on, py::array_t<int> controls_off, bool is_LSB, int
                qubit_ancilla) {
              builder.ControlledSubtraction(py_array_to_std_vec(qubits_larger),
                                            py_array_to_std_vec(qubits_smaller),
                                            py_array_to_std_vec(controls_on),
                                            py_array_to_std_vec(controls_off),
                                            is_LSB, qubit_ancilla);
            },
            py::arg("qubits_larger"), py::arg("qubits_smaller"),
            py::arg("controls_on") = py::array_t<int>(),
            py::arg("controls_off") = py::array_t<int>(),
            py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1,
            R"(
      Controlled Subtraction

     This method adds a controlled subtraction to the circuit.

     Performs a subtraction operation on a and b if an only if the
     controls are satisfied.

     Parameters:

     - **qubits_larger** the indices of the qubits encoding the larger value [list of int]
     - **qubits_smaller** the indices of the qubits encoding the smaller value [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **qubit_ancilla** the index of the required ancilla [list of int]

    )")
        .def(
            "proper_fraction_division",
            [&](qb::CircuitBuilder &builder, py::array_t<int>
            qubits_numerator,
                py::array_t<int> qubits_denominator,
                py::array_t<int> qubits_fraction, py::array_t<int>
                qubits_ancilla, bool is_LSB) {
              builder.ProperFractionDivision(
                  py_array_to_std_vec(qubits_numerator),
                  py_array_to_std_vec(qubits_denominator),
                  py_array_to_std_vec(qubits_fraction),
                  py_array_to_std_vec(qubits_ancilla), is_LSB);
            },
            py::arg("qubits_numerator"), py::arg("qubits_denominator"),
            py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
            py::arg("is_LSB") = true,
            R"(
      Proper Fraction Division

     This method adds a proper fraction division to the circuit.

     Given two inputs num and denom, calculates num/denom and stores the result in a new register, assuming denom > num

     Parameters:

     - **qubits_numerator** the indices of the qubits encoding the numerator [list of int]
     - **qubits_denominator** the indices of the qubits encoding the denominator [list of int]
     - **qubits_fraction** the indices of the qubits that will ecode the division result [list of int]
     - **qubit_ancilla** the index of the required ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )")
        .def(
            "controlled_proper_fraction_division",
            [&](qb::CircuitBuilder &builder, py::array_t<int>
            qubits_numerator,
                py::array_t<int> qubits_denominator,
                py::array_t<int> qubits_fraction, py::array_t<int>
                qubits_ancilla, py::array_t<int> controls_on,
                py::array_t<int> controls_off, bool is_LSB) {
              builder.ControlledProperFractionDivision(
                  py_array_to_std_vec(qubits_numerator),
                  py_array_to_std_vec(qubits_denominator),
                  py_array_to_std_vec(qubits_fraction),
                  py_array_to_std_vec(qubits_ancilla),
                  py_array_to_std_vec(controls_on),
                  py_array_to_std_vec(controls_off), is_LSB);
            },
            py::arg("qubits_numerator"), py::arg("qubits_denominator"),
            py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
            py::arg("controls_on") = py::array_t<int>(),
            py::arg("controls_off") = py::array_t<int>(),
            py::arg("is_LSB") = true,
            R"(
      Controlled Proper Fraction Division

     This method adds a controlled proper fraction division to the circuit.

     Performs a PFD operation on a and b if an only if the controls are
     satisfied.

     Parameters:

     - **qubits_numerator** the indices of the qubits encoding the numerator [list of int]
     - **qubits_denominator** the indices of the qubits encoding the denominator [list of int]
     - **qubits_fraction** the indices of the qubits that will ecode the division result [list of int]
     - **qubit_ancilla** the index of the required ancilla [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )"
  )
        .def(
            "compare_gt",
            [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
                py::array_t<int> qubits_b,
                int qubit_flag, int qubit_ancilla,
                bool is_LSB) {
              builder.CompareGT(
                  py_array_to_std_vec(qubits_a),
                  py_array_to_std_vec(qubits_b),
                  qubit_flag,
                  qubit_ancilla, is_LSB);
            },
            py::arg("qubits_a"), py::arg("qubits_b"),
            py::arg("qubit_flag"), py::arg("qubit_ancilla"),
            py::arg("is_LSB") = true,
            R"(
    Compare Greater Than

   This method adds a greater-than comparator to the circuit.

   Given two binary strings a and b, this comparator flips a flag qubit
   whenever a>b. This method uses far less ancilla than the more general
   comparator method provided.

   Parameters:

   - **qubits_a** The indices of the qubits encoding a [list of int]
   - **qubits_b** The indices of the qubits encoding b [list of int]
   - **qubit_flag** The index of the flag qubit that is flipped whenever a>b [int]
   - **qubit_ancilla** The index of the single ancilla qubit required [int]
   - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
  )");
  m.def(
      "run_canonical_ae",
      [&](py::object &state_prep, py::object &grover_op, int precision,
          int num_state_prep_qubits, int num_trial_qubits,
          py::array_t<int> precision_qubits, py::array_t<int> trial_qubits,
          py::str acc_name) {
        qb::CircuitBuilder builder;

        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_grover_op =
            grover_op.cast<qb::CircuitBuilder *>();
        assert(casted_grover_op);

        return builder.RunCanonicalAmplitudeEstimation(
            *casted_state_prep, *casted_grover_op, precision,
            num_state_prep_qubits, num_trial_qubits,
            py_array_to_std_vec(precision_qubits),
            py_array_to_std_vec(trial_qubits), acc_name);
      },
      py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
      py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
      py::arg("precision_qubits") = py::array_t<int>(),
      py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
      "Execute Canonical Quantum Amplitude Estimation Procedure with "
      "pre-constructed Grover operator circuit, including "
      "post-processing.");
  m.def(
      "run_canonical_ae_with_oracle",
      [&](py::object &state_prep, py::object &oracle, int precision,
          int num_state_prep_qubits, int num_trial_qubits,
          py::array_t<int> precision_qubits, py::array_t<int> trial_qubits,
          py::str acc_name) {
        qb::CircuitBuilder builder;
        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_oracle = oracle.cast<qb::CircuitBuilder *>();
        assert(casted_oracle);
        return builder.RunCanonicalAmplitudeEstimationWithOracle(
            *casted_state_prep, *casted_oracle, precision,
            num_state_prep_qubits, num_trial_qubits,
            py_array_to_std_vec(precision_qubits),
            py_array_to_std_vec(trial_qubits), acc_name);
      },
      py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
      py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
      py::arg("precision_qubits") = py::array_t<int>(),
      py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
      "Execute Canonical Quantum Amplitude Estimation procedure for the "
      "oracle "
      "including "
      "post-processing.");
  m.def(
      "run_MLQAE",
      [&](py::object &state_prep, py::object &oracle,
          std::function<int(std::string, int)> is_in_good_subspace,
          py::array_t<int> score_qubits, int total_num_qubits, int num_runs,
          int shots, py::str acc_name) {
        qb::CircuitBuilder builder;
        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_oracle = oracle.cast<qb::CircuitBuilder *>();
        assert(casted_oracle);
        return builder.RunMLAmplitudeEstimation(
            *casted_state_prep, *casted_oracle, is_in_good_subspace,
            py_array_to_std_vec(score_qubits), total_num_qubits, num_runs,
            shots, acc_name);
      },
      py::arg("state_prep"), py::arg("oracle"), py::arg("is_in_good_subspace"),
      py::arg("score_qubits"), py::arg("total_num_qubits"),
      py::arg("num_runs") = 4, py::arg("shots") = 100, py::arg("qpu") = "qpp",
      "MLQAE");
  m.def(
      "exponential_search",
      [&](py::str method, py::object state_prep, OracleFuncPyType oracle_func,
          int best_score, const std::function<int(int)> f_score, int total_num_qubits,
          py::array_t<int> qubits_string, py::array_t<int> total_metric,
          py::str acc_name) {

        qb::CircuitBuilder builder;
        qb::OracleFuncCType oracle_converted =
            [&](int best_score) {
              // Do conversion
              auto conv = oracle_func(best_score);
              return conv.get();
            };

        std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
        StatePrepFuncPyType state_prep_casted;
        qb::StatePrepFuncCType state_prep_func;
        try {
          qb::CircuitBuilder state_prep_casted =
              state_prep.cast<qb::CircuitBuilder>();
          static_state_prep_circ = state_prep_casted.get();
          state_prep_func = [&](std::vector<int> a, std::vector<int> b,
                                std::vector<int> c, std::vector<int> d,
                                std::vector<int> e) {
            return static_state_prep_circ;
          };
        } catch (...) {
          state_prep_casted = state_prep.cast<StatePrepFuncPyType>();
          state_prep_func = [&](std::vector<int> qubits_string,
                                std::vector<int> qubits_metric,
                                std::vector<int> qubits_next_letter,
                                std::vector<int> qubits_next_metric,
                                std::vector<int> qubits_ancilla_adder) {
            // Do conversion
            auto conv =
                state_prep_casted(std_vec_to_py_array(qubits_string),
                                  std_vec_to_py_array(qubits_metric),
                                  std_vec_to_py_array(qubits_next_letter),
                                  std_vec_to_py_array(qubits_next_metric),
                                  std_vec_to_py_array(qubits_ancilla_adder));
            return conv.get();
          };
        }
        return builder.ExponentialSearch(
            method, state_prep_func, oracle_converted, best_score, f_score,
            total_num_qubits, py_array_to_std_vec(qubits_string),
            py_array_to_std_vec(total_metric), acc_name);
      },
      py::arg("method"), py::arg("state_prep"), py::arg("oracle"),
      py::arg("best_score"), py::arg("f_score"), py::arg("total_num_qubits"),
      py::arg("qubits_string"), py::arg("total_metric"),
      py::arg("qpu") = "qpp", "Exp Search");

  py::add_ostream_redirect(m);

  // - - - - - - - - - - - - - - - - -  optimization modules - - - - - - - //
  // - - - - - - - - - - - - - - - - -  vqee - - - - - - - - - - - - - - - //
  py::module_ m_opt =
      m.def_submodule("optimization", "Optimization modules within qb_core");
  py::class_<qb::vqee::Params>(m_opt, "vqee_Params")
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
      .def_readwrite("optimalParameters", &qb::vqee::Params::theta)
      .def_readonly("optimalValue", &qb::vqee::Params::optimalValue);

  py::enum_<qb::vqee::JobID>(m_opt, "vqee_JobID")
      .value("H2_explicit", qb::vqee::JobID::H2_explicit)
      .value("H1_HEA", qb::vqee::JobID::H1_HEA)
      .value("H2_UCCSD", qb::vqee::JobID::H2_UCCSD)
      .value("H2_ASWAP", qb::vqee::JobID::H2_ASWAP)
      .value("H5_UCCSD", qb::vqee::JobID::H5_UCCSD);

  m_opt.def("makeJob", &qb::vqee::makeJob,
            "makeJob(vqee_JobID) -> vqee::Params: returns a predefined example "
            "job setup",
            py::arg("jobID"));

  py::class_<qb::vqee::VQEE>(m_opt, "vqee_VQEE")
      .def(py::init<qb::vqee::Params &>())
      .def("run", &qb::vqee::VQEE::optimize, "solve VQE problem");


  // - - - - - - - - - - - - - - - - -  qml  - - - - - - - - - - - - - - - //
  py::enum_<qb::qml::DefaultAnsatzes>(m_opt, "defaultAnsatzes")
    .value("qrlRDBMS", qb::qml::DefaultAnsatzes::qrlRDBMS)
    ;

  py::class_<qb::qml::ParamCirc, qb::CircuitBuilder>(m_opt, "ParamCirc") 
    .def(py::init<size_t, qb::qml::DefaultAnsatzes, size_t, qb::VectorString>())
    .def(py::init<size_t>())
    .def("numInputs", &qb::qml::ParamCirc::getNumInputs)
    .def("numParams", &qb::qml::ParamCirc::getNumParams)
    .def("numQubits", &qb::qml::ParamCirc::getNumQubits)
    .def("numAnsatzRepetitions_", &qb::qml::ParamCirc::getNumAnsatzRepetitions)
    .def("RX", &qb::qml::ParamCirc::RX)
    .def("RY", &qb::qml::ParamCirc::RY)
    .def("RZ", &qb::qml::ParamCirc::RZ)
    .def("U1", &qb::qml::ParamCirc::U1)
    .def("CPhase", &qb::qml::ParamCirc::CPhase)
    .def("reupload", &qb::qml::ParamCirc::reupload)
    ;
  
  py::class_<qb::qml::QMLExecutor>(m_opt, "QMLExecutor") 
    .def(py::init<qb::qml::ParamCirc &, std::vector<double> , std::vector<double> >())
    .def_property("circuit", &qb::qml::QMLExecutor::getCircuit, &qb::qml::QMLExecutor::setCircuit)
    .def_property("inputParams", &qb::qml::QMLExecutor::getInputParams, &qb::qml::QMLExecutor::setInputParams)
    .def_property("weights", &qb::qml::QMLExecutor::getWeights, &qb::qml::QMLExecutor::setWeights)
    .def_readwrite("acc", &qb::qml::QMLExecutor::acc)
    .def("run", &qb::qml::QMLExecutor::run)
    .def("getStats", &qb::qml::QMLExecutor::getStats)
    .def("runGradients", &qb::qml::QMLExecutor::runGradients)
    .def("getGradients", &qb::qml::QMLExecutor::getStatGradients)
    ;


  // - - - - - - - - - - - - - - - - -  qaoa - - - - - - - - - - - - - - - //
  py::class_<qb::op::QaoaSimple>(m_opt, "qaoa_QaoaSimple")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname",  & qb::op::QaoaSimple::get_colnames, & qb::op::QaoaSimple::set_colname,  qb::op::QaoaSimple::help_colnames_ )
      .def_property("colnames", & qb::op::QaoaSimple::get_colnames, & qb::op::QaoaSimple::set_colnames, qb::op::QaoaSimple::help_colnames_ )
      .def_property("rowname",  & qb::op::QaoaSimple::get_rownames, & qb::op::QaoaSimple::set_rowname,  qb::op::QaoaSimple::help_rownames_ )
      .def_property("rownames", & qb::op::QaoaSimple::get_rownames, & qb::op::QaoaSimple::set_rownames, qb::op::QaoaSimple::help_rownames_ )
      .def_property("theta",  & qb::op::QaoaSimple::get_thetas, & qb::op::QaoaSimple::set_theta,  qb::op::QaoaSimple::help_thetas_ )
      .def_property("thetas", & qb::op::QaoaSimple::get_thetas, & qb::op::QaoaSimple::set_thetas, qb::op::QaoaSimple::help_thetas_ )
      .def_property("acc",  & qb::op::QaoaSimple::get_accs, & qb::op::QaoaSimple::set_acc,  qb::op::QaoaSimple::help_accs_ )
      .def_property("accs", & qb::op::QaoaSimple::get_accs, & qb::op::QaoaSimple::set_accs, qb::op::QaoaSimple::help_accs_ )
      .def_property("ham",  & qb::op::QaoaSimple::get_hams, & qb::op::QaoaSimple::set_ham,  qb::op::QaoaSimple::help_hams_ )
      .def_property("hams", & qb::op::QaoaSimple::get_hams, & qb::op::QaoaSimple::set_hams, qb::op::QaoaSimple::help_hams_ )
      .def_property("qaoa_step",  & qb::op::QaoaSimple::get_qaoa_steps, & qb::op::QaoaSimple::set_qaoa_step,  qb::op::QaoaSimple::help_qaoa_steps_ )
      .def_property("qaoa_steps", & qb::op::QaoaSimple::get_qaoa_steps, & qb::op::QaoaSimple::set_qaoa_steps, qb::op::QaoaSimple::help_qaoa_steps_ )
      .def_property("qn",  & qb::op::QaoaSimple::get_qns, & qb::op::QaoaSimple::set_qn,  qb::op::QaoaSimple::help_qns_ )
      .def_property("qns", & qb::op::QaoaSimple::get_qns, & qb::op::QaoaSimple::set_qns, qb::op::QaoaSimple::help_qns_ )
      .def_property("rn",  & qb::op::QaoaSimple::get_rns, & qb::op::QaoaSimple::set_rn,  qb::op::QaoaSimple::help_rns_ )
      .def_property("rns", & qb::op::QaoaSimple::get_rns, & qb::op::QaoaSimple::set_rns, qb::op::QaoaSimple::help_rns_ )
      .def_property("sn",  & qb::op::QaoaSimple::get_sns, & qb::op::QaoaSimple::set_sn,  qb::op::QaoaSimple::help_sns_ )
      .def_property("sns", & qb::op::QaoaSimple::get_sns, & qb::op::QaoaSimple::set_sns, qb::op::QaoaSimple::help_sns_ )
      .def_property("noise",  & qb::op::QaoaSimple::get_noises, & qb::op::QaoaSimple::set_noise,  qb::op::QaoaSimple::help_noises_ )
      .def_property("noises", & qb::op::QaoaSimple::get_noises, & qb::op::QaoaSimple::set_noises, qb::op::QaoaSimple::help_noises_ )
      .def_property("extended_param",  & qb::op::QaoaSimple::get_extended_params, & qb::op::QaoaSimple::set_extended_param,  qb::op::QaoaSimple::help_extended_params_ )
      .def_property("extended_params", & qb::op::QaoaSimple::get_extended_params, & qb::op::QaoaSimple::set_extended_params, qb::op::QaoaSimple::help_extended_params_ )
      .def_property("method",  & qb::op::QaoaSimple::get_methods, & qb::op::QaoaSimple::set_method,  qb::op::QaoaSimple::help_methods_ )
      .def_property("methods", & qb::op::QaoaSimple::get_methods, & qb::op::QaoaSimple::set_methods, qb::op::QaoaSimple::help_methods_ )
      .def_property("grad",  & qb::op::QaoaSimple::get_grads, & qb::op::QaoaSimple::set_grad,  qb::op::QaoaSimple::help_grads_ )
      .def_property("grads", & qb::op::QaoaSimple::get_grads, & qb::op::QaoaSimple::set_grads, qb::op::QaoaSimple::help_grads_ )
      .def_property("gradient_strategy",  & qb::op::QaoaSimple::get_gradient_strategys, & qb::op::QaoaSimple::set_gradient_strategy,  qb::op::QaoaSimple::help_gradient_strategys_ )
      .def_property("gradient_strategys", & qb::op::QaoaSimple::get_gradient_strategys, & qb::op::QaoaSimple::set_gradient_strategys, qb::op::QaoaSimple::help_gradient_strategys_ )
      .def_property("maxeval",  & qb::op::QaoaSimple::get_maxevals, & qb::op::QaoaSimple::set_maxeval,  qb::op::QaoaSimple::help_maxevals_ )
      .def_property("maxevals", & qb::op::QaoaSimple::get_maxevals, & qb::op::QaoaSimple::set_maxevals, qb::op::QaoaSimple::help_maxevals_ )
      .def_property("functol",  & qb::op::QaoaSimple::get_functols, & qb::op::QaoaSimple::set_functol,  qb::op::QaoaSimple::help_functols_ )
      .def_property("functols", & qb::op::QaoaSimple::get_functols, & qb::op::QaoaSimple::set_functols, qb::op::QaoaSimple::help_functols_ )
      .def_property("optimum_energy_abstol",  & qb::op::QaoaSimple::get_optimum_energy_abstols, & qb::op::QaoaSimple::set_optimum_energy_abstol,  qb::op::QaoaSimple::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_abstols", & qb::op::QaoaSimple::get_optimum_energy_abstols, & qb::op::QaoaSimple::set_optimum_energy_abstols, qb::op::QaoaSimple::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_lowerbound",  & qb::op::QaoaSimple::get_optimum_energy_lowerbounds, & qb::op::QaoaSimple::set_optimum_energy_lowerbound,  qb::op::QaoaSimple::help_optimum_energy_lowerbounds_ )
      .def_property("optimum_energy_lowerbounds", & qb::op::QaoaSimple::get_optimum_energy_lowerbounds, & qb::op::QaoaSimple::set_optimum_energy_lowerbounds, qb::op::QaoaSimple::help_optimum_energy_lowerbounds_ )
      .def_property("out_eigenstate",  & qb::op::QaoaSimple::get_out_eigenstates, & qb::op::QaoaSimple::set_out_eigenstate,  qb::op::QaoaSimple::help_out_eigenstates_ )
      .def_property("out_eigenstates", & qb::op::QaoaSimple::get_out_eigenstates, & qb::op::QaoaSimple::set_out_eigenstates, qb::op::QaoaSimple::help_out_eigenstates_ )
      .def_property("out_energy",  & qb::op::QaoaSimple::get_out_energys, & qb::op::QaoaSimple::set_out_energy,  qb::op::QaoaSimple::help_out_energys_ )
      .def_property("out_energys", & qb::op::QaoaSimple::get_out_energys, & qb::op::QaoaSimple::set_out_energys, qb::op::QaoaSimple::help_out_energys_ )
      .def_property("out_jacobian",  & qb::op::QaoaSimple::get_out_jacobians, & qb::op::QaoaSimple::set_out_jacobian,  qb::op::QaoaSimple::help_out_jacobians_ )
      .def_property("out_jacobians", & qb::op::QaoaSimple::get_out_jacobians, & qb::op::QaoaSimple::set_out_jacobians, qb::op::QaoaSimple::help_out_jacobians_ )
      .def_property("out_theta",  & qb::op::QaoaSimple::get_out_thetas, & qb::op::QaoaSimple::set_out_theta,  qb::op::QaoaSimple::help_out_thetas_ )
      .def_property("out_thetas", & qb::op::QaoaSimple::get_out_thetas, & qb::op::QaoaSimple::set_out_thetas, qb::op::QaoaSimple::help_out_thetas_ )
      .def_property("out_quantum_energy_calc_time",  & qb::op::QaoaSimple::get_out_quantum_energy_calc_times, & qb::op::QaoaSimple::set_out_quantum_energy_calc_time,  qb::op::QaoaSimple::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_energy_calc_times", & qb::op::QaoaSimple::get_out_quantum_energy_calc_times, & qb::op::QaoaSimple::set_out_quantum_energy_calc_times, qb::op::QaoaSimple::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_jacobian_calc_time",  & qb::op::QaoaSimple::get_out_quantum_jacobian_calc_times, & qb::op::QaoaSimple::set_out_quantum_jacobian_calc_time,  qb::op::QaoaSimple::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_quantum_jacobian_calc_times", & qb::op::QaoaSimple::get_out_quantum_jacobian_calc_times, & qb::op::QaoaSimple::set_out_quantum_jacobian_calc_times, qb::op::QaoaSimple::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_time",  & qb::op::QaoaSimple::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaSimple::set_out_classical_energy_jacobian_total_calc_time,  qb::op::QaoaSimple::help_out_classical_energy_jacobian_total_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_times", & qb::op::QaoaSimple::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaSimple::set_out_classical_energy_jacobian_total_calc_times, qb::op::QaoaSimple::help_out_classical_energy_jacobian_total_calc_times_ )
      
      .def("run", py::overload_cast<>(& qb::op::QaoaBase::run), "Execute all declared experiments under all conditions")
      .def("runit", py::overload_cast<const size_t &, const size_t &>(& qb::op::QaoaSimple::run), "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", & qb::op::QaoaSimple::get_summary,  "Print summary of qb_op_qaoa settings")
      ;

// - - - - - - - - - - - - - - - - -  qaoa recursive - - - - - - - - - - - - - - - //
    py::class_<qb::op::QaoaRecursive>(m_opt, "qaoa_QaoaRecursive")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname",  & qb::op::QaoaRecursive::get_colnames, & qb::op::QaoaRecursive::set_colname,  qb::op::QaoaRecursive::help_colnames_ )
      .def_property("colnames", & qb::op::QaoaRecursive::get_colnames, & qb::op::QaoaRecursive::set_colnames, qb::op::QaoaRecursive::help_colnames_ )
      .def_property("rowname",  & qb::op::QaoaRecursive::get_rownames, & qb::op::QaoaRecursive::set_rowname,  qb::op::QaoaRecursive::help_rownames_ )
      .def_property("rownames", & qb::op::QaoaRecursive::get_rownames, & qb::op::QaoaRecursive::set_rownames, qb::op::QaoaRecursive::help_rownames_ )
      .def_property("acc",  & qb::op::QaoaRecursive::get_accs, & qb::op::QaoaRecursive::set_acc,  qb::op::QaoaRecursive::help_accs_ )
      .def_property("accs", & qb::op::QaoaRecursive::get_accs, & qb::op::QaoaRecursive::set_accs, qb::op::QaoaRecursive::help_accs_ )
      .def_property("ham",  & qb::op::QaoaRecursive::get_hams, & qb::op::QaoaRecursive::set_ham,  qb::op::QaoaRecursive::help_hams_ )
      .def_property("hams", & qb::op::QaoaRecursive::get_hams, & qb::op::QaoaRecursive::set_hams, qb::op::QaoaRecursive::help_hams_ )
      .def_property("qaoa_step",  & qb::op::QaoaRecursive::get_qaoa_steps, & qb::op::QaoaRecursive::set_qaoa_step,  qb::op::QaoaRecursive::help_qaoa_steps_ )
      .def_property("qaoa_steps", & qb::op::QaoaRecursive::get_qaoa_steps, & qb::op::QaoaRecursive::set_qaoa_steps, qb::op::QaoaRecursive::help_qaoa_steps_ )
      .def_property("n_c",  & qb::op::QaoaRecursive::get_n_cs, & qb::op::QaoaRecursive::set_n_c,  qb::op::QaoaRecursive::help_n_cs_ )
      .def_property("n_cs", & qb::op::QaoaRecursive::get_n_cs, & qb::op::QaoaRecursive::set_n_cs, qb::op::QaoaRecursive::help_n_cs_ )
      .def_property("qn",  & qb::op::QaoaRecursive::get_qns, & qb::op::QaoaRecursive::set_qn,  qb::op::QaoaRecursive::help_qns_ )
      .def_property("qns", & qb::op::QaoaRecursive::get_qns, & qb::op::QaoaRecursive::set_qns, qb::op::QaoaRecursive::help_qns_ )
      .def_property("rn",  & qb::op::QaoaRecursive::get_rns, & qb::op::QaoaRecursive::set_rn,  qb::op::QaoaRecursive::help_rns_ )
      .def_property("rns", & qb::op::QaoaRecursive::get_rns, & qb::op::QaoaRecursive::set_rns, qb::op::QaoaRecursive::help_rns_ )
      .def_property("sn",  & qb::op::QaoaRecursive::get_sns, & qb::op::QaoaRecursive::set_sn,  qb::op::QaoaRecursive::help_sns_ )
      .def_property("sns", & qb::op::QaoaRecursive::get_sns, & qb::op::QaoaRecursive::set_sns, qb::op::QaoaRecursive::help_sns_ )
      .def_property("noise",  & qb::op::QaoaRecursive::get_noises, & qb::op::QaoaRecursive::set_noise,  qb::op::QaoaRecursive::help_noises_ )
      .def_property("noises", & qb::op::QaoaRecursive::get_noises, & qb::op::QaoaRecursive::set_noises, qb::op::QaoaRecursive::help_noises_ )
      .def_property("extended_param",  & qb::op::QaoaRecursive::get_extended_params, & qb::op::QaoaRecursive::set_extended_param,  qb::op::QaoaRecursive::help_extended_params_ )
      .def_property("extended_params", & qb::op::QaoaRecursive::get_extended_params, & qb::op::QaoaRecursive::set_extended_params, qb::op::QaoaRecursive::help_extended_params_ )
      .def_property("method",  & qb::op::QaoaRecursive::get_methods, & qb::op::QaoaRecursive::set_method,  qb::op::QaoaRecursive::help_methods_ )
      .def_property("methods", & qb::op::QaoaRecursive::get_methods, & qb::op::QaoaRecursive::set_methods, qb::op::QaoaRecursive::help_methods_ )
      .def_property("grad",  & qb::op::QaoaRecursive::get_grads, & qb::op::QaoaRecursive::set_grad,  qb::op::QaoaRecursive::help_grads_ )
      .def_property("grads", & qb::op::QaoaRecursive::get_grads, & qb::op::QaoaRecursive::set_grads, qb::op::QaoaRecursive::help_grads_ )
      .def_property("gradient_strategy",  & qb::op::QaoaRecursive::get_gradient_strategys, & qb::op::QaoaRecursive::set_gradient_strategy,  qb::op::QaoaRecursive::help_gradient_strategys_ )
      .def_property("gradient_strategys", & qb::op::QaoaRecursive::get_gradient_strategys, & qb::op::QaoaRecursive::set_gradient_strategys, qb::op::QaoaRecursive::help_gradient_strategys_ )
      .def_property("maxeval",  & qb::op::QaoaRecursive::get_maxevals, & qb::op::QaoaRecursive::set_maxeval,  qb::op::QaoaRecursive::help_maxevals_ )
      .def_property("maxevals", & qb::op::QaoaRecursive::get_maxevals, & qb::op::QaoaRecursive::set_maxevals, qb::op::QaoaRecursive::help_maxevals_ )
      .def_property("functol",  & qb::op::QaoaRecursive::get_functols, & qb::op::QaoaRecursive::set_functol,  qb::op::QaoaRecursive::help_functols_ )
      .def_property("functols", & qb::op::QaoaRecursive::get_functols, & qb::op::QaoaRecursive::set_functols, qb::op::QaoaRecursive::help_functols_ )
      .def_property("optimum_energy_abstol",  & qb::op::QaoaRecursive::get_optimum_energy_abstols, & qb::op::QaoaRecursive::set_optimum_energy_abstol,  qb::op::QaoaRecursive::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_abstols", & qb::op::QaoaRecursive::get_optimum_energy_abstols, & qb::op::QaoaRecursive::set_optimum_energy_abstols, qb::op::QaoaRecursive::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_lowerbound",  & qb::op::QaoaRecursive::get_optimum_energy_lowerbounds, & qb::op::QaoaRecursive::set_optimum_energy_lowerbound,  qb::op::QaoaRecursive::help_optimum_energy_lowerbounds_ )
      .def_property("optimum_energy_lowerbounds", & qb::op::QaoaRecursive::get_optimum_energy_lowerbounds, & qb::op::QaoaRecursive::set_optimum_energy_lowerbounds, qb::op::QaoaRecursive::help_optimum_energy_lowerbounds_ )
      .def_property("out_eigenstate",  & qb::op::QaoaRecursive::get_out_eigenstates, & qb::op::QaoaRecursive::set_out_eigenstate,  qb::op::QaoaRecursive::help_out_eigenstates_ )
      .def_property("out_eigenstates", & qb::op::QaoaRecursive::get_out_eigenstates, & qb::op::QaoaRecursive::set_out_eigenstates, qb::op::QaoaRecursive::help_out_eigenstates_ )
      .def_property("out_energy",  & qb::op::QaoaRecursive::get_out_energys, & qb::op::QaoaRecursive::set_out_energy,  qb::op::QaoaRecursive::help_out_energys_ )
      .def_property("out_energys", & qb::op::QaoaRecursive::get_out_energys, & qb::op::QaoaRecursive::set_out_energys, qb::op::QaoaRecursive::help_out_energys_ )
      .def_property("out_jacobian",  & qb::op::QaoaRecursive::get_out_jacobians, & qb::op::QaoaRecursive::set_out_jacobian,  qb::op::QaoaRecursive::help_out_jacobians_ )
      .def_property("out_jacobians", & qb::op::QaoaRecursive::get_out_jacobians, & qb::op::QaoaRecursive::set_out_jacobians, qb::op::QaoaRecursive::help_out_jacobians_ )
      .def_property("out_theta",  & qb::op::QaoaRecursive::get_out_thetas, & qb::op::QaoaRecursive::set_out_theta,  qb::op::QaoaRecursive::help_out_thetas_ )
      .def_property("out_thetas", & qb::op::QaoaRecursive::get_out_thetas, & qb::op::QaoaRecursive::set_out_thetas, qb::op::QaoaRecursive::help_out_thetas_ )
      .def_property("out_quantum_energy_calc_time",  & qb::op::QaoaRecursive::get_out_quantum_energy_calc_times, & qb::op::QaoaRecursive::set_out_quantum_energy_calc_time,  qb::op::QaoaRecursive::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_energy_calc_times", & qb::op::QaoaRecursive::get_out_quantum_energy_calc_times, & qb::op::QaoaRecursive::set_out_quantum_energy_calc_times, qb::op::QaoaRecursive::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_jacobian_calc_time",  & qb::op::QaoaRecursive::get_out_quantum_jacobian_calc_times, & qb::op::QaoaRecursive::set_out_quantum_jacobian_calc_time,  qb::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_quantum_jacobian_calc_times", & qb::op::QaoaRecursive::get_out_quantum_jacobian_calc_times, & qb::op::QaoaRecursive::set_out_quantum_jacobian_calc_times, qb::op::QaoaRecursive::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_time",  & qb::op::QaoaRecursive::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaRecursive::set_out_classical_energy_jacobian_total_calc_time,  qb::op::QaoaRecursive::help_out_classical_energy_jacobian_total_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_times", & qb::op::QaoaRecursive::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaRecursive::set_out_classical_energy_jacobian_total_calc_times, qb::op::QaoaRecursive::help_out_classical_energy_jacobian_total_calc_times_ )
      
      .def("run", py::overload_cast<>(& qb::op::QaoaBase::run), "Execute all declared experiments under all conditions")
      .def("runit", py::overload_cast<const size_t &, const size_t &>(& qb::op::QaoaRecursive::run), "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", & qb::op::QaoaRecursive::get_summary,  "Print summary of qb_op_qaoa settings")
      ;

// - - - - - - - - - - - - - - - - -  qaoa warm start - - - - - - - - - - - - - - - //
    py::class_<qb::op::QaoaWarmStart>(m_opt, "qaoa_QaoaWarmStart")    //  py::class_<qb::op::QaoaWarmStart, qb::op::QaoaBase>(m, "QaoaWarmStart")
      .def(py::init<const bool>())
      .def(py::init())
      .def_property("colname",  & qb::op::QaoaWarmStart::get_colnames, & qb::op::QaoaWarmStart::set_colname,  qb::op::QaoaWarmStart::help_colnames_ )
      .def_property("colnames", & qb::op::QaoaWarmStart::get_colnames, & qb::op::QaoaWarmStart::set_colnames, qb::op::QaoaWarmStart::help_colnames_ )
      .def_property("rowname",  & qb::op::QaoaWarmStart::get_rownames, & qb::op::QaoaWarmStart::set_rowname,  qb::op::QaoaWarmStart::help_rownames_ )
      .def_property("rownames", & qb::op::QaoaWarmStart::get_rownames, & qb::op::QaoaWarmStart::set_rownames, qb::op::QaoaWarmStart::help_rownames_ )
      .def_property("theta",  & qb::op::QaoaWarmStart::get_thetas, & qb::op::QaoaWarmStart::set_theta,  qb::op::QaoaWarmStart::help_thetas_ )
      .def_property("thetas", & qb::op::QaoaWarmStart::get_thetas, & qb::op::QaoaWarmStart::set_thetas, qb::op::QaoaWarmStart::help_thetas_ )
      .def_property("acc",  & qb::op::QaoaWarmStart::get_accs, & qb::op::QaoaWarmStart::set_acc,  qb::op::QaoaWarmStart::help_accs_ )
      .def_property("accs", & qb::op::QaoaWarmStart::get_accs, & qb::op::QaoaWarmStart::set_accs, qb::op::QaoaWarmStart::help_accs_ )
      .def_property("good_cut",  & qb::op::QaoaWarmStart::get_good_cuts, & qb::op::QaoaWarmStart::set_good_cut,  qb::op::QaoaWarmStart::help_good_cuts_ )
      .def_property("good_cuts", & qb::op::QaoaWarmStart::get_good_cuts, & qb::op::QaoaWarmStart::set_good_cuts, qb::op::QaoaWarmStart::help_good_cuts_ )
      .def_property("ham",  & qb::op::QaoaWarmStart::get_hams, & qb::op::QaoaWarmStart::set_ham,  qb::op::QaoaWarmStart::help_hams_ )
      .def_property("hams", & qb::op::QaoaWarmStart::get_hams, & qb::op::QaoaWarmStart::set_hams, qb::op::QaoaWarmStart::help_hams_ )
      .def_property("qaoa_step",  & qb::op::QaoaWarmStart::get_qaoa_steps, & qb::op::QaoaWarmStart::set_qaoa_step,  qb::op::QaoaWarmStart::help_qaoa_steps_ )
      .def_property("qaoa_steps", & qb::op::QaoaWarmStart::get_qaoa_steps, & qb::op::QaoaWarmStart::set_qaoa_steps, qb::op::QaoaWarmStart::help_qaoa_steps_ )
      .def_property("qn",  & qb::op::QaoaWarmStart::get_qns, & qb::op::QaoaWarmStart::set_qn,  qb::op::QaoaWarmStart::help_qns_ )
      .def_property("qns", & qb::op::QaoaWarmStart::get_qns, & qb::op::QaoaWarmStart::set_qns, qb::op::QaoaWarmStart::help_qns_ )
      .def_property("rn",  & qb::op::QaoaWarmStart::get_rns, & qb::op::QaoaWarmStart::set_rn,  qb::op::QaoaWarmStart::help_rns_ )
      .def_property("rns", & qb::op::QaoaWarmStart::get_rns, & qb::op::QaoaWarmStart::set_rns, qb::op::QaoaWarmStart::help_rns_ )
      .def_property("sn",  & qb::op::QaoaWarmStart::get_sns, & qb::op::QaoaWarmStart::set_sn,  qb::op::QaoaWarmStart::help_sns_ )
      .def_property("sns", & qb::op::QaoaWarmStart::get_sns, & qb::op::QaoaWarmStart::set_sns, qb::op::QaoaWarmStart::help_sns_ )
      .def_property("noise",  & qb::op::QaoaWarmStart::get_noises, & qb::op::QaoaWarmStart::set_noise,  qb::op::QaoaWarmStart::help_noises_ )
      .def_property("noises", & qb::op::QaoaWarmStart::get_noises, & qb::op::QaoaWarmStart::set_noises, qb::op::QaoaWarmStart::help_noises_ )
      .def_property("extended_param",  & qb::op::QaoaWarmStart::get_extended_params, & qb::op::QaoaWarmStart::set_extended_param,  qb::op::QaoaWarmStart::help_extended_params_ )
      .def_property("extended_params", & qb::op::QaoaWarmStart::get_extended_params, & qb::op::QaoaWarmStart::set_extended_params, qb::op::QaoaWarmStart::help_extended_params_ )
      .def_property("method",  & qb::op::QaoaWarmStart::get_methods, & qb::op::QaoaWarmStart::set_method,  qb::op::QaoaWarmStart::help_methods_ )
      .def_property("methods", & qb::op::QaoaWarmStart::get_methods, & qb::op::QaoaWarmStart::set_methods, qb::op::QaoaWarmStart::help_methods_ )
      .def_property("grad",  & qb::op::QaoaWarmStart::get_grads, & qb::op::QaoaWarmStart::set_grad,  qb::op::QaoaWarmStart::help_grads_ )
      .def_property("grads", & qb::op::QaoaWarmStart::get_grads, & qb::op::QaoaWarmStart::set_grads, qb::op::QaoaWarmStart::help_grads_ )
      .def_property("gradient_strategy",  & qb::op::QaoaWarmStart::get_gradient_strategys, & qb::op::QaoaWarmStart::set_gradient_strategy,  qb::op::QaoaWarmStart::help_gradient_strategys_ )
      .def_property("gradient_strategys", & qb::op::QaoaWarmStart::get_gradient_strategys, & qb::op::QaoaWarmStart::set_gradient_strategys, qb::op::QaoaWarmStart::help_gradient_strategys_ )
      .def_property("maxeval",  & qb::op::QaoaWarmStart::get_maxevals, & qb::op::QaoaWarmStart::set_maxeval,  qb::op::QaoaWarmStart::help_maxevals_ )
      .def_property("maxevals", & qb::op::QaoaWarmStart::get_maxevals, & qb::op::QaoaWarmStart::set_maxevals, qb::op::QaoaWarmStart::help_maxevals_ )
      .def_property("functol",  & qb::op::QaoaWarmStart::get_functols, & qb::op::QaoaWarmStart::set_functol,  qb::op::QaoaWarmStart::help_functols_ )
      .def_property("functols", & qb::op::QaoaWarmStart::get_functols, & qb::op::QaoaWarmStart::set_functols, qb::op::QaoaWarmStart::help_functols_ )
      .def_property("optimum_energy_abstol",  & qb::op::QaoaWarmStart::get_optimum_energy_abstols, & qb::op::QaoaWarmStart::set_optimum_energy_abstol,  qb::op::QaoaWarmStart::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_abstols", & qb::op::QaoaWarmStart::get_optimum_energy_abstols, & qb::op::QaoaWarmStart::set_optimum_energy_abstols, qb::op::QaoaWarmStart::help_optimum_energy_abstols_ )
      .def_property("optimum_energy_lowerbound",  & qb::op::QaoaWarmStart::get_optimum_energy_lowerbounds, & qb::op::QaoaWarmStart::set_optimum_energy_lowerbound,  qb::op::QaoaWarmStart::help_optimum_energy_lowerbounds_ )
      .def_property("optimum_energy_lowerbounds", & qb::op::QaoaWarmStart::get_optimum_energy_lowerbounds, & qb::op::QaoaWarmStart::set_optimum_energy_lowerbounds, qb::op::QaoaWarmStart::help_optimum_energy_lowerbounds_ )
      .def_property("out_eigenstate",  & qb::op::QaoaWarmStart::get_out_eigenstates, & qb::op::QaoaWarmStart::set_out_eigenstate,  qb::op::QaoaWarmStart::help_out_eigenstates_ )
      .def_property("out_eigenstates", & qb::op::QaoaWarmStart::get_out_eigenstates, & qb::op::QaoaWarmStart::set_out_eigenstates, qb::op::QaoaWarmStart::help_out_eigenstates_ )
      .def_property("out_energy",  & qb::op::QaoaWarmStart::get_out_energys, & qb::op::QaoaWarmStart::set_out_energy,  qb::op::QaoaWarmStart::help_out_energys_ )
      .def_property("out_energys", & qb::op::QaoaWarmStart::get_out_energys, & qb::op::QaoaWarmStart::set_out_energys, qb::op::QaoaWarmStart::help_out_energys_ )
      .def_property("out_jacobian",  & qb::op::QaoaWarmStart::get_out_jacobians, & qb::op::QaoaWarmStart::set_out_jacobian,  qb::op::QaoaWarmStart::help_out_jacobians_ )
      .def_property("out_jacobians", & qb::op::QaoaWarmStart::get_out_jacobians, & qb::op::QaoaWarmStart::set_out_jacobians, qb::op::QaoaWarmStart::help_out_jacobians_ )
      .def_property("out_theta",  & qb::op::QaoaWarmStart::get_out_thetas, & qb::op::QaoaWarmStart::set_out_theta,  qb::op::QaoaWarmStart::help_out_thetas_ )
      .def_property("out_thetas", & qb::op::QaoaWarmStart::get_out_thetas, & qb::op::QaoaWarmStart::set_out_thetas, qb::op::QaoaWarmStart::help_out_thetas_ )
      .def_property("out_quantum_energy_calc_time",  & qb::op::QaoaWarmStart::get_out_quantum_energy_calc_times, & qb::op::QaoaWarmStart::set_out_quantum_energy_calc_time,  qb::op::QaoaWarmStart::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_energy_calc_times", & qb::op::QaoaWarmStart::get_out_quantum_energy_calc_times, & qb::op::QaoaWarmStart::set_out_quantum_energy_calc_times, qb::op::QaoaWarmStart::help_out_quantum_energy_calc_times_ )
      .def_property("out_quantum_jacobian_calc_time",  & qb::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times, & qb::op::QaoaWarmStart::set_out_quantum_jacobian_calc_time,  qb::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_quantum_jacobian_calc_times", & qb::op::QaoaWarmStart::get_out_quantum_jacobian_calc_times, & qb::op::QaoaWarmStart::set_out_quantum_jacobian_calc_times, qb::op::QaoaWarmStart::help_out_quantum_jacobian_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_time",  & qb::op::QaoaWarmStart::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaWarmStart::set_out_classical_energy_jacobian_total_calc_time,  qb::op::QaoaWarmStart::help_out_classical_energy_jacobian_total_calc_times_ )
      .def_property("out_classical_energy_jacobian_total_calc_times", & qb::op::QaoaWarmStart::get_out_classical_energy_jacobian_total_calc_times, & qb::op::QaoaWarmStart::set_out_classical_energy_jacobian_total_calc_times, qb::op::QaoaWarmStart::help_out_classical_energy_jacobian_total_calc_times_ )
      
      .def("run", py::overload_cast<>(& qb::op::QaoaBase::run), "Execute all declared experiments under all conditions")
      .def("runit", py::overload_cast<const size_t &, const size_t &>(& qb::op::QaoaWarmStart::run), "runit(i,j) : Execute ansatz i, condition j")

      .def("__repr__", & qb::op::QaoaWarmStart::get_summary,  "Print summary of qbos_op_qaoa settings")
      ;
}
