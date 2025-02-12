// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// Qristal
#include "qristal/core/backends/qb_hardware/qb_visitor.hpp"
#include "qristal/core/session_utils.hpp"

// XACC
#include "RemoteAccelerator.hpp"
#include "InstructionIterator.hpp"

// STL
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <vector>


namespace xacc
{
  namespace quantum
  {

    /// @brief Provides an `execute` implementation that maps XACC IR to a suitable JSON message for execution on QB hardware.
    ///
    /// Execution is in two phases:
    /// 1. Circuit submission via HTTP POST to QB hardware device
    /// 2. Using the returned ID, form a HTTP GET request and poll repeatedly until results are returned
    ///
    class qb_qpu : virtual public RemoteAccelerator
    {

      public:

        /// Safe limit for QB hardware
        const int QB_SAFE_LIMIT_SHOTS = 512;

        /// Default constructor that just inits the parent class
        qb_qpu(const std::string name, const bool debug_flag = false) : RemoteAccelerator(), debug(debug_flag), qpu_name(name) {}

        /// Destructor
        virtual ~qb_qpu() {}

        /// @{ @brief Getters
        const std::string getSignature() override;
        const std::string name() const override;
        const std::string description() const override;
        const std::string get_qbjson() const;
        /// @}

        /// Indicate that this is indeed a remote XACC accelerator
        bool isRemote() override;

        /// Retrieve the properties of the backend
        HeterogeneousMap getProperties() override;

        /// Get the available configuration settings
        const std::vector<std::string> configurationKeys() override;

        /// @brief Change the configuration of QB hardware
        ///
        /// @param config Input the new configuration settings to be applied
        ///
        void updateConfiguration(const HeterogeneousMap &config) override;

        /// @brief Initialize the configuration of QB hardware
        ///
        /// @param params Input the new configuration settings to be applied
        ///
        void initialize(const HeterogeneousMap &params = {}) override;

        /// @brief Initialise the QB hardware (reserve, get native gateset, etc.)
        ///
        /// @param check_hardware_lifesigns Check for a response from the hardware
        ///
        void setup_hardware(bool check_hardware_lifesigns);

        /// @brief Submit the circuit with HTTP POST to QB hardware and poll for results with HTTP GET
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param functions Input circuit in XACC IR format
        /// @param execute_circuit Actually send the circuit to the hardware for execution
        ///
        void execute(std::shared_ptr<AcceleratorBuffer> buffer,
         const std::vector<std::shared_ptr<CompositeInstruction>> functions,
         bool execute_circuit);

        /// @brief Converts the circuit to a representation that QB hardware accepts
        ///
        /// Sets up QB specific metadata, visits XACC IR to construct JSON strings
        /// for the circuit and required measurements, then combines both into the
        /// HTTP POST request body.
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param functions Input circuit in XACC IR format
        ///
        const std::string processInput(
         std::shared_ptr<AcceleratorBuffer> buffer,
         std::vector<std::shared_ptr<CompositeInstruction>> functions) override;

        /// @brief Handle the response to the initial POST (circuit submission)
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param response Input the response body returned by the prior POST request
        ///
        void processResponse(std::shared_ptr<AcceleratorBuffer> buffer,
                             const std::string &response) override;

        /// @brief Poll QB hardware for circuit results
        std::map<std::string, int> poll_for_results();


      protected:

        bool debug;
        std::string qpu_name;

        /// Command
        std::string command = "circuit";

        /// Number of shots in a cycle
        int shots = 0;

        /// Poll seconds
        double poll_secs = 0;

        /// Number of qubits
        size_t n_qubits = 0;

        /// Init (vector of qubits, value is the initial state)
        std::vector<uint> init = {0, 0};

        /// Contrast thresholds
        bool use_default_contrast_settings = true;
        double init_contrast_threshold = 0;
        std::map<int,double> qubit_contrast_thresholds = {};

        /// Id number of last submitted circuit
        uint circuit_id;

        /// Format for results
        std::string results = "normal";

        /// HTTP poll retries allowed
        uint poll_retries = 0;

        /// Order of measurements
        std::vector<int> order_of_m = {};

        /// Assume exclusive use of the hardware device. If this flag is set true, the hardware
        /// will be assumed to only accept circuits accompanied by an appropriate token.
        bool exclusive_access;

        /// The encrypted JSON web token used to authenticate with a hardware device operating
        /// in exclusive access mode.
        std::string exclusive_access_token;

        /// The http header sent to the hardware
        std::map<std::string, std::string> http_header;

        /// The JSON string sent to the hardware
        std::string qbjson;

        /// HTTP POST, returning the HTTP status code
        std::string Post(
            const std::string& url,
            const std::string& path,
            const std::string& postStr,
            std::map<std::string, std::string> headers = {});

        /// HTTP GET, returning the HTTP status code
        std::string Get(
            const std::string& url,
            const std::string& path,
            std::map<std::string, std::string> headers = {},
            std::map<std::string, std::string> extraParams = {});

        /// HTTP PUT, returning the HTTP status code
        std::string Put(
            const std::string& url,
            const std::string& path,
            const std::string& putStr,
            std::map<std::string, std::string> headers = {});

        /// @brief Polling for circuit execution results via HTTP GET
        ///
        /// @param counts Output location for shot outcomes
        bool resultsReady(std::map<std::string, int> &counts);

    };

  }
}

namespace qristal
{
  /// Execute the circuit on QB hardware
  void execute_on_qb_hardware(
      std::shared_ptr<xacc::quantum::qb_qpu> qdk,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
      const run_i_j_config &run_config,
      bool debug);
}
