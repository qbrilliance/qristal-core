// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_job_handle.hpp"
#include "qb/core/remote_async_accelerator.hpp"
#include "qb/core/session.hpp"
#include "qb/core/thread_pool.hpp"
#include "py_stl_containers.hpp"

namespace qb {
/// Returns true if the job is completed.
bool JobHandle::complete() const {
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

std::string JobHandle::qpu_name() const { return m_qpuName; }

/// Post the (i, j) job asynchronously to be executed on the virtualized QPU
/// pool.
void JobHandle::post_async(qb::session &s, int i, int j) {
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
std::string JobHandle::get_async_result() {
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
void JobHandle::terminate() {
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
std::shared_ptr<JobHandle> JobHandle::getJobHandle(int i, int j) {
  const std::scoped_lock guard(g_mutex);
  auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(i, j));
  if (iter != JOB_HANDLE_REGISTRY.end()) {
    return iter->second;
  }
  return nullptr;
}

/// Add this to the JOB_HANDLE_REGISTRY
void JobHandle::addJobHandle() {
  const std::scoped_lock guard(g_mutex);
  JOB_HANDLE_REGISTRY[std::make_pair(m_i, m_j)] = shared_from_this();
}

/// Remove this from the JOB_HANDLE_REGISTRY
void JobHandle::removeJobHandle() {
  const std::scoped_lock guard(g_mutex);
  auto iter = JOB_HANDLE_REGISTRY.find(std::make_pair(m_i, m_j));
  if (iter != JOB_HANDLE_REGISTRY.end()) {
    JOB_HANDLE_REGISTRY.erase(iter);
  }
}

/// Asynchronously run this job.
// !IMPORTANT! This method will be called on a different thread (one from the
// thread pool).
std::string JobHandle::run_async_internal() {
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
}

void bind_job_handle(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qb::JobHandle, std::shared_ptr<qb::JobHandle>>(m, "Handle")
      .def(py::init<>())
      .def("complete", &qb::JobHandle::complete,
           "Check if the job execution is complete.")
      .def("qpu_name", &qb::JobHandle::qpu_name,
           "Get the name of the QPU accelerator that executed this job.")
      .def("get", &qb::JobHandle::get_async_result, "Get the job result.")
      .def("terminate", &qb::JobHandle::terminate,
           "Terminate the running job.");
}

} // namespace qb