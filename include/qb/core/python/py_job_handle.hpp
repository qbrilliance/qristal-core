
// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <future>
#include <map>
#include <memory>
#include <string>
#include <pybind11/pybind11.h>

namespace xacc {
class Accelerator;
}
namespace qb {
class session;
class async_job_handle;
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
  std::future<std::map<std::vector<bool>, int>> m_threadResult;
  /// Flag to indicate whether the execution thread is still running.
  /// For local simulators, this translates to the completion status of the job.
  bool m_thread_running = false;
  /// Row index to the job table
  int m_i;
  /// Column index to the job table
  int m_j;
  /// Name of the QPU that this job is assigned to.
  std::string m_qpuName;
  /// Non-owning pointer to the session
  // !Important!: Within this JobHandle, only thread-safe methods of the session
  // class should be called.
  qb::session *m_session;
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
  bool complete() const;

  std::string qpu_name() const;

  /// Post the (i, j) job asynchronously to be executed on the virtualized QPU
  /// pool.
  void post_async(qb::session &s, int i, int j);

  /// Retrieve the async execution result.
  /// Blocking if the job is not completed yet.
  std::map<std::vector<bool>, int> get_async_result();

  /// Terminate a job.
  void terminate();

  /// Retrieve the job handle for the (i,j) index.
  /// Return null if not found (e.g., not-yet posted or cancelled)
  static std::shared_ptr<JobHandle> getJobHandle(int i, int j);

private:
  /// Add this to the JOB_HANDLE_REGISTRY
  void addJobHandle();

  /// Remove this from the JOB_HANDLE_REGISTRY
  void removeJobHandle();

  /// Asynchronously run this job.
  // !IMPORTANT! This method will be called on a different thread (one from the
  // thread pool).
  std::map<std::vector<bool>, int> run_async_internal();
};

/// Bind JobHandle class to the Python API
void bind_job_handle(pybind11::module &m);
} // namespace qb
