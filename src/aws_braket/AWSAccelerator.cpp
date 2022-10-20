#include "qb/core/aws_braket/AWSAccelerator.hpp"
#include "qb/core/cmake_variables.hpp"
#include "qb/core/aws_braket/AWSOpenQasm3Visitor.hpp"
#include "qb/core/aws_braket/AWSQuantumTask.hpp"
#include "xacc_plugin.hpp"
#include <AcceleratorBuffer.hpp>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace xacc
{
  namespace quantum
  {

    /// Constructor
    AWSAccelerator::AWSAccelerator(bool debug) :
     debug_aws_(debug),
     m_device("DM1"),
     m_s3("amazon-braket-qbos"),
     m_path("output"),
     m_format("openqasm3"),
     m_noise(false),
     m_verbatim(false),
     m_shots(256)
    {}

    /// Return the name of the accelerator
    const std::string AWSAccelerator::name() const { return "aws_acc"; }

    /// Return the descriptions of the accelerator
    const std::string AWSAccelerator::description() const
    {
      return "The AWS Accelerator allows qbOS to offload to hosted AWS "
             "Simulators and hardware QPUs.";
    }

    /// Return the configuration keys of the accelerator
    const std::vector<std::string> AWSAccelerator::configurationKeys()
    {
      return {"device", "format", "s3", "path", "shots", "noise", "verbatim"};
    }

    /// Helper to traverse the input circuit IR and generate AWS string and list of measured qubits
    std::pair<std::string, std::vector<int>> AWSAccelerator::generate_aws_string(
        std::shared_ptr<CompositeInstruction> CompositeInstruction) const
    {
      // Convert XACC IR into AWS Braket compatible format (OPENQASM 3)
      xacc::info("Input Composite:\n" + CompositeInstruction->toString() + "\n");
      std::vector<int> measure_bits;
      std::string aws_str;
      auto make_measurements = [&](auto& visitor)
      {
        xacc::InstructionIterator it(CompositeInstruction);
        if (debug_aws_) std::cout << "# InstructionIterator created" << "\n";
        while (it.hasNext())
        {
          auto nextInst = it.next();
          if (nextInst->isEnabled())
          {
            nextInst->accept(&visitor);
            if (nextInst->name() == "Measure")
            {
              assert(!nextInst->bits().empty());
              measure_bits.emplace_back(nextInst->bits()[0]);
            }
          }
        }
      };
      if (m_format == "openqasm3")
      {
        qbOS::AWSOpenQASM3Visitor visitor(CompositeInstruction->nPhysicalBits(), m_noise, m_verbatim);
        make_measurements(visitor);
        aws_str = visitor.getOpenQasm();
      }
      else
      {
        AWSVisitor visitor(CompositeInstruction->nPhysicalBits(), false, m_verbatim);
        make_measurements(visitor);
        aws_str = visitor.getFinishedOpenQasmQpu();
      }
      xacc::info("AWS string:\n" + aws_str + "\n");
      if (debug_aws_) std::cout << "# AWS string: " << aws_str << "\n";
      return std::make_pair(aws_str, measure_bits);
    }

    /// Set up Python path to import Python wrapper scripts for Braket offloading
    void AWSAccelerator::setup_python_path()
    {
      if (debug_aws_) std::cout << "# Importing Python sys" << "\n";
      py::module_ sys = py::module_::import("sys");
      if (debug_aws_) std::cout << "# Importing Python path" << "\n";
      auto path = sys.attr("path");
      if (debug_aws_) std::cout << "# Inserting " << SDK_DIR << " into path" << "\n";
      path.attr("insert")(0, SDK_DIR);
    }


    /// Helper to post-process and save measurement results to the buffer.
    void AWSAccelerator::save_distribution_to_buffer(std::shared_ptr<AcceleratorBuffer> buffer,
                                                     const std::vector<int>& measure_bits,
                                                     const std::unordered_map<std::string, int>& count_map) const
    {
      if (!count_map.empty() && measure_bits.size() == count_map.begin()->first.length())
      {
        // Measure all qubits
        for (const auto &[bitStr, count] : count_map)
          buffer->appendMeasurement(bitStr, count);
      }
      else
      {
        // Measure a subset of qubits
        for (const auto &[bitStr, count] : count_map)
          buffer->appendMeasurement(bitStr, count);
        const auto marginal_counts = buffer->getMarginalCounts(
            measure_bits, xacc::AcceleratorBuffer::BitOrder::LSB);
        buffer->clearMeasurements();
        for (const auto &[bitStr, count] : marginal_counts)
          buffer->appendMeasurement(bitStr, count);
      }
    }

    /// Proceed to offload to AWS Braket and retrieve resultant counts
    void AWSAccelerator::execute(
     std::shared_ptr<AcceleratorBuffer> buffer,
     const std::shared_ptr<CompositeInstruction> CompositeInstruction)
    {
      // Process the IR to generate AWS string and list of measure qubits
      const auto [aws_str, measure_bits] = generate_aws_string(CompositeInstruction);

      if (m_format == "openqasm3")
      {
        // AWS OPENQASM 3 format with noise pragma
        // This is only supported by the DM1 backend.
        try
        {
          // Acquire the GIL
          py::gil_scoped_acquire acquire;
          if (debug_aws_)
            std::cout << "# Acquired GIL"
                      << "\n";
          // Set up PYTHONPATH to import aws_python_script
          setup_python_path();
          if (debug_aws_)
            std::cout << "# Importing aws_python_script"
                      << "\n";
          // Import aws_python_script as a Python module
          py::module_ qb_aws_mod = py::module_::import("aws_python_script");
          if (debug_aws_)
            std::cout << "# Binding qb_aws_mod_run_aws_braket"
                      << "\n";

          pybind11::function qb_aws_mod_run_aws_braket = qb_aws_mod.attr("run_aws_braket");
          // Run the circuit: blocking mode (polling till the result is available)
          if (debug_aws_)
            std::cout << "# About to run AWS Braket"
                      << "\n";
          /// This will post the job and wait (via polling) until the result is available.
          auto aws_result = qb_aws_mod_run_aws_braket(m_device, m_shots, aws_str, m_verbatim, m_format, m_s3, m_path);
          if (debug_aws_)
            std::cout << "# Ran AWS Braket"
                      << "\n";

          auto count_map = aws_result.cast<std::unordered_map<std::string, int>>();
          // Store the results to the buffer.
          save_distribution_to_buffer(buffer, measure_bits, count_map);
        }
        catch (const std::exception& ex)
        {
          std::cout << "Exception raised: " << ex.what() << std::endl;
          xacc::error("Failed to run AWS Braket");
        }
        catch (...)
        {
          xacc::error("Failed to run AWS Braket");
        }
      }
      else
      {
        xacc::error("Format not supported.  Please use: openqasm3");
      }
      if (debug_aws_) std::cout << "# Done executing AWS Braket!" << "\n";
    }

    /// Asynchronous offload a circuit to AWS Braket.
    /// Returns a handle to check job status and retrieve the result.
    std::shared_ptr<qbOS::async_job_handle> AWSAccelerator::async_execute(
        const std::shared_ptr<CompositeInstruction> CompositeInstruction) {
      // Process the IR to generate AWS string and list of measure qubits
      const auto [aws_str, measure_bits] = generate_aws_string(CompositeInstruction);
      if (m_format == "openqasm3") {
        // AWS OPENQASM 3 format with noise pragma
        // This is only supported by the DM1 backend.
        try {
          // Acquire the GIL
          py::gil_scoped_acquire acquire;
          if (debug_aws_)
            std::cout << "# Acquired GIL"
                      << "\n";
          // Set up PYTHONPATH to import aws_python_script
          setup_python_path();
          if (debug_aws_)
            std::cout << "# Importing aws_python_script"
                      << "\n";
          // Import aws_python_script as a Python module
          py::module_ qb_aws_mod = py::module_::import("aws_python_script");
          if (debug_aws_)
            std::cout << "# Binding qb_aws_mod_run_aws_braket_async"
                      << "\n";
          // Retrieve the async offloading function
          pybind11::function qb_aws_mod_run_aws_braket_async =
              qb_aws_mod.attr("run_aws_braket_async");

          // Asynchronous job submission: post the job and return.
          if (debug_aws_)
            std::cout << "# About to submit asynchronous job to AWS Braket"
                      << "\n";
          /// Post the job to AWS Braket and retrieve the task handle.
          auto py_aws_quantum_task = qb_aws_mod_run_aws_braket_async(
              m_device, m_shots, aws_str, m_verbatim, m_format, m_s3, m_path);
          auto aws_task_handle = std::make_shared<qbOS::aws_async_job_handle>(
              py_aws_quantum_task, measure_bits);
          if (debug_aws_)
            std::cout << "# Done submitting an asynchronous task to AWS Braket!"
                      << "\n";

          // This will get upcast to shared_ptr<async_job_handle> type on return.
          return aws_task_handle;
        }
        catch (const std::exception &ex) {
          std::cout << "Exception raised: " << ex.what() << std::endl;
          xacc::error("Failed to run AWS Braket");
        } catch (...) {
          xacc::error("Failed to run AWS Braket");
        }
      } else {
        xacc::error("Format not supported.  Please use: openqasm3");
      }

      throw std::runtime_error("Failed to submit an asynchronous task to AWS Braket!");
    }

    /// Proceed to offload multiple instructions to AWS Braket and retrieve resultant counts
    void AWSAccelerator::execute(
     std::shared_ptr<AcceleratorBuffer> buffer,
     const std::vector<std::shared_ptr<CompositeInstruction>> CompositeInstruction)
    {
      for (auto &f : CompositeInstruction)
      {
        auto tempBuffer = xacc::qalloc(buffer->size());
        execute(tempBuffer, f);
        buffer->appendChild(f->name(), tempBuffer);
      }
    }

    /// Initialise the accelerator's parameters and load its module into the Python interpreter
    void AWSAccelerator::initialize(const HeterogeneousMap &params)
    {
      if (params.stringExists("device"))        m_device = params.getString("device");
      if (params.stringExists("format"))        m_format = params.getString("format");
      if (params.stringExists("s3"))                m_s3 = params.getString("s3");
      if (params.stringExists("path"))            m_path = params.getString("path");
      if (params.keyExists<int>("shots"))        m_shots = params.get<int>("shots");
      if (params.keyExists<bool>("noise"))       m_noise = params.get<bool>("noise");
      if (params.keyExists<bool>("verbatim")) m_verbatim = params.get<bool>("verbatim");
      if (m_device == "Rigetti")  device_properties_json = queryRigettiHardwareProperties();
      if (debug_aws_) std::cout << "# Initialized AWSAccelerator" << "\n";

      // Import the aws_python_script for the first time on the main thread, as the AWS modules
      // that it imports cause deadlocks if they are imported for the first time on another thread.
      static bool first = true;
      if (first)
      {
        first = false;
        try
        {
          // Acquire the GIL
          py::gil_scoped_acquire acquire;
          if (debug_aws_) std::cout << "# Acquired GIL" << "\n";
          // Setup binding with aws_python_script.py
          if (debug_aws_) std::cout << "# Importing Python sys" << "\n";
          py::module_ sys = py::module_::import("sys");
          if (debug_aws_) std::cout << "# Importing Python path" << "\n";
          auto path = sys.attr("path");
          if (debug_aws_) std::cout << "# Inserting " << SDK_DIR << " into path" << "\n";
          path.attr("insert")(0, SDK_DIR);
          if (debug_aws_) std::cout << "# Importing aws_python_script" << "\n";
          py::module_ qb_aws_mod = py::module_::import("aws_python_script");
        }
        catch (const std::exception& ex)
        {
          std::cout << "Exception raised: " << ex.what() << std::endl;
          xacc::error("Failed to initialise AWS Braket");
        }
        catch (...)
        {
          xacc::error("Failed to initialise AWS Braket");
        }
      }
    }

    /// Re-initialise the accelerator's parameters
    void AWSAccelerator::updateConfiguration(const HeterogeneousMap &config)
    {
      initialize(config);
    }

    /// Retrieve the accelerator's parameters
    HeterogeneousMap AWSAccelerator::getProperties()
    {
      HeterogeneousMap m;
      m.insert("m_device", m_device);
      m.insert("m_format", m_format);
      m.insert("m_s3", m_s3);
      m.insert("m_path", m_path);
      m.insert("m_noise", m_noise);
      m.insert("m_verbatim", m_verbatim);
      m.insert("m_shots", m_shots);
      m.insert("device_properties", device_properties_json);
      return m;
    }

    /// Clone the accelerator
    // TODO: this does not seem to clone the current accelerator at all
    std::shared_ptr<xacc::Accelerator> AWSAccelerator::clone()
    {
      return std::make_shared<xacc::quantum::AWSAccelerator>();
    }

    /// Retrieve properties from Rigetti hardware on AWS
    std::string AWSAccelerator::queryRigettiHardwareProperties() const
    {
      py::gil_scoped_acquire acquire;
      static const std::string RIGETTI_DEVICE_ARN =
          "arn:aws:braket:us-west-1::device/qpu/rigetti/Aspen-M-2";
      assert(m_device == "Rigetti");
      auto AwsDevice = py::module_::import("braket.aws").attr("AwsDevice");
      auto AwsSession =
          py::module_::import("braket.aws.aws_session").attr("AwsSession");
      auto device_region = AwsDevice.attr("get_device_region")(RIGETTI_DEVICE_ARN);
      auto region_session = AwsSession().attr("copy_session")(device_region);
      auto metadata = region_session.attr("get_device")(RIGETTI_DEVICE_ARN);
      const std::string qpu_properties =
          metadata.attr("get")("deviceCapabilities").cast<std::string>();
      // std::cout << "Properties JSON:\n" << qpu_properties << "\n";
      return qpu_properties;
    }

  } // namespace quantum
} // namespace xacc

REGISTER_ACCELERATOR(xacc::quantum::AWSAccelerator)
