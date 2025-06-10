// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/python/py_job_handle.hpp>
#include <qristal/core/python/py_stl_containers.hpp>
#include <qristal/core/remote_async_accelerator.hpp>
#include <qristal/core/thread_pool.hpp>

namespace qristal {
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

/// Submit the job to be executed asynchronously.
void JobHandle::post_async(qristal::session &s) {
  m_session = &s;
  m_thread_running = true;
  // Add a functor to the thread pool to run the job.
  m_threadResult = thread_pool::submit(&JobHandle::run_async_internal, this);
}

/// Retrieve the async execution result.
/// Blocking if the job is not completed yet.
const session::ResultsMapType& JobHandle::get_async_result() {
  // If this is a remote job, wait for its completion.
  if (m_handle) {
    m_handle->wait_for_completion();
    return m_session->results();
  }

  // If this is a local simulation, wait for the simulation to complete on a thread.
  return m_threadResult.get();
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
    m_session->cancel_run();
  }
}

/// Asynchronously run this job.
// !IMPORTANT! This method will be called on a different thread (one from the
// thread pool).
const session::ResultsMapType& JobHandle::run_async_internal() {
  pybind11::gil_scoped_acquire acquire;
  std::shared_ptr<async_job_handle> async_handle = m_session->run();
  pybind11::gil_scoped_release release;
  m_thread_running = false;
  // If run_async executed synchronously on this thead, the result is available now.
  if (not async_handle) return m_session->results();
  // If this is a remote accelerator, i.e., run_async returns a valid handle, cache it to m_handle.
  m_handle = async_handle;
  // Returns a dummy result, not yet completed.
  static session::ResultsMapType dummy;
  return dummy;
}

void bind_job_handle(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qristal::JobHandle, std::shared_ptr<qristal::JobHandle>>(m, "Handle")
      .def(py::init<>())
      .def("complete", &qristal::JobHandle::complete,
           "Check if the job execution is complete.")
      .def("get", &qristal::JobHandle::get_async_result, "Get the job result.")
      .def("terminate", &qristal::JobHandle::terminate,
           "Terminate the running job.");
}

}
