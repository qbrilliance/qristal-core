// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#include "qb/core/methods.hpp"
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
using namespace qbOS;

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

using OracleFuncPyType = std::function<qbOS::CircuitBuilder(
    int, int, py::array_t<int>, int, py::array_t<int>, py::array_t<int>)>;
using StatePrepFuncPyType = std::function<qbOS::CircuitBuilder(
    py::array_t<int>, py::array_t<int>, py::array_t<int>, py::array_t<int>, py::array_t<int>)>;

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

namespace qbOS {
/// Python interop job handle (exposing to qbOS Python) for async. execution.
/// Supports both true async. remote backends (e.g., AWS Braket) and threading-based local backends (e.g., multiple
/// instances of local accelerators).
/// (1) Remote backends (fully async.) will release the thread (from threadpool) as soon as it finishes job submission.
/// It returns a handle to check for completion.
/// (2) Local simulator/emulator instances will run on different threads,
/// i.e., the completion of thread execution indicates the job completion.
class JobHandle : public std::enable_shared_from_this<JobHandle> {
private:
  /// Results from virtualized local simulator running on a dedicated thread.
  std::future<std::string> m_threadResult;
  /// Flag to indicate whether the execution thread is still running.
  /// For local simulators, this translates to the completion status of the job.
  bool m_thread_running = false;
  /// Row index to the qbOS QBQE jon table
  int m_i;
  /// Column index to the qbOS QBQE jon table
  int m_j;
  /// Name of the QPU that this job is assigned to.
  std::string m_qpuName;
  /// Non-owning reference to the QBQE instance (Python's qbOS.core())
  // !Important!: Within this JobHandle, only thread-safe methods of QBQE should be called.
  qbOS::Qbqe *m_qpqe;
  /// Instance of the QPU/Accelerator from the pool that this job is assigned to.
  std::shared_ptr<xacc::Accelerator> m_qpu;
  /// Async. job handle when the QPU is a remote Accelerator.
  /// Note: This will be null when the QPU is a local instance running on a dedicated thread.
  std::shared_ptr<async_job_handle> m_handle;
  /// Static map of all job handles.
  static inline std::map<std::pair<int, int>, std::shared_ptr<JobHandle>>
      JOB_HANDLE_REGISTRY;
  /// Mutex to guard access to the JOB_HANDLE_REGISTRY map.
  static inline std::mutex g_mutex;

public:
  /// Returns true if the job is completed.
  bool complete() const
  {
    if (m_handle)
    {
      // For remote accelerator (e.g. AWS Braket), use the handle to query the job status.
      return m_handle->done();
    }
    else
    {
      // Otherwise, this job is running locally on a thread from the thread pool.
      // Returns the thread status.
      return !m_thread_running;
    }
  }

  std::string qpu_name() const { return m_qpuName; }

  /// Post the (i, j) job asynchronously to be executed on the virtualized QPU pool.
  void post_async(qbOS::Qbqe &qbqe, int i, int j) {
    m_qpqe = &qbqe;
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
  std::string get_async_result()
  {
    if (m_handle)
    {
      // If this is a remote job, wait for its completion.
      m_handle->wait_for_completion();
      return m_qpqe->get_out_raws().at(m_i).at(m_j);
    }
    else
    {
      /// If this is a local simulation, wait for the simulation (on a thread) to complete.
      auto result = m_threadResult.get();
      return result;
    }
  }

  /// Terminate a job.
  void terminate()
  {
    if (complete())
    {
      // Nothing to do if already completed.
      return;
    }

    if (m_handle)
    {
      // Cancel the remote job.
      // Note: a remote accelerator instance can have multiple jobs in-flight, so the job cancellation must be
      // associated with a job handle.
      m_handle->cancel();
    }
    else
    {
      // For local simulators, ask the accelerator to stop.
      if (m_qpu)
      {
        // Terminate the job if still running.
        m_qpu->cancel();
      }
    }

    // Remove the job handle from the list of in-flight jobs.
    removeJobHandle();
  }


  /// Retrieve the job handle for the (i,j) index.
  /// Return null if not found (e.g., not-yet posted or cancelled)
  static std::shared_ptr<JobHandle> getJobHandle(int i, int j)
  {
    const std::scoped_lock guard(g_mutex);
    auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(i, j));
    if (iter != JOB_HANDLE_REGISTRY.end())
    {
      return iter->second;
    }
    return nullptr;
  }

private:
  /// Add this to the JOB_HANDLE_REGISTRY
  void addJobHandle()
  {
    const std::scoped_lock guard(g_mutex);
    JOB_HANDLE_REGISTRY[std::make_pair(m_i, m_j)] = shared_from_this();
  }

  /// Remove this from the JOB_HANDLE_REGISTRY
  void removeJobHandle()
  {
    const std::scoped_lock guard(g_mutex);
    auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(m_i, m_j));
    if (iter != JOB_HANDLE_REGISTRY.end())
    {
      JOB_HANDLE_REGISTRY.erase(iter);
    }
  }


  /// Asynchronously run this job.
  // !IMPORTANT! This method will be called on a different thread (one from the thread pool).
  std::string run_async_internal()
  {
    m_qpu             = m_qpqe->get_executor().getNextAvailableQpu();
    auto async_handle = m_qpqe->run_async(m_i, m_j, m_qpu);
    m_qpuName         = m_qpu->name();
    m_thread_running  = false;
    m_qpqe->get_executor().release(std::move(m_qpu));
    /// If this is a remote accelerator, i.e., run_async returns a valid handle,
    /// cache it to m_handle.
    if (async_handle)
    {
      m_handle = async_handle;
      /// Returns a dummy result, not yet completed.
      return "";
    }
    else
    {
      /// If run_async executed synchronously on this thead, the result is available now.
      return m_qpqe->get_out_raws()[m_i][m_j];
    }
  };
};
} // namespace qbOS

/**
 * PYBIND11_MODULE(my-module-name, type_py::module_)
 **/
PYBIND11_MODULE(core, m) {
  m.doc() = "pybind11 for qbos";
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

  py::class_<qbOS::JobHandle, std::shared_ptr<qbOS::JobHandle>>(m, "Handle")
      .def(py::init<>())
      .def("complete", &qbOS::JobHandle::complete,
           "Check if the job execution is complete.")
      .def("qpu_name", &qbOS::JobHandle::qpu_name,
           "Get the name of the QPU accelerator that executed this job.")
      .def("get", &qbOS::JobHandle::get_async_result, "Get the job result.")
      .def("terminate", &qbOS::JobHandle::terminate,
           "Terminate the running job.");

  py::class_<qbOS::Qbqe>(m, "session")
      .def(py::init<const std::string &>())
      .def(py::init<const bool>())
      .def(py::init())
      .def_property(
          "name_p", &qbOS::Qbqe::getName,
          py::overload_cast<const std::string &>(&qbOS::Qbqe::setName))
      .def_property(
          "names_p", &qbOS::Qbqe::getName,
          py::overload_cast<const VectorString &>(&qbOS::Qbqe::setName))
      .def_property("infile", &qbOS::Qbqe::get_infiles, &qbOS::Qbqe::set_infile,
                    qbOS::Qbqe::help_infiles_)
      .def_property("infiles", &qbOS::Qbqe::get_infiles,
                    &qbOS::Qbqe::set_infiles, qbOS::Qbqe::help_infiles_)
      .def_property("instring", &qbOS::Qbqe::get_instrings,
                    &qbOS::Qbqe::set_instring, qbOS::Qbqe::help_instrings_)
      .def_property("instrings", &qbOS::Qbqe::get_instrings,
                    &qbOS::Qbqe::set_instrings, qbOS::Qbqe::help_instrings_)
      .def_property(
          "ir_target", [&](qbOS::Qbqe &qbqe) {
              std::vector<std::vector<qbOS::CircuitBuilder>> circuits;
              std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> instructions = qbqe.get_irtarget_ms();
              for (auto vec_instructions : instructions){
                  std::vector<qbOS::CircuitBuilder> vec;
                  for (auto instruction : vec_instructions) {
                      vec.push_back(qbOS::CircuitBuilder(instruction));
                  }
                  circuits.push_back(vec);
              }
              return circuits;
          },
          [&](qbOS::Qbqe &qbqe, qbOS::CircuitBuilder &circuit) {
            qbqe.set_irtarget_m(circuit.get());
          },
          qbOS::Qbqe::help_irtarget_ms_)
      .def_property(
          "ir_targets", [&](qbOS::Qbqe &qbqe) {
              std::vector<std::vector<qbOS::CircuitBuilder>> circuits;
              std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> instructions = qbqe.get_irtarget_ms();
              for (auto vec_instructions : instructions){
                  std::vector<qbOS::CircuitBuilder> vec;
                  for (auto instruction : vec_instructions){
                      vec.push_back(qbOS::CircuitBuilder(instruction));
                  }
                  circuits.push_back(vec);
              }
              return circuits;
          },
          [&](qbOS::Qbqe &qbqe,
              std::vector<std::vector<qbOS::CircuitBuilder>> &circuits) {
            std::vector<
                std::vector<std::shared_ptr<xacc::CompositeInstruction>>>
                circuits_get;
            for (std::vector<qbOS::CircuitBuilder> vec : circuits) {
              std::vector<std::shared_ptr<xacc::CompositeInstruction>> vec_get;
              for (qbOS::CircuitBuilder circuit : vec) {
                std::shared_ptr<xacc::CompositeInstruction> circuit_get =
                    circuit.get();
                vec_get.push_back(circuit_get);
              }
              circuits_get.push_back(vec_get);
            }
            qbqe.set_irtarget_ms(circuits_get);
          },
          qbOS::Qbqe::help_irtarget_ms_)
      .def_property("include_qb", &qbOS::Qbqe::get_include_qbs,
                    &qbOS::Qbqe::set_include_qb, qbOS::Qbqe::help_include_qbs_)
      .def_property("include_qbs", &qbOS::Qbqe::get_include_qbs,
                    &qbOS::Qbqe::set_include_qbs, qbOS::Qbqe::help_include_qbs_)
      .def_property("qpu_config", &qbOS::Qbqe::get_qpu_configs,
                    &qbOS::Qbqe::set_qpu_config, qbOS::Qbqe::help_qpu_configs_)
      .def_property("qpu_configs", &qbOS::Qbqe::get_qpu_configs,
                    &qbOS::Qbqe::set_qpu_configs, qbOS::Qbqe::help_qpu_configs_)
      .def_property("acc", &qbOS::Qbqe::get_accs, &qbOS::Qbqe::set_acc,
                    qbOS::Qbqe::help_accs_)
      .def_property("accs", &qbOS::Qbqe::get_accs, &qbOS::Qbqe::set_accs,
                    qbOS::Qbqe::help_accs_)
      .def_property("aws_verbatim", &qbOS::Qbqe::get_aws_verbatims, &qbOS::Qbqe::set_aws_verbatim, qbOS::Qbqe::help_aws_verbatims_)
      .def_property("aws_verbatims", &qbOS::Qbqe::get_aws_verbatims, &qbOS::Qbqe::set_aws_verbatims, qbOS::Qbqe::help_aws_verbatims_)
      .def_property("aws_format", &qbOS::Qbqe::get_aws_formats, &qbOS::Qbqe::set_aws_format, qbOS::Qbqe::help_aws_formats_)
      .def_property("aws_formats", &qbOS::Qbqe::get_aws_formats, &qbOS::Qbqe::set_aws_formats, qbOS::Qbqe::help_aws_formats_)
      .def_property("aws_device", &qbOS::Qbqe::get_aws_device_names, &qbOS::Qbqe::set_aws_device_name, qbOS::Qbqe::help_aws_device_names_)
      .def_property("aws_devices", &qbOS::Qbqe::get_aws_device_names, &qbOS::Qbqe::set_aws_device_names, qbOS::Qbqe::help_aws_device_names_)
      .def_property("aws_s3", &qbOS::Qbqe::get_aws_s3s, &qbOS::Qbqe::set_aws_s3, qbOS::Qbqe::help_aws_s3s_)
      .def_property("aws_s3s", &qbOS::Qbqe::get_aws_s3s, &qbOS::Qbqe::set_aws_s3s, qbOS::Qbqe::help_aws_s3s_)
      .def_property("aws_s3_path", &qbOS::Qbqe::get_aws_s3_paths, &qbOS::Qbqe::set_aws_s3_path, qbOS::Qbqe::help_aws_s3_paths_)
      .def_property("aws_s3_paths", &qbOS::Qbqe::get_aws_s3_paths, &qbOS::Qbqe::set_aws_s3_paths, qbOS::Qbqe::help_aws_s3_paths_)
      .def_property("aer_sim_type", &qbOS::Qbqe::get_aer_sim_types, &qbOS::Qbqe::set_aer_sim_type, qbOS::Qbqe::help_aer_sim_types_)
      .def_property("aer_sim_types", &qbOS::Qbqe::get_aer_sim_types, &qbOS::Qbqe::set_aer_sim_types, qbOS::Qbqe::help_aer_sim_types_)
      .def_property("random", &qbOS::Qbqe::get_randoms, &qbOS::Qbqe::set_random,
                    qbOS::Qbqe::help_randoms_)
      .def_property("randoms", &qbOS::Qbqe::get_randoms,
                    &qbOS::Qbqe::set_randoms, qbOS::Qbqe::help_randoms_)
      .def_property("xasm", &qbOS::Qbqe::get_xasms, &qbOS::Qbqe::set_xasm,
                    qbOS::Qbqe::help_xasms_)
      .def_property("xasms", &qbOS::Qbqe::get_xasms, &qbOS::Qbqe::set_xasms,
                    qbOS::Qbqe::help_xasms_)
      .def_property("quil1", &qbOS::Qbqe::get_quil1s, &qbOS::Qbqe::set_quil1,
                    qbOS::Qbqe::help_quil1s_)
      .def_property("quil1s", &qbOS::Qbqe::get_quil1s, &qbOS::Qbqe::set_quil1s,
                    qbOS::Qbqe::help_quil1s_)
      .def_property("noplacement", &qbOS::Qbqe::get_noplacements,
                    &qbOS::Qbqe::set_noplacement,
                    qbOS::Qbqe::help_noplacements_)
      .def_property("noplacements", &qbOS::Qbqe::get_noplacements,
                    &qbOS::Qbqe::set_noplacements,
                    qbOS::Qbqe::help_noplacements_)
      .def_property("placement", &qbOS::Qbqe::get_placements, &qbOS::Qbqe::set_placement, qbOS::Qbqe::help_placements_)
      .def_property("placements", &qbOS::Qbqe::get_placements, &qbOS::Qbqe::set_placements, qbOS::Qbqe::help_placements_)
      .def_property("nooptimise", &qbOS::Qbqe::get_nooptimises,
                    &qbOS::Qbqe::set_nooptimise, qbOS::Qbqe::help_nooptimises_)
      .def_property("nooptimises", &qbOS::Qbqe::get_nooptimises,
                    &qbOS::Qbqe::set_nooptimises, qbOS::Qbqe::help_nooptimises_)
      .def_property("nosim", &qbOS::Qbqe::get_nosims, &qbOS::Qbqe::set_nosim,
                    qbOS::Qbqe::help_nosims_)
      .def_property("nosims", &qbOS::Qbqe::get_nosims, &qbOS::Qbqe::set_nosims,
                    qbOS::Qbqe::help_nosims_)
      .def_property("noise", &qbOS::Qbqe::get_noises, &qbOS::Qbqe::set_noise,
                    qbOS::Qbqe::help_noises_)
      .def_property("noises", &qbOS::Qbqe::get_noises, &qbOS::Qbqe::set_noises,
                    qbOS::Qbqe::help_noises_)
      .def_property("noise_model", &qbOS::Qbqe::get_noise_models, &qbOS::Qbqe::set_noise_model, qbOS::Qbqe::help_noise_models_)
      .def_property("noise_models", &qbOS::Qbqe::get_noise_models, &qbOS::Qbqe::set_noise_models, qbOS::Qbqe::help_noise_models_)
      .def_property("noise_mitigation", &qbOS::Qbqe::get_noise_mitigations, &qbOS::Qbqe::set_noise_mitigation,
                    qbOS::Qbqe::help_noise_mitigations_)
      .def_property("noise_mitigations", &qbOS::Qbqe::get_noise_mitigations, &qbOS::Qbqe::set_noise_mitigations,
                    qbOS::Qbqe::help_noise_mitigations_)
      .def_property("notiming", &qbOS::Qbqe::get_notimings,
                    &qbOS::Qbqe::set_notiming, qbOS::Qbqe::help_notimings_)
      .def_property("notimings", &qbOS::Qbqe::get_notimings,
                    &qbOS::Qbqe::set_notimings, qbOS::Qbqe::help_notimings_)
      .def_property("output_oqm_enabled", &qbOS::Qbqe::get_output_oqm_enableds,
                    &qbOS::Qbqe::set_output_oqm_enabled,
                    qbOS::Qbqe::help_output_oqm_enableds_)
      .def_property("output_oqm_enableds", &qbOS::Qbqe::get_output_oqm_enableds,
                    &qbOS::Qbqe::set_output_oqm_enableds,
                    qbOS::Qbqe::help_output_oqm_enableds_)
      .def_property("log_enabled", &qbOS::Qbqe::get_log_enableds,
                    &qbOS::Qbqe::set_log_enabled,
                    qbOS::Qbqe::help_log_enableds_)
      .def_property("log_enableds", &qbOS::Qbqe::get_log_enableds,
                    &qbOS::Qbqe::set_log_enableds,
                    qbOS::Qbqe::help_log_enableds_)
      .def_property("qn", &qbOS::Qbqe::get_qns, &qbOS::Qbqe::set_qn,
                    qbOS::Qbqe::help_qns_)
      .def_property("qns", &qbOS::Qbqe::get_qns, &qbOS::Qbqe::set_qns,
                    qbOS::Qbqe::help_qns_)
      .def_property("rn", &qbOS::Qbqe::get_rns, &qbOS::Qbqe::set_rn,
                    qbOS::Qbqe::help_rns_)
      .def_property("rns", &qbOS::Qbqe::get_rns, &qbOS::Qbqe::set_rns,
                    qbOS::Qbqe::help_rns_)
      .def_property("sn", &qbOS::Qbqe::get_sns, &qbOS::Qbqe::set_sn,
                    qbOS::Qbqe::help_sns_)
      .def_property("sns", &qbOS::Qbqe::get_sns, &qbOS::Qbqe::set_sns,
                    qbOS::Qbqe::help_sns_)
      .def_property("beta", &qbOS::Qbqe::get_betas, &qbOS::Qbqe::set_beta,
                    qbOS::Qbqe::help_betas_)
      .def_property("betas", &qbOS::Qbqe::get_betas, &qbOS::Qbqe::set_betas,
                    qbOS::Qbqe::help_betas_)
      .def_property("theta", &qbOS::Qbqe::get_thetas, &qbOS::Qbqe::set_theta,
                    qbOS::Qbqe::help_thetas_)
      .def_property("thetas", &qbOS::Qbqe::get_thetas, &qbOS::Qbqe::set_thetas,
                    qbOS::Qbqe::help_thetas_)
      .def_property("svd_cutoff", &qbOS::Qbqe::get_svd_cutoffs,
                    &qbOS::Qbqe::set_svd_cutoff, qbOS::Qbqe::help_svd_cutoffs_)
      .def_property("svd_cutoffs", &qbOS::Qbqe::get_svd_cutoffs,
                    &qbOS::Qbqe::set_svd_cutoffs, qbOS::Qbqe::help_svd_cutoffs_)
      .def_property("max_bond_dimension", &qbOS::Qbqe::get_max_bond_dimensions,
                    &qbOS::Qbqe::set_max_bond_dimension,
                    qbOS::Qbqe::help_max_bond_dimensions_)
      .def_property("max_bond_dimensions", &qbOS::Qbqe::get_max_bond_dimensions,
                    &qbOS::Qbqe::set_max_bond_dimensions,
                    qbOS::Qbqe::help_max_bond_dimensions_)
      .def_property("output_amplitude", &qbOS::Qbqe::get_output_amplitudes,
                    &qbOS::Qbqe::set_output_amplitude,
                    qbOS::Qbqe::help_output_amplitudes_)
      .def_property("output_amplitudes", &qbOS::Qbqe::get_output_amplitudes,
                    &qbOS::Qbqe::set_output_amplitudes,
                    qbOS::Qbqe::help_output_amplitudes_)
      .def_property("out_raw", &qbOS::Qbqe::get_out_raws,
                    &qbOS::Qbqe::set_out_raw, qbOS::Qbqe::help_out_raws_)
      .def_property("out_raws", &qbOS::Qbqe::get_out_raws,
                    &qbOS::Qbqe::set_out_raws, qbOS::Qbqe::help_out_raws_)
      .def_property("out_count", &qbOS::Qbqe::get_out_counts,
                    &qbOS::Qbqe::set_out_count, qbOS::Qbqe::help_out_counts_)
      .def_property("out_counts", &qbOS::Qbqe::get_out_counts,
                    &qbOS::Qbqe::set_out_counts, qbOS::Qbqe::help_out_counts_)
      .def_property("out_divergence", &qbOS::Qbqe::get_out_divergences,
                    &qbOS::Qbqe::set_out_divergence,
                    qbOS::Qbqe::help_out_divergences_)
      .def_property("out_divergences", &qbOS::Qbqe::get_out_divergences,
                    &qbOS::Qbqe::set_out_divergences,
                    qbOS::Qbqe::help_out_divergences_)
      .def_property("out_transpiled_circuit",
                    &qbOS::Qbqe::get_out_transpiled_circuits,
                    &qbOS::Qbqe::set_out_transpiled_circuit,
                    qbOS::Qbqe::help_out_transpiled_circuits_)
      .def_property("out_transpiled_circuits",
                    &qbOS::Qbqe::get_out_transpiled_circuits,
                    &qbOS::Qbqe::set_out_transpiled_circuits,
                    qbOS::Qbqe::help_out_transpiled_circuits_)
      .def_property("out_qobj", &qbOS::Qbqe::get_out_qobjs,
                    &qbOS::Qbqe::set_out_qobj, qbOS::Qbqe::help_out_qobjs_)
      .def_property("out_qobjs", &qbOS::Qbqe::get_out_qobjs,
                    &qbOS::Qbqe::set_out_qobjs, qbOS::Qbqe::help_out_qobjs_)
      .def_property("out_qbjson", &qbOS::Qbqe::get_out_qbjsons,
                    &qbOS::Qbqe::set_out_qbjson, qbOS::Qbqe::help_out_qbjsons_)
      .def_property("out_qbjsons", &qbOS::Qbqe::get_out_qbjsons,
                    &qbOS::Qbqe::set_out_qbjsons, qbOS::Qbqe::help_out_qbjsons_)
      .def_property("out_single_qubit_gate_qty",
                    &qbOS::Qbqe::get_out_single_qubit_gate_qtys,
                    &qbOS::Qbqe::set_out_single_qubit_gate_qty,
                    qbOS::Qbqe::help_out_single_qubit_gate_qtys_)
      .def_property("out_single_qubit_gate_qtys",
                    &qbOS::Qbqe::get_out_single_qubit_gate_qtys,
                    &qbOS::Qbqe::set_out_single_qubit_gate_qtys,
                    qbOS::Qbqe::help_out_single_qubit_gate_qtys_)
      .def_property("out_double_qubit_gate_qty",
                    &qbOS::Qbqe::get_out_double_qubit_gate_qtys,
                    &qbOS::Qbqe::set_out_double_qubit_gate_qty,
                    qbOS::Qbqe::help_out_double_qubit_gate_qtys_)
      .def_property("out_double_qubit_gate_qtys",
                    &qbOS::Qbqe::get_out_double_qubit_gate_qtys,
                    &qbOS::Qbqe::set_out_double_qubit_gate_qtys,
                    qbOS::Qbqe::help_out_double_qubit_gate_qtys_)

      .def_property("out_total_init_maxgate_readout_time",
                    &qbOS::Qbqe::get_out_total_init_maxgate_readout_times,
                    &qbOS::Qbqe::set_out_total_init_maxgate_readout_time,
                    qbOS::Qbqe::help_out_total_init_maxgate_readout_times_)
      .def_property("out_total_init_maxgate_readout_times",
                    &qbOS::Qbqe::get_out_total_init_maxgate_readout_times,
                    &qbOS::Qbqe::set_out_total_init_maxgate_readout_times,
                    qbOS::Qbqe::help_out_total_init_maxgate_readout_times_)

      .def_property("out_z_op_expect", &qbOS::Qbqe::get_out_z_op_expects,
                    &qbOS::Qbqe::set_out_z_op_expect,
                    qbOS::Qbqe::help_out_z_op_expects_)
      .def_property("out_z_op_expects", &qbOS::Qbqe::get_out_z_op_expects,
                    &qbOS::Qbqe::set_out_z_op_expects,
                    qbOS::Qbqe::help_out_z_op_expects_)

      .def_property("debug", &qbOS::Qbqe::get_debug_qbqe,
                    &qbOS::Qbqe::set_debug_qbqe, qbOS::Qbqe::help_debug_qbqe_)

      .def_property("num_threads", [&](qbOS::Qbqe &qbqe) { return qbOS::thread_pool::get_num_threads(); },
                    [&](qbOS::Qbqe &qbqe, int i) { qbOS::thread_pool::set_num_threads(i); },
                    "num_threads: The number of threads in the qbOS thread pool")

      .def_property("seed", &qbOS::Qbqe::get_seeds, &qbOS::Qbqe::set_seed,
                    qbOS::Qbqe::help_seeds_)
      .def_property("seeds", &qbOS::Qbqe::get_seeds, &qbOS::Qbqe::set_seeds,
                    qbOS::Qbqe::help_seeds_)

      .def("__repr__", &qbOS::Qbqe::get_summary,
           "Print summary of qbqe settings")
      .def("run", py::overload_cast<>(&qbOS::Qbqe::run),
           "Execute all declared quantum circuits under all conditions")
      .def("runit",
           py::overload_cast<const size_t &, const size_t &>(&qbOS::Qbqe::run),
           "runit(i,j) : Execute circuit i, condition j")
      .def("divergence", py::overload_cast<>(&qbOS::Qbqe::get_jensen_shannon),
           "Calculate Jensen-Shannon divergence")
      .def("qb12", py::overload_cast<>(&qbOS::Qbqe::qb12),
           "Quantum Brilliance 12-qubit defaults")
      .def("aws32dm1", py::overload_cast<>(&qbOS::Qbqe::aws32dm1),
           "AWS Braket DM1, 32 async workers")
      .def("aws32sv1", py::overload_cast<>(&qbOS::Qbqe::aws32sv1),
           "AWS Braket SV1, 32 async workers")
      .def("aws8tn1", py::overload_cast<>(&qbOS::Qbqe::aws8tn1),
           "AWS Braket TN1, 8 async workers")
      .def("set_parallel_run_config", &qbOS::Qbqe::set_parallel_run_config,
           "Set the parallel execution configuration")
      .def(
          "run_async",
          [&](qbOS::Qbqe &qbqe, int i, int j) {
            auto handle = std::make_shared<qbOS::JobHandle>();
            // Allow accelerators to acquire the GIL for themselves from a different thread
            pybind11::gil_scoped_release release;
            handle->post_async(qbqe, i, j);
            return handle;
          },
          "run_async(i,j) : Launch the execution of circuit i, condition j "
          "asynchronously.")
      .def(
          "run_complete",
          [&](qbOS::Qbqe &qbqe, int i, int j) {
            auto handle = qbOS::JobHandle::getJobHandle(i, j);
            if (handle) {
              return handle->complete();
            } else {
              return true;
            }
          },
          "run_complete(i,j) : Check if the execution of circuit i, condition "
          "j has been completed.");

  // Overloaded C++

  // m.def("add_", & Qbqe::add, "A function which adds 2 numbers", py::arg("i")
  // = 0, py::arg("j") = 0); m.def("cx_p", & Qbqe::tcx, "Test complex doubles");
  // m.def("print_p", & Qbqe::print_kv<int,std::complex<double>>, "Overloaded
  // print method"); m.def("print_p", &
  // Qbqe::print_vector_kv<int,std::complex<double>>, "Overloaded print
  // method"); m.def("print_vector_p", py::overload_cast<const
  // std::map<int,std::complex<double>>     &>(& Qbqe::print_kv), "Use C++ to
  // output container contents", py::arg("elems")); m.def("print_vector_p",
  // py::overload_cast<const std::vector<std::map<int,std::complex<double>>>
  // &>(& Qbqe::print_vector_kv), "Use C++ to output container contents",
  // py::arg("elems"));

  // std::vector<std::vector<double>> v_one;
  // v_one.push_back({2.3,3.5});
  // v_one.push_back({9.3});
  // v_one.push_back({0.1,0.6,0.1,0.1});
  // py::object myname = py::cast(v_one);
  // m.attr("name_") = myname;

  py::class_<qbOS::CircuitBuilder>(m, "Circuit")
      .def(py::init())
      .def("print", &qbOS::CircuitBuilder::print,
           "Print the quantum circuit that has been built.")
      .def(
          "openqasm",
          [&](qbOS::CircuitBuilder &this_) {
            auto staq = xacc::getCompiler("staq");
            const auto openQASMSrc = staq->translate(this_.get());
            return openQASMSrc;
          },
          "Get the OpenQASM representation of the circuit.")
      .def("append", &qbOS::CircuitBuilder::append,
           "Append another quantum circuit to this circuit.")
      // Temporary interface to execute the circuit.
      // TODO: using qbqe `run` once the QE-382 is implemented
      .def(
          "execute",
          [&](qbOS::CircuitBuilder &this_, const std::string &QPU,
              int NUM_SHOTS, int NUM_QUBITS) {
            auto acc = xacc::getAccelerator(QPU, {{"shots", NUM_SHOTS}});
            if (NUM_QUBITS < 0) {
              NUM_QUBITS = this_.get()->nPhysicalBits();
            }
            auto buffer = xacc::qalloc(NUM_QUBITS);
            acc->execute(buffer, this_.get());
            return buffer->toString();
          },
          py::arg("QPU") = "qpp", py::arg("NUM_SHOTS") = 1024,
          py::arg("NUM_QUBITS") = -1, "Run the circuit")
      .def("h", &qbOS::CircuitBuilder::H, "Hadamard gate.")
      .def("x", &qbOS::CircuitBuilder::X, "Pauli-X gate.")
      .def("y", &qbOS::CircuitBuilder::Y, "Pauli-Y gate.")
      .def("z", &qbOS::CircuitBuilder::Z, "Pauli-Z gate.")
      .def("t", &qbOS::CircuitBuilder::T, "T gate.")
      .def("tdg", &qbOS::CircuitBuilder::Tdg, "Adjoint T gate.")
      .def("s", &qbOS::CircuitBuilder::S, "S gate.")
      .def("sdg", &qbOS::CircuitBuilder::Sdg, "Adjoint S gate.")
      .def("rx", &qbOS::CircuitBuilder::RX, "Rotation around X gate.")
      .def("ry", &qbOS::CircuitBuilder::RY, "Rotation around Y gate.")
      .def("rz", &qbOS::CircuitBuilder::RZ, "Rotation around Z gate.")
      .def("cnot", &qbOS::CircuitBuilder::CNOT, "CNOT gate.")
      .def(
          "mcx",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> ctrl_inds,
              int target_idx) {
            builder.MCX(py_array_to_std_vec(ctrl_inds), target_idx);
          },
          "Multi-controlled NOT gate.")
      .def(
          "ccx",
          [&](qbOS::CircuitBuilder &builder, int ctrl_idx1, int ctrl_idx2,
              int target_idx) {
            builder.MCX({ctrl_idx1, ctrl_idx2}, target_idx);
          },
          "CCNOT (Toffoli) gate.")
      .def("swap", &qbOS::CircuitBuilder::SWAP, "SWAP gate.")
      .def("cphase", &qbOS::CircuitBuilder::CPhase,
           "Controlled phase gate (CU1).")
      .def("cz", &qbOS::CircuitBuilder::CZ, "CZ gate.")
      .def("ch", &qbOS::CircuitBuilder::CH, "Controlled-Hadamard (CH) gate.")
      .def("u1", &qbOS::CircuitBuilder::U1, "U1 gate.")
      .def("u3", &qbOS::CircuitBuilder::U3, "U3 gate.")
      .def("measure", &qbOS::CircuitBuilder::Measure, "Measure a qubit.")
      .def("measure_all", &qbOS::CircuitBuilder::MeasureAll,
            py::arg("NUM_QUBITS") = -1,
           "Measure all qubits.")
      .def(
          "qft",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> inds) {
            builder.QFT(py_array_to_std_vec(inds));
          },
          py::arg("qubits"), "Quantum Fourier Transform.")
      .def(
          "iqft",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> inds) {
            builder.IQFT(py_array_to_std_vec(inds));
          },
          py::arg("qubits"), "Inverse Quantum Fourier Transform.")
      .def(
          "exponent",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_log,
              py::array_t<int> qubits_exponent, py::array_t<int> qubits_ancilla,
              int min_significance, bool is_LSB) {
            qbOS::Exponent build_exp;
            xacc::HeterogeneousMap map = {{"qubits_log",py_array_to_std_vec(qubits_log)}, {"min_significance", min_significance}, {"is_LSB", is_LSB}};
            if (qubits_exponent.size() > 0) {
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
          "Exponent base 2.")
      .def(
          "qpe",
          [&](qbOS::CircuitBuilder &builder, py::object &oracle, int precision,
              py::array_t<int> trial_qubits,
              py::array_t<int> evaluation_qubits) {
            qbOS::CircuitBuilder *casted =
                oracle.cast<qbOS::CircuitBuilder *>();
            assert(casted);
            builder.QPE(*casted, precision, py_array_to_std_vec(trial_qubits),
                        py_array_to_std_vec(evaluation_qubits));
          },
          py::arg("oracle"), py::arg("precision"),
          py::arg("trial_qubits") = py::array_t<int>(),
          py::arg("precision_qubits") = py::array_t<int>(),
          "Quantum Phase Estimation.")
      .def(
          "canonical_ae",
          [&](qbOS::CircuitBuilder &builder, py::object &state_prep,
              py::object &grover_op, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, bool no_state_prep) {
            qbOS::CircuitBuilder *casted_state_prep =
                state_prep.cast<qbOS::CircuitBuilder *>();
            assert(state_prep);

            qbOS::CircuitBuilder *casted_grover_op =
                grover_op.cast<qbOS::CircuitBuilder *>();
            assert(casted_grover_op);
            builder.CanonicalAmplitudeEstimation(
                *casted_state_prep, *casted_grover_op, precision,
                num_state_prep_qubits, num_trial_qubits,
                py_array_to_std_vec(precision_qubits),
                py_array_to_std_vec(trial_qubits), no_state_prep);
          },
          py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
          py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
          py::arg("precision_qubits") = py::array_t<int>(),
          py::arg("trial_qubits") = py::array_t<int>(), py::arg("no_state_prep") = false,
          "Construct Canonical Quantum Amplitude Estimation Circuit.")
      .def(
          "run_canonical_ae",
          [&](qbOS::CircuitBuilder &builder, py::object &state_prep,
              py::object &grover_op, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, py::str acc_name) {
            qbOS::CircuitBuilder *casted_state_prep =
                state_prep.cast<qbOS::CircuitBuilder *>();
            assert(state_prep);

            qbOS::CircuitBuilder *casted_grover_op =
                grover_op.cast<qbOS::CircuitBuilder *>();
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
          "post-processing.")
      .def(
          "amcu",
          [&](qbOS::CircuitBuilder &builder, py::object &U,
              py::array_t<int> qubits_control,
              py::array_t<int> qubits_ancilla) {
            qbOS::CircuitBuilder *casted_U =
                U.cast<qbOS::CircuitBuilder *>();

            return builder.MultiControlledUWithAncilla(
                *casted_U,
                py_array_to_std_vec(qubits_control),
                py_array_to_std_vec(qubits_ancilla));
          },
          py::arg("U"), py::arg("qubits_control"), py::arg("qubits_ancilla"),
          "Multi Controlled U With Ancilla")
      .def(
          "run_canonical_ae_with_oracle",
          [&](qbOS::CircuitBuilder &builder, py::object &state_prep,
              py::object &oracle, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, py::str acc_name) {
            qbOS::CircuitBuilder *casted_state_prep =
                state_prep.cast<qbOS::CircuitBuilder *>();
            assert(state_prep);

            qbOS::CircuitBuilder *casted_oracle =
                oracle.cast<qbOS::CircuitBuilder *>();
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
          py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
          "Execute Canonical Quantum Amplitude Estimation procedure for the "
          "oracle "
          "including "
          "post-processing.")
      .def(
          "run_MLQAE",
          [&](qbOS::CircuitBuilder &builder, py::object &state_prep,
              py::object &oracle,
              std::function<int(std::string, int)> is_in_good_subspace,
              py::array_t<int> score_qubits, int total_num_qubits, int num_runs,
              int shots, py::str acc_name) {
            qbOS::CircuitBuilder *casted_state_prep =
                state_prep.cast<qbOS::CircuitBuilder *>();
            assert(state_prep);

            qbOS::CircuitBuilder *casted_oracle =
                oracle.cast<qbOS::CircuitBuilder *>();
            assert(casted_oracle);
            return builder.RunMLAmplitudeEstimation(
                *casted_state_prep, *casted_oracle, is_in_good_subspace,
                py_array_to_std_vec(score_qubits), total_num_qubits, num_runs,
                shots, acc_name);
          },
          py::arg("state_prep"), py::arg("oracle"),
          py::arg("is_in_good_subspace"), py::arg("score_qubits"),
          py::arg("total_num_qubits"), py::arg("num_runs") = 4,
          py::arg("shots") = 100, py::arg("qpu") = "qpp", "MLQAE")
      .def(
          "amplitude_amplification",
          [&](qbOS::CircuitBuilder &builder, py::object &oracle,
              py::object &state_prep, int power) {
            qbOS::CircuitBuilder *oracle_casted =
                oracle.cast<qbOS::CircuitBuilder *>();
            assert(oracle_casted);
            qbOS::CircuitBuilder *state_prep_casted =
                state_prep.cast<qbOS::CircuitBuilder *>();
            assert(state_prep_casted);
            builder.AmplitudeAmplification(*oracle_casted, *state_prep_casted,
                                           power);
          },
          py::arg("oracle"), py::arg("state_prep"), py::arg("power") = 1,
          "Amplitude Amplification.")
      .def(
          "ripple_add",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> a,
              py::array_t<int> b, int c_in) {
            builder.RippleAdd(py_array_to_std_vec(a), py_array_to_std_vec(b),
                              c_in);
          },
          py::arg("a"), py::arg("b"), py::arg("carry_bit"),
          "Ripple-carry adder circuit. The first register is added to the "
          "second register. The number of qubits in the result register must "
          "be greater than "
          "that of the first register to hold the carry over bit.")
      .def(
          "comparator",
          [&](qbOS::CircuitBuilder &builder, int BestScore,
              int num_scoring_qubits, py::array_t<int> trial_score_qubits,
              int flag_qubit, py::array_t<int> best_score_qubits,
              py::array_t<int> ancilla_qubits, bool is_LSB,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
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
          py::arg("is_LSB") = true, py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(), "Comparator.")
      .def(
          "efficient_encoding",
          [&](qbOS::CircuitBuilder &builder,
              std::function<int(int)> scoring_function, int num_state_qubits,
              int num_scoring_qubits, py::array_t<int> state_qubits,
              py::array_t<int> scoring_qubits, bool is_LSB, bool use_ancilla,
              py::array_t<int> qubits_init_flags, int flag_integer) {
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
          py::arg("qubits_init_flags") = py::array_t<int>(), py::arg("flag_integer") = 0,
          "Efficient Encoding.")
      .def(
          "equality_checker",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, int flag, bool use_ancilla,
              py::array_t<int> qubits_ancilla, py::array_t<int> controls_on,
              py::array_t<int> controls_off) {
            builder.EqualityChecker(
                py_array_to_std_vec(qubits_a),
                py_array_to_std_vec(qubits_b), flag, use_ancilla, py_array_to_std_vec(qubits_ancilla),
                py_array_to_std_vec(controls_on), py_array_to_std_vec(controls_off));
          },
          py::arg("qubits_a"), py::arg("qubits_b"), py::arg("flag"),
          py::arg("use_ancilla") = false,
          py::arg("qubits_ancilla") = py::array_t<int>(), py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(), "Equality Checker.")
      .def(
          "controlled_swap",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_a,
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
          py::arg("flags_off") = py::array_t<int>(), "Controlled swap.")
      .def(
          "controlled_ripple_carry_adder",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_adder,
              py::array_t<int> qubits_sum, int c_in, py::array_t<int> flags_on,
              py::array_t<int> flags_off, bool no_overflow) {
            builder.ControlledAddition(
                py_array_to_std_vec(qubits_adder),
                py_array_to_std_vec(qubits_sum), c_in,
                py_array_to_std_vec(flags_on),
                py_array_to_std_vec(flags_off), no_overflow);
          },
          py::arg("qubits_adder"), py::arg("qubits_sum"), py::arg("c_in"),
          py::arg("flags_on") = py::array_t<int>(),
          py::arg("flags_off") = py::array_t<int>(), py::arg("no_overflow") = false,
          "Controlled ripple carry adder.")
      .def(
          "generalised_mcx",
          [&](qbOS::CircuitBuilder &builder, int target,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
            builder.GeneralisedMCX(target,
                py_array_to_std_vec(controls_on),
                py_array_to_std_vec(controls_off));
          },
          py::arg("target"), py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(), "Generalised MCX.")
      .def(
          "compare_beam_oracle",
          [&](qbOS::CircuitBuilder &builder, int q0, int q1, int q2,
              py::array_t<int> FA, py::array_t<int> FB, py::array_t<int> SA, py::array_t<int> SB,
              bool simplified) {
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
          py::arg("simplified") = true, "Compare beam oracle.")
      .def(
          "inverse_circuit",
          [&](qbOS::CircuitBuilder &builder, py::object &circ) {
            qbOS::CircuitBuilder *casted_circ =
                circ.cast<qbOS::CircuitBuilder *>();
            builder.InverseCircuit(*casted_circ);
          },
          py::arg("circ"), "Inverse circuit.")
      .def(
          "comparator_as_oracle",
          [&](qbOS::CircuitBuilder &builder, int BestScore,
              int num_scoring_qubits, py::array_t<int> trial_score_qubits,
              int flag_qubit, py::array_t<int> best_score_qubits,
              py::array_t<int> ancilla_qubits, bool is_LSB,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
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
          py::arg("is_LSB") = true, py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(), "Comparator as oracle.")
      .def(
          "multiplication",
          [&](qbOS::CircuitBuilder &builder,
              py::array_t<int> qubits_a, py::array_t<int> qubits_b,
              py::array_t<int> qubits_result, int qubit_ancilla, bool is_LSB) {
            builder.Multiplication(
                py_array_to_std_vec(qubits_a),
                py_array_to_std_vec(qubits_b),
                py_array_to_std_vec(qubits_result), qubit_ancilla, is_LSB);
          },
          py::arg("qubit_ancilla"),
          py::arg("qubits_a"), py::arg("qubits_b"),
          py::arg("qubits_result"), py::arg("is_LSB") = true,
          "Multiplication.")
      .def(
          "controlled_multiplication",
          [&](qbOS::CircuitBuilder &builder,
              py::array_t<int> qubits_a, py::array_t<int> qubits_b,
              py::array_t<int> qubits_result,
              int qubit_ancilla, bool is_LSB, py::array_t<int> controls_on, py::array_t<int> controls_off) {
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
          py::arg("controls_on") = py::array_t<int>(), py::arg("controls_off") = py::array_t<int>(),
          "Controlled Multiplication.")
      .def(
          "exponential_search",
          [&](qbOS::CircuitBuilder &builder, py::str method,
              OracleFuncPyType oracle_func, py::object state_prep,
              const std::function<int(int)> f_score, int best_score,
              py::array_t<int> qubits_string, py::array_t<int> qubits_metric,
              py::array_t<int> qubits_next_letter,
              py::array_t<int> qubits_next_metric, int flag_qubit,
              py::array_t<int> qubits_best_score,
              py::array_t<int> qubits_ancilla_oracle,
              py::array_t<int> qubits_ancilla_adder,
              py::array_t<int> total_metric, int CQAE_num_evaluation_qubits,
              std::function<int(std::string, int)> MLQAE_is_in_good_subspace,
              int MLQAE_num_runs, int MLQAE_num_shots, py::str acc_name) {
            std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
            StatePrepFuncPyType state_prep_casted;
            qbOS::OracleFuncCType oracle_converted =
                [&](int best_score, int num_scoring_qubits,
                    std::vector<int> trial_score_qubits, int flag_qubit,
                    std::vector<int> best_score_qubits,
                    std::vector<int> ancilla_qubits) {
                  // Do conversion
                  auto conv = oracle_func(
                      best_score, num_scoring_qubits,
                      std_vec_to_py_array(trial_score_qubits), flag_qubit,
                      std_vec_to_py_array(best_score_qubits),
                      std_vec_to_py_array(ancilla_qubits));
                  return conv.get();
                };

            qbOS::StatePrepFuncCType state_prep_func;
            try {
              qbOS::CircuitBuilder state_prep_casted =
                  state_prep.cast<qbOS::CircuitBuilder>();
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
                method, oracle_converted, state_prep_func, f_score, best_score,
                py_array_to_std_vec(qubits_string),
                py_array_to_std_vec(qubits_metric),
                py_array_to_std_vec(qubits_next_letter),
                py_array_to_std_vec(qubits_next_metric), flag_qubit,
                py_array_to_std_vec(qubits_best_score),
                py_array_to_std_vec(qubits_ancilla_oracle),
                py_array_to_std_vec(qubits_ancilla_adder),
                py_array_to_std_vec(total_metric),
                CQAE_num_evaluation_qubits, MLQAE_is_in_good_subspace,
                MLQAE_num_runs, MLQAE_num_shots, acc_name);
          },
          py::arg("method"), py::arg("oracle"), py::arg("state_prep"),
          py::arg("f_score"), py::arg("best_score"), py::arg("qubits_string"),
          py::arg("qubits_metric"), py::arg("qubits_next_letter"),
          py::arg("qubits_next_metric"), py::arg("qubit_flag"),
          py::arg("qubits_best_score"), py::arg("qubits_ancilla_oracle"),
          py::arg("qubits_ancilla_adder") = py::array_t<int>(),
          py::arg("total_metric") = py::array_t<int>(),
          py::arg("CQAE_num_evaluation_qubits") = 10,
          py::arg("MLQAE_is_in_good_subspace") = py::cpp_function(
              [](std::string str, int i) {
                // Only required for MLQAE
                return 0;
              },
              py::arg("str"), py::arg("val")),

          py::arg("MLQAE_num_runs") = 6, py::arg("MLQAE_num_shots") = 100,
          py::arg("qpu") = "qpp", "Exp Search")
      .def(
          "q_prime_unitary",
          [&](qbOS::CircuitBuilder &builder, int nb_qubits_ancilla_metric,
              int nb_qubits_ancilla_letter,
              int nb_qubits_next_letter_probabilities,
              int nb_qubits_next_letter) {
            builder.QPrime(nb_qubits_ancilla_metric, nb_qubits_ancilla_letter,
                           nb_qubits_next_letter_probabilities,
                           nb_qubits_next_letter);
          },
          py::arg("nb_qubits_ancilla_metric"),
          py::arg("nb_qubits_ancilla_letter"),
          py::arg("nb_qubits_next_letter_probabilities"),
          py::arg("nb_qubits_next_letter"), "QPrime.")
      .def(
          "inverse_circuit",
          [&](qbOS::CircuitBuilder &builder, py::object &circ) {
            qbOS::CircuitBuilder *casted_circ =
                circ.cast<qbOS::CircuitBuilder *>();
            builder.InverseCircuit(*casted_circ);
          },
          py::arg("circ"), "Inverse circuit.")
      .def(
          "subtraction",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_larger,
              py::array_t<int> qubits_smaller, bool is_LSB, int qubit_ancilla) {
            builder.Subtraction(py_array_to_std_vec(qubits_larger),
                                py_array_to_std_vec(qubits_smaller), is_LSB, qubit_ancilla);
          },
          py::arg("qubits_larger"), py::arg("qubits_smaller"),
          py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1, "Subtraction circuit.")
      .def(
          "controlled_subtraction",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_larger,
              py::array_t<int> qubits_smaller, py::array_t<int> controls_on,
              py::array_t<int> controls_off, bool is_LSB, int qubit_ancilla) {
            builder.ControlledSubtraction(py_array_to_std_vec(qubits_larger),
                                          py_array_to_std_vec(qubits_smaller),
                                          py_array_to_std_vec(controls_on),
                                          py_array_to_std_vec(controls_off),
                                          is_LSB, qubit_ancilla);
          },
          py::arg("qubits_larger"), py::arg("qubits_smaller"),
          py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1, "Controlled subtraction circuit.")
      .def(
          "proper_fraction_division",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_numerator,
              py::array_t<int> qubits_denominator,
              py::array_t<int> qubits_fraction, py::array_t<int> qubits_ancilla,
              bool is_LSB) {
            builder.ProperFractionDivision(
                py_array_to_std_vec(qubits_numerator),
                py_array_to_std_vec(qubits_denominator),
                py_array_to_std_vec(qubits_fraction),
                py_array_to_std_vec(qubits_ancilla), is_LSB);
          },
          py::arg("qubits_numerator"), py::arg("qubits_denominator"),
          py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
          py::arg("is_LSB") = true, "Proper fraction division circuit.")
      .def(
          "controlled_proper_fraction_division",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_numerator,
              py::array_t<int> qubits_denominator,
              py::array_t<int> qubits_fraction, py::array_t<int> qubits_ancilla,
              py::array_t<int> controls_on, py::array_t<int> controls_off,
              bool is_LSB) {
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
          "Controlled proper fraction division circuit.")
      .def(
          "compare_gt",
          [&](qbOS::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b,
              int qubit_flag, int qubit_ancilla,
              bool is_LSB) {
            builder.CompareGT(
                py_array_to_std_vec(qubits_a),
                py_array_to_std_vec(qubits_b),
                qubit_flag,
                qubit_ancilla, is_LSB);
          },
          py::arg("qubits_numerator"), py::arg("qubits_denominator"),
          py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
          py::arg("is_LSB") = true, "Proper fraction division circuit.");
  m.def(
          "run_canonical_ae",
          [&](py::object &state_prep, py::object &grover_op, int precision,
              int num_state_prep_qubits, int num_trial_qubits,
              py::array_t<int> precision_qubits, py::array_t<int> trial_qubits,
              py::str acc_name) {
            qbOS::CircuitBuilder builder;

        qbOS::CircuitBuilder *casted_state_prep =
            state_prep.cast<qbOS::CircuitBuilder *>();
        assert(state_prep);

        qbOS::CircuitBuilder *casted_grover_op =
            grover_op.cast<qbOS::CircuitBuilder *>();
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
        qbOS::CircuitBuilder builder;
        qbOS::CircuitBuilder *casted_state_prep =
            state_prep.cast<qbOS::CircuitBuilder *>();
        assert(state_prep);

        qbOS::CircuitBuilder *casted_oracle =
            oracle.cast<qbOS::CircuitBuilder *>();
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
        qbOS::CircuitBuilder builder;
        qbOS::CircuitBuilder *casted_state_prep =
            state_prep.cast<qbOS::CircuitBuilder *>();
        assert(state_prep);

        qbOS::CircuitBuilder *casted_oracle =
            oracle.cast<qbOS::CircuitBuilder *>();
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
      [&](py::str method, OracleFuncPyType oracle_func, py::object state_prep,
          const std::function<int(int)> f_score, int best_score,
          py::array_t<int> qubits_string, py::array_t<int> qubits_metric,
          py::array_t<int> qubits_next_letter,
          py::array_t<int> qubits_next_metric, int flag_qubit,
          py::array_t<int> qubits_best_score,
          py::array_t<int> qubits_ancilla_oracle,
          py::array_t<int> qubits_ancilla_adder,
          py::array_t<int> total_metric,
          int CQAE_num_evaluation_qubits,
          std::function<int(std::string, int)> MLQAE_is_in_good_subspace,
          int MLQAE_num_runs, int MLQAE_num_shots, py::str acc_name) {
        qbOS::CircuitBuilder builder;
        std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
        StatePrepFuncPyType state_prep_casted;
        qbOS::OracleFuncCType oracle_converted =
            [&](int best_score, int num_scoring_qubits,
                std::vector<int> trial_score_qubits, int flag_qubit,
                std::vector<int> best_score_qubits,
                std::vector<int> ancilla_qubits) {
              // Do conversion
              auto conv = oracle_func(best_score, num_scoring_qubits,
                                      std_vec_to_py_array(trial_score_qubits),
                                      flag_qubit,
                                      std_vec_to_py_array(best_score_qubits),
                                      std_vec_to_py_array(ancilla_qubits));
              return conv.get();
            };

        qbOS::StatePrepFuncCType state_prep_func;
        try {
          qbOS::CircuitBuilder state_prep_casted =
              state_prep.cast<qbOS::CircuitBuilder>();
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
            method, oracle_converted, state_prep_func, f_score, best_score,
            py_array_to_std_vec(qubits_string),
            py_array_to_std_vec(qubits_metric),
            py_array_to_std_vec(qubits_next_letter),
            py_array_to_std_vec(qubits_next_metric), flag_qubit,
            py_array_to_std_vec(qubits_best_score),
            py_array_to_std_vec(qubits_ancilla_oracle),
            py_array_to_std_vec(qubits_ancilla_adder),
            py_array_to_std_vec(total_metric),
            CQAE_num_evaluation_qubits, MLQAE_is_in_good_subspace,
            MLQAE_num_runs, MLQAE_num_shots, acc_name);
      },
      py::arg("method"), py::arg("oracle"), py::arg("state_prep"),
      py::arg("f_score"), py::arg("best_score"), py::arg("qubits_string"),
      py::arg("qubits_metric"), py::arg("qubits_next_letter"),
      py::arg("qubits_next_metric"), py::arg("qubit_flag"),
      py::arg("qubits_best_score"), py::arg("qubits_ancilla_oracle"),
      py::arg("qubits_ancilla_adder") = py::array_t<int>(),
      py::arg("total_metric") = py::array_t<int>(),
      py::arg("CQAE_num_evaluation_qubits") = 10,
      py::arg("MLQAE_is_in_good_subspace") = py::cpp_function(
          [](std::string str, int i) {
            // Only required for MLQAE
            return 0;
          },
          py::arg("str"), py::arg("val")),
      py::arg("MLQAE_num_runs") = 6, py::arg("MLQAE_num_shots") = 100,
      py::arg("qpu") = "qpp","Exp Search");

  py::add_ostream_redirect(m);
}
