// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// Qristal
#include "qb/core/backends/qb_hardware/qb_visitor.hpp"
#include "qb/core/session_utils.hpp"

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
    /// Options are provided to handle any shortfall in the requested number of shots:
    ///  - Over-request factor
    ///  - Recursive request
    ///  - Sample with replacement
    ///
    class qb_qpu : virtual public RemoteAccelerator
    {

      public:

        /// Safe limit for QB hardware
        const int QB_SAFE_LIMIT_SHOTS = 512;
      
        /// Default constructor that just inits the parent class
        qb_qpu(const bool debug = false) : RemoteAccelerator(), debug_qb_hw_(debug) {}
        
        /// Constructor that uses a custom HTTP client, such as QCStackClient
        qb_qpu(std::shared_ptr<Client> client, const bool debug = false)
            : RemoteAccelerator(client), debug_qb_hw_(debug) {}
            
        /// Destructor
        virtual ~qb_qpu() {}
      
        /// @{ @brief Getters
        const std::string getSignature() override;
        const std::string name() const override;
        const std::string description() const override;
        /// @}
      
        /// @brief Get the JSON payload that is sent to QB hardware
        ///
        /// @param program Input the XACC IR representation of a quantum circuit
        /// @param config Input the configuration settings to apply to a quantum circuit
        ///
        std::string getNativeCode(std::shared_ptr<CompositeInstruction> program,
                                  const HeterogeneousMap &config = {}) override;
      
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
        
        /// @brief Converts the circuit to a representation that QB hardware accepts
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param functions Input circuit in XACC IR format
        ///
        const std::string processInput(
            std::shared_ptr<AcceleratorBuffer> buffer,
            std::vector<std::shared_ptr<CompositeInstruction>> functions) override;
      
        /// Validate the capabilities of the QB hardware against what the session requires
        int validate_capability();
      
        /// @brief Submit the circuit with HTTP POST to QB hardware and poll for results with HTTP GET
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param functions Input circuit in XACC IR format
        ///
        void execute(std::shared_ptr<AcceleratorBuffer> buffer,
                     const std::vector<std::shared_ptr<CompositeInstruction>>
                         functions) override;
      
        /**
         * Handle the response to the initial POST (circuit submission)
         *
         * @param buffer Output location and storage of intermediate results
         * @param response Input the response body returned by the prior POST request
         */
        void processResponse(std::shared_ptr<AcceleratorBuffer> buffer,
                             const std::string &response) override;
      
        /// @brief Polling for circuit execution results via HTTP GET
        ///
        /// @param buffer Output location and storage of intermediate results
        /// @param citargets Input circuit that has been previously submitted, for which the results are being polled for
        /// @param counts Output location for shot outcomes
        /// @param polling_interval Input the time in seconds between polling attempts - used only during recursive execution
        /// @param polling_attempts Input the max number of attempts to poll for the shot outcomes - used only during recursive execution
        ///
        int pollForResults(
            std::shared_ptr<AcceleratorBuffer> buffer,
            const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
            std::map<std::string, int> &counts, int polling_interval,
            int polling_attempts);
      
      
      protected:
      
        bool debug_qb_hw_;
      
        /// Command
        std::string command_ = "circuit";
      
        /// Number of shots in a cycle
        int shots_ = 1024;
      
        /// Request ID
        int request_id_ = 0;
      
        /// Poll ID
        int poll_id_ = 0;
      
        /// Poll seconds
        double poll_secs_ = 0;

        /// Number of qubits
        size_t n_qubits_ = 2;
      
        /// Init (vector of qubits, value is the initial state)
        std::vector<int> init_ = {0, 0};
      
        /// Contrast thresholds
        bool use_default_contrast_settings_ = true;
        double init_contrast_threshold_ = 0; 
        std::map<int,double> qubit_contrast_thresholds_ = {}; 
      
        /// Number of cycles
        int cycles_ = 1;
      
        /// Format for results
        std::string results_ = "normal";
      
        /// Real or dummy backend
        std::string hwbackend_ = "gen1_canberra";
      
        /// HTTP POST, returning the HTTP status code
        std::string handleExceptionRestClientPost(
            const std::string &postUrl, const std::string &path,
            const std::string &postStr, std::map<std::string, std::string> headers);
      
        /// HTTP poll retries allowed
        int poll_retries_ = 1;
      
        /// Order of measurements
        std::vector<int> order_of_m_ = {};
      
        /// Over-request factor
        int over_request_ = 4;
      
        /// Enable recursive request to fulfill the shots_
        bool recursive_request_ = true;
      
        /// Enable sample-with-replacement when set to true
        bool resample_ = false;
        
        /// HTTP GET, returning the HTTP status code
        std::string handleExceptionRestClientGet(
            const std::string &postUrl, const std::string &path,
            std::map<std::string, std::string> headers =
                std::map<std::string, std::string>{},
            std::map<std::string, std::string> extraParams = {});
            
        /// % threshold for valid shot results (as a proportion of requested shots) 
        /// above which we will force the use of sample-with-replacement
        int resample_above_percentage_ = 95;
      
        /// To keep history of the HTTP POST path
        std::string previous_post_path_ = {};
      
        /// Endpoint for GET of native gates
        std::string native_gates_get_path_ = "native-gates";

    };
  
  }
}

namespace qb
{
  /// Execute the circuit on QB hardware
  void execute_on_qb_hardware(
      std::shared_ptr<xacc::quantum::qb_qpu> qdk,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
      const run_i_j_config &run_config,
      bool debug);
}
