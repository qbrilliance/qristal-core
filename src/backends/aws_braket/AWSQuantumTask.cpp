/// Copyright (c) Quantum Brilliance Pty Ltd

#include <iostream>
#include <thread>
#include "qristal/core/backends/aws_braket/AWSQuantumTask.hpp"
#include "qristal/core/thread_pool.hpp"

namespace qristal
{
  /// Constructor
  aws_async_job_handle::aws_async_job_handle(const pybind11::object& aws_task, const std::vector<int>& measure_bits)
      : py_aws_task(aws_task)
      , m_measure_bits(measure_bits)
  {
  }

  /// Destructor
  aws_async_job_handle::~aws_async_job_handle()
  {
    // Graciously handle any Python exceptions raised during object destruction with try/catch.
    try
    {
      // Safely release the Python Braket Task object
      py_aws_task.release();
    }
    catch (const std::exception& ex)
    {
      // Logs any exceptions raised:
      std::cout << "Exception raised when releasing AWS QuantumTask Python object: " << ex.what() << std::endl;
    }
    catch (...)
    {
      std::cout << "Unknown exception raised when releasing AWS QuantumTask Python object.\n";
    }
  }

  /// Cancel the AWS Braket quantum task.
  void aws_async_job_handle::cancel()
  {
    // Call the 'cancel' method of Python AWSQuantumTask
    py_aws_task.attr("cancel")();
  }

  /// Return true if the Braket task is done.
  bool aws_async_job_handle::done()
  {
    // Check AWS status string for completed task
    const std::string task_status = py_aws_task.attr("state")().cast<std::string>();
    // Once completed, run all registered callbacks.
    if (task_status == "COMPLETED")
    {
      // The remote task has been completed, post-process the results according to registered callbacks.
      if (!m_fut.valid())
      {
        // Launch callbacks on a thread from the thread pool.
        m_fut = thread_pool::submit(
            [&]
            {
              // Acquire the GIL
              pybind11::gil_scoped_acquire acquire;
              for (auto& cb : m_done_cbs)
              {
                cb(*this);
              }
              return 0;
            });
      }

      // Return the status of the task (which is just the status of the post-processing callbacks).
      return m_fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
      // Once std::future::is_ready enters a version of the C++ standard that we support, this line should be replaced with
      // return m_fut.is_ready();
    }
    else
    {
      return false;
    }
  }

  /// Helper to convert Python measurement count, as Dict[String, int] to C++
  /// unordered_map
  std::unordered_map<std::string, int> aws_async_job_handle::py_measurement_count_to_cpp_map(const pybind11::dict& py_dict)
  {
    std::unordered_map<std::string, int> count_map;
    for (auto item : py_dict)
    {
      count_map[item.first.cast<std::string>()] = item.second.cast<int>();
    }
    return count_map;
  }

  /// Retrieve the result measurement distribution from the task.
  /// Blocking if the task is not completed.
  std::unordered_map<std::string, int> aws_async_job_handle::result()
  {
    pybind11::dict measurement_counts = py_aws_task.attr("result")().attr("measurement_counts");
    return py_measurement_count_to_cpp_map(measurement_counts);
  }

  /// Load async results to the buffer.
  void aws_async_job_handle::load_result(std::shared_ptr<xacc::AcceleratorBuffer> buffer)
  {
    const auto count_map = this->result();
    for (const auto& [bitStr, count] : count_map)
    {
      buffer->appendMeasurement(bitStr, count);
    }
    const auto marginal_counts = buffer->getMarginalCounts(m_measure_bits, xacc::AcceleratorBuffer::BitOrder::LSB);
    buffer->clearMeasurements();
    for (const auto& [bitStr, count] : marginal_counts)
    {
      buffer->appendMeasurement(bitStr, count);
    }
  }

  /// Adding a callback to be executed when the job is completed.
  void aws_async_job_handle::add_done_callback(std::function<void(async_job_handle&)> cb)
  {
    m_done_cbs.emplace_back(std::move(cb));
  }

  /// Blocking wait till the job is completed.
  void aws_async_job_handle::wait_for_completion(int poll_interval_ms)
  {
    while (!this->done())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
    }
  }
}
