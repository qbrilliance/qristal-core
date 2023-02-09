/**
 * Copyright Quantum Brilliance
 */
#ifndef QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEREMOTEACCELERATOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEREMOTEACCELERATOR_HPP_

#include "RemoteAccelerator.hpp"
#include "InstructionIterator.hpp"
#include "QuantumBrillianceRemoteVisitor.hpp"
#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>
namespace xacc {

/**
 * @brief Qristal implementation of a client for QC Stack Server
 * 
 * Methods supported:
 *    POST
 *    GET
 * Return codes 300 (legacy) or 425 (api/v1) are used by 
 * the QC Stack Server to indicate to a 
 * client that polling for results should be reattempted.
 */
class QCStackClient : public xacc::Client {
private:
  /// Toggles debug mode
  bool debug_qb_hw_;

  ///
  /// List of HTTP return codes that the client should interpret as: "continue with polling"
  /// The 300 code was legacy from an earlier implementations of QC Stack server.
  /// We keep this until the new api/v1 that uses code 425 is out and verified
  /// in deployed QDKs.
  const std::vector<int> VALID_HTTP_RETURN_CODES_{300, 425};

public:
  
  /// Default constructor that inits the parent class (xacc::Client)
  QCStackClient() : xacc::Client(), debug_qb_hw_(false) {}

  /// Constructor with debugging output
  QCStackClient(const bool debug) : QCStackClient() { debug_qb_hw_ = debug; }

  /*
   * HTTP POST implementation
   *
   * @param remoteUrl Input the URL where the server is located
   * @param path Input the endpoint path that handles the POST request
   * @param postStr Input body of the request, in JSON format
   * @param headers Input key-value pairs that are used for the HTTP Header
   */
  const std::string post(const std::string &remoteUrl, const std::string &path,
                         const std::string &postStr,
                         std::map<std::string, std::string> headers =
                             std::map<std::string, std::string>{});

  /*
   * HTTP GET implementation
   *
   * @param remoteUrl Input the URL where the QC Stack server is located
   * @param path Input the endpoint path that handles the GET request
   * @param headers Input key-value pairs that are used for the HTTP Header
   * @param extraParams Input key-value pairs that are used for query parameters
   */
  const std::string get(const std::string &remoteUrl, const std::string &path,
                        std::map<std::string, std::string> headers =
                            std::map<std::string, std::string>{},
                        std::map<std::string, std::string> extraParams = {});
};

} // namespace xacc

namespace xacc {
namespace quantum {

/**
 * @brief Provides an `execute` implementation that maps XACC IR to a suitable JSON message for execution on QB hardware.
 *
 * Execution is in two phases:
 *   1. Circuit submission via HTTP POST to QB hardware device
 *   2. Using the returned ID, form a HTTP GET request and poll repeatedly until results are returned
 * 
 * Options are provided to handle any shortfall in the requested number of shots:
 *   - Over-request factor
 *   - Recursive request
 *   - Sample with replacement
 */
class QuantumBrillianceRemoteAccelerator : virtual public RemoteAccelerator {
public:
  /// Safe limit for QB hardware
  const int QB_SAFE_LIMIT_SHOTS = 512;

  /// Default constructor that inits the parent class
  QuantumBrillianceRemoteAccelerator()
      : RemoteAccelerator(), debug_qb_hw_(false) {}
  
  /// Constructor that uses a custom HTTP client, such as QCStackClient
  /// Debugging output can be enabled with this constructor
  QuantumBrillianceRemoteAccelerator(std::shared_ptr<Client> client,
                                     const bool debug = false)
      : RemoteAccelerator(client) {
    debug_qb_hw_ = debug;
  }

  /// Constructor that enables debugging output
  QuantumBrillianceRemoteAccelerator(const bool debug)
      : QuantumBrillianceRemoteAccelerator() {
    debug_qb_hw_ = debug;
  }

  /// Destructor
  virtual ~QuantumBrillianceRemoteAccelerator() {}

  /// Getters
  const std::string getSignature() override { return name() + ":"; }
  const std::string name() const override { return "qb-qdk"; }
  const std::string description() const override {
    return "The Quantum Brilliance Remote Accelerator interacts with the QB "
           "QDK to execute XACC quantum IR.";
  }

  /*
   * Get the JSON payload that is sent to QB hardware
   *
   * @param program Input the XACC IR representation of a quantum circuit
   * @param config Input the configuration settings to apply to a quantum circuit
   */
  std::string getNativeCode(std::shared_ptr<CompositeInstruction> program,
                            const HeterogeneousMap &config = {}) override;

  bool isRemote() override { return true; }

  HeterogeneousMap getProperties() override {
    HeterogeneousMap m;
    m.insert("command", command_);
    m.insert("init", init_);
    m.insert("n_qubits", n_qubits_);
    m.insert("shots", shots_);
    m.insert("request_id", request_id_);
    m.insert("poll_id", poll_id_);
    m.insert("use_default_contrast_settings", use_default_contrast_settings_);
    m.insert("init_contrast_thresholds", init_contrast_thresholds_);
    m.insert("qubit_contrast_thresholds", qubit_contrast_thresholds_);
    m.insert("cycles", cycles_);
    m.insert("results", results_);
    m.insert("hwbackend", hwbackend_);
    m.insert("remote_url", remoteUrl);
    m.insert("post_path", postPath);
    m.insert("over_request", over_request_);
    m.insert("recursive_request", recursive_request_);
    m.insert("resample", resample_);
    m.insert("retries_post", retries_post_);
    m.insert("retries_get", retries_get_);
    m.insert("resample_above_percentage", resample_above_percentage_);
    return m;
  }

  /// Get the available configuration settings
  const std::vector<std::string> configurationKeys() override {
    return {"command",           "init",       "n_qubits",    "shots",
            "request_id",        "poll_id",    "use_default_contrast_settings", "init_contrast_thresholds",
            "qubit_contrast_thresholds",
            "cycles",      "results",
            "hwbackend",         "remote_url", "post_path",   "over_request",
            "recursive_request", "resample",   "retries_get", "retries_post",
            "resample_above_percentage"};
  }

  /// Get the endpoint for querying the QB hardware about native-gates
  const std::string get_native_gates_endpoint() {
    std::string retval;

    if (remoteUrl.find("api/v1") != std::string::npos) {
      retval = native_gates_get_path_;
    } else {
      retval = "api/v1/" + native_gates_get_path_;
    }
    return retval;
  }

  /**
    * Change the configuration of QB hardware
    *
    * @param config Input the new configuration settings to be applied
    */
  void updateConfiguration(const HeterogeneousMap &config) override {
    if (config.keyExists<std::string>("command")) {
      command_ = config.get<std::string>("command");
    }
    if (config.keyExists<std::vector<int>>("init")) {
      init_ = config.get<std::vector<int>>("init");
    }
    if (config.keyExists<int>("shots")) {
      shots_ = config.get<int>("shots");
    }
    if (config.keyExists<size_t>("n_qubits")) {
      n_qubits_ = config.get<size_t>("n_qubits");
    }
    if (config.keyExists<int>("request_id")) {
      request_id_ = config.get<int>("request_id");
    }
    if (config.keyExists<int>("poll_id")) {
      poll_id_ = config.get<int>("poll_id");
    }
    if (config.keyExists<bool>("use_default_contrast_settings")) {
      use_default_contrast_settings_ = config.get<bool>("use_default_contrast_settings");
    }
    if (config.keyExists<std::map<int,double>>("init_contrast_thresholds")) {
      init_contrast_thresholds_ = config.get<std::map<int,double>>("init_contrast_thresholds");
    }
    if (config.keyExists<std::map<int,double>>("qubit_contrast_thresholds")) {
      qubit_contrast_thresholds_ = config.get<std::map<int,double>>("qubit_contrast_thresholds");
    }
    if (config.keyExists<int>("cycles")) {
      cycles_ = config.get<int>("cycles");
    }
    if (config.keyExists<std::string>("results")) {
      results_ = config.get<std::string>("results");
    }
    if (config.keyExists<std::string>("hwbackend")) {
      hwbackend_ = config.get<std::string>("hwbackend");
    }
    if (config.keyExists<std::string>("remote_url")) {
      remoteUrl = config.get<std::string>("remote_url");
    }
    if (config.keyExists<std::string>("post_path")) {
      postPath = config.get<std::string>("post_path");
    }
    if (config.keyExists<int>("over_request")) {
      over_request_ = config.get<int>("over_request");
    }
    if (config.keyExists<bool>("recursive_request")) {
      recursive_request_ = config.get<bool>("recursive_request");
    }
    if (config.keyExists<bool>("resample")) {
      resample_ = config.get<bool>("resample");
    }
    if (config.keyExists<int>("request_id")) {
      request_id_ = config.get<int>("request_id");
    }
    if (config.keyExists<int>("poll_id")) {
      poll_id_ = config.get<int>("poll_id");
    }
    if (config.keyExists<int>("cycles")) {
      cycles_ = config.get<int>("cycles");
    }
    if (config.keyExists<std::string>("results")) {
      results_ = config.get<std::string>("results");
    }
    if (config.keyExists<std::string>("hwbackend")) {
      hwbackend_ = config.get<std::string>("hwbackend");
    }
    if (config.keyExists<std::string>("remote_url")) {
      remoteUrl = config.get<std::string>("remote_url");
    }
    if (config.keyExists<std::string>("post_path")) {
      postPath = config.get<std::string>("post_path");
    }
    if (config.keyExists<int>("retries_post")) {
      retries_post_ = config.get<int>("retries_post");
    }
    if (config.keyExists<int>("retries_get")) {
      retries_get_ = config.get<int>("retries_get");
    }
    if (config.keyExists<int>("resample_above_percentage")) {
        resample_above_percentage_ = config.get<int>("resample_above_percentage");
    }
  }

  /**
    * Initialize the configuration of QB hardware
    *
    * @param params Input the new configuration settings to be applied
    */
  void initialize(const HeterogeneousMap &params = {}) override {
    updateConfiguration(params);
  }

  /**
   * Converts the circuit to a representation that QB hardware accepts
   *
   * @param buffer Output location and storage of intermediate results
   * @param functions Input circuit in XACC IR format
   */
  const std::string processInput(
      std::shared_ptr<AcceleratorBuffer> buffer,
      std::vector<std::shared_ptr<CompositeInstruction>> functions) override;

  /**
   * Validate the capabilities of the QB hardware against
   * what the session requires
   */
   int validate_capability();

  /**
   * Submit the circuit with HTTP POST to QB hardware
   * and poll for results with HTTP GET
   *
   * @param buffer Output location and storage of intermediate results
   * @param functions Input circuit in XACC IR format
   */
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

  /**
   * Polling for circuit execution results via HTTP GET
   *
   * @param buffer Output location and storage of intermediate results
   * @param citargets Input circuit that has been previously submitted, for which the results are being polled for
   * @param counts Output location for shot outcomes
   * @param polling_interval Input the time in seconds between polling attempts - used only during recursive execution
   * @param polling_attempts Input the max number of attempts to poll for the shot outcomes - used only during recursive execution
   */
  int pollForResults(
      std::shared_ptr<AcceleratorBuffer> buffer,
      const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
      std::map<std::string, int> &counts, int polling_interval,
      int polling_attempts);

  const int POLLING_NOT_READY = 300;
  const int POLLING_PROCESS_FAILED = 500;
  const int POLLING_SUCCESS = 0;
  const int QRISTAL_QB_NATIVE_GATES_SUCCESS = 0;
  const int QRISTAL_QB_ARBITRARY_ROTATION_NOT_AVAILABLE = 4001;
  


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

  /// Number of qubits
  size_t n_qubits_ = 2;

  /// Init (vector of qubits, value is the initial state)
  std::vector<int> init_ = {0, 0};

  /// Contrast thresholds
  bool use_default_contrast_settings_ = true;
  std::map<int,double> init_contrast_thresholds_ = {}; 
  std::map<int,double> qubit_contrast_thresholds_ = {}; 

  /// Number of cycles
  int cycles_ = 1;

  /// Format for results
  std::string results_ = "normal";

  /// Real or dummy backend
  std::string hwbackend_ = "gen1_canberra";

  /// HTTP POST, returning the HTTP status code
  std::string handleExceptionRestClientPost(
      const std::string &_url, const std::string &path,
      const std::string &postStr, std::map<std::string, std::string> headers);

  /// HTTP POST retries allowed
  int retries_post_ = 1;

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
      const std::string &_url, const std::string &path,
      std::map<std::string, std::string> headers =
          std::map<std::string, std::string>{},
      std::map<std::string, std::string> extraParams = {});

  /// HTTP GET retries allowed for a single attempt
  int retries_get_ = 1;

  /// % threshold for valid shot results (as a proportion of requested shots) 
  /// above which we will force the use of sample-with-replacement
  int resample_above_percentage_ = 95;

  /// To keep history of the HTTP POST path
  std::string previous_post_path_ = {};

  /// Endpoint for GET of native gates
  std::string native_gates_get_path_ = "native-gates";
};

} // namespace quantum
} // namespace xacc
#endif
