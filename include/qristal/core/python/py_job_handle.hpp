
// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <qristal/core/session.hpp>

#include <future>
#include <map>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>

namespace xacc { class Accelerator; }

namespace qristal {

  // Forward declarations
  class async_job_handle;

  /// Python interop job handle for async. execution.
  /// Supports both asynchronous remote backends (e.g., AWS Braket) and
  /// asynchronous execution of local backends
  /// (1) Remote backends (fully async.) will release the thread
  /// (from threadpool) as soon as it finishes job submission. It returns a handle
  /// to check for completion. (2) Local simulator/emulator instances will run on
  /// a separate thread, i.e., the completion of thread execution indicates the
  /// job completion.
  class JobHandle : public std::enable_shared_from_this<JobHandle> {

    private:

      /// Results from virtualized local simulator running on a dedicated thread.
      std::future<const session::ResultsMapType&> m_threadResult;

      /// Flag to indicate whether the execution thread is still running.
      /// For local simulators, this translates to the completion status of the job.
      bool m_thread_running = false;

      /// Non-owning pointer to the session
      qristal::session* m_session;

      /// Async. job handle when the QPU is a remote Accelerator.
      /// Note: This will be null when the QPU is a local instance running on a
      /// dedicated thread.
      std::shared_ptr<async_job_handle> m_handle;

    public:

      /// Returns true if the job is completed.
      bool complete() const;

      /// Post the circuit execution job asynchronously to be executed on the virtualized QPU pool.
      void post_async(qristal::session& s);

      /// Retrieve the async execution result.
      /// Blocking if the job is not completed yet.
      const session::ResultsMapType& get_async_result();

      /// Terminate a job.
      void terminate();

    private:

      /// Asynchronously run this job.
      // !IMPORTANT! This method will be called on a different thread (one from the
      // thread pool).
      const session::ResultsMapType& run_async_internal();
  };

  /// Bind JobHandle class to the Python API
  void bind_job_handle(pybind11::module &m);

}
