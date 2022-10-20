/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#ifndef QUANTUM_GATE_ACCELERATORS_AWSACCELERATOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_AWSACCELERATOR_HPP_

#include "AWSVisitor.hpp"
#include "Accelerator.hpp"
#include "InstructionIterator.hpp"

#include <cstdint>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <string>
#include <vector>

#include "qb/core/remote_async_accelerator.hpp"

namespace xacc
{
  namespace quantum
  {

    /**
     * The AWSAccelerator provides an execute()/async_execute() implementation that maps XACC IR
     * to AWS Braket OPENQASM3
     */
    class AWSAccelerator : public qbOS::remote_accelerator, public xacc::Cloneable<xacc::Accelerator>
    {

      private:

        std::string m_device;   // AWS Braket hosted simulator or hosted hardware QPU to run circuits on.
        std::string m_format;
        std::string m_s3;       // Name of S3 Bucket that will store AWS Braket results.
        std::string m_path;     // Path inside S3 Bucket where AWS Braket results are kept.
        bool m_noise;
        bool m_verbatim;        // Verbatim mode on AWS Braket hardware QPUs (Rigetti)
        bool debug_aws_;

      protected:

        int m_shots;
        std::string device_properties_json;

      public:

        /// Constructor
        AWSAccelerator(bool debug = false);

        /// Destructor
        ~AWSAccelerator() {}

        /// Return the name of the accelerator
        const std::string name() const override;

        /// Return the descriptions of the accelerator
        const std::string description() const override;

        /// Return the configuration keys of the accelerator
        const std::vector<std::string> configurationKeys() override;

        /// Proceed to offload to AWS Braket and retrieve resultant counts
        /// This will wait (polling) until the result is available.
        virtual void execute(std::shared_ptr<AcceleratorBuffer> buffer,
                             const std::shared_ptr<CompositeInstruction>
                                 CompositeInstruction) override;

        /// Proceed to offload multiple instructions to AWS Braket and retrieve resultant counts
        /// This will wait (polling) until all the results are available.
        virtual void execute(std::shared_ptr<AcceleratorBuffer> buffer,
                             const std::vector<std::shared_ptr<CompositeInstruction>>
                                 CompositeInstruction) override;

        /// Asynchronous offload a quantum circuit to AWS Braket.
        virtual std::shared_ptr<qbOS::async_job_handle> async_execute(
            const std::shared_ptr<CompositeInstruction> CompositeInstruction) override;

        /// Initialise the accelerator's parameters and load its module into the Python interpreter
        virtual void initialize(const HeterogeneousMap&params = {}) override;

        /// Re-initialise the accelerator's parameters
        virtual void updateConfiguration(const HeterogeneousMap &config) override;

        /// Retrieve the accelerator's parameters
        HeterogeneousMap getProperties() override;

        /// Clone the accelerator
        virtual std::shared_ptr<xacc::Accelerator> clone() override;

        /// Retrieve properties from Rigetti hardware on AWS
        std::string queryRigettiHardwareProperties() const;

      private:
        /// Helper to traverse the input circuit IR and generate AWS string and list of measured qubits
        std::pair<std::string, std::vector<int>> generate_aws_string(
            std::shared_ptr<CompositeInstruction> CompositeInstruction) const;

        /// Set up Python path to import Python wrapper scripts for Braket offloading
        void setup_python_path();

        /// Helper to post-process and save measurement results to the buffer.
        void save_distribution_to_buffer(std::shared_ptr<AcceleratorBuffer> buffer,
                                         const std::vector<int>& measure_bits,
                                         const std::unordered_map<std::string, int>& count_map) const;
    };

  } // namespace quantum
} // namespace xacc
#endif
