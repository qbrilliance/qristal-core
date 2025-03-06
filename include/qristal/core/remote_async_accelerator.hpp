// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include "Accelerator.hpp"
#include <memory>

/// Define generic/abstract interface for remotely-hosted accelerators
/// whereby job submission and result retrieval occur asynchronously
namespace qristal
{
  /// Abstract interface for async job offload,
  /// whereby the result is available at a later point for loading into the AcceleratorBuffer.
  struct async_job_handle
  {
    /// Default poll interval (in milliseconds) for blocking wait for completion.
    static constexpr int DEFAULT_RESULTS_POLL_INTERVAL_MS = 100;
    /// Cancel the async. task.
    virtual void cancel() = 0;

    /// Return true if async. is done.
    virtual bool done() = 0;

    /// Blocking wait for completion
    virtual void wait_for_completion(int poll_interval_ms = DEFAULT_RESULTS_POLL_INTERVAL_MS) = 0;

    /// Load async results to the buffer.
    virtual void load_result(std::shared_ptr<xacc::AcceleratorBuffer> buffer) = 0;

    /// Add a callback function to be executed once this job is completed.
    virtual void add_done_callback(std::function<void(async_job_handle&)> cb) = 0;
  };

  /// Abstract interface for remote Accelerators that support async. (non-blocking) job offloading.
  class remote_accelerator : public xacc::Accelerator
  {
  public:
    virtual std::shared_ptr<qristal::async_job_handle> async_execute(
        const std::shared_ptr<xacc::CompositeInstruction> CompositeInstruction) = 0;
  };
}
