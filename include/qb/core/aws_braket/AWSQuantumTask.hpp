/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "qb/core/remote_async_accelerator.hpp"
#include <string>
#include <unordered_map>
#include <future>
#include <pybind11/pybind11.h>
#include <AcceleratorBuffer.hpp>
namespace qb
{
  /// An awaitable class for interop with Python AWSQuantumTask
  /// This will get upcast to the base async_job_handle as the return type of AWS Accelerator implementation of
  /// async_execute().
  // Note: This class contains a Pybind11 object hence set to hidden to prevent compiler warnings.
  class __attribute__((visibility("hidden"))) aws_async_job_handle : public async_job_handle
  {
  public:
    /// Constructor
    aws_async_job_handle(const pybind11::object& aws_task, const std::vector<int>& measure_bits);

    /// Destructor
    ~aws_async_job_handle();

    /// Cancel the AWS Braket quantum task.
    virtual void cancel() override;

    /// Return true if the Braket task is done.
    virtual bool done() override;

    /// Load async results to the buffer.
    virtual void load_result(std::shared_ptr<xacc::AcceleratorBuffer> buffer) override;

    /// Adding a callback to be executed on the job is completed.
    virtual void add_done_callback(std::function<void(async_job_handle&)> cb) override;

    /// Blocking wait till the job is completed.
    virtual void wait_for_completion(int poll_interval_ms) override;

    /// Retrieve the result measurement distribution from the task.
    /// Blocking if the task is not completed.
    std::unordered_map<std::string, int> result();

  private:
    /// Helper to convert Python measurement count, as Dict[String, int] to C++
    /// unordered_map
    static std::unordered_map<std::string, int> py_measurement_count_to_cpp_map(const pybind11::dict& py_dict);

  private:
    /// The underlying AWS quantum task
    pybind11::object py_aws_task;

    /// The list of qubits that are measured in this AWS task.
    /// AWS Braket always performs measure all, hence we need to post-process the result based on the list of measured
    /// qubits in the circuit.
    std::vector<int> m_measure_bits;

    /// Done callbacks
    std::vector<std::function<void(async_job_handle&)>> m_done_cbs;

    /// C++ future indicating the AWS task has fully completed,
    /// including all the execution of callbacks.
    /// Note: when the remote result is available, we still need to execute
    /// post-processing tasks (on a thread from the thread pool) to parse the
    /// data into standard QB SDK data structures.
    std::future<int> m_fut;
  };
} // namespace qb
