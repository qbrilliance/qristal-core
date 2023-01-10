/***
 *** Copyright (c) 2021 Quantum Brilliance Pty Ltd
 ***/
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

class QCStackClient : public xacc::Client {
private:
  bool debug_qb_hw_;
  const std::vector<int> VALID_HTTP_RETURN_CODES_{300, 425};

public:
  QCStackClient() : xacc::Client(), debug_qb_hw_(false) {}

  QCStackClient(const bool debug) : QCStackClient() { debug_qb_hw_ = debug; }
  const std::string post(const std::string &remoteUrl, const std::string &path,
                         const std::string &postStr,
                         std::map<std::string, std::string> headers =
                             std::map<std::string, std::string>{});

  const std::string get(const std::string &remoteUrl, const std::string &path,
                        std::map<std::string, std::string> headers =
                            std::map<std::string, std::string>{},
                        std::map<std::string, std::string> extraParams = {});
};

} // namespace xacc

namespace xacc {
namespace quantum {

/**
 * The QuantumBrillianceRemoteAccelerator
 * provides an execute implementation that maps XACC IR
 * to a suitable JSON message for execution on QB hardware
 */
class QuantumBrillianceRemoteAccelerator : virtual public RemoteAccelerator {
public:
  // Safe limit for QB hardware
  const int QB_SAFE_LIMIT_SHOTS = 512;

  // Constructors
  QuantumBrillianceRemoteAccelerator()
      : RemoteAccelerator(), debug_qb_hw_(false) {}
  QuantumBrillianceRemoteAccelerator(std::shared_ptr<Client> client,
                                     const bool debug = false)
      : RemoteAccelerator(client) {
    debug_qb_hw_ = debug;
  }
  QuantumBrillianceRemoteAccelerator(const bool debug)
      : QuantumBrillianceRemoteAccelerator() {
    debug_qb_hw_ = debug;
  }

  // Destructor
  virtual ~QuantumBrillianceRemoteAccelerator() {}

  // Getters
  const std::string getSignature() override { return name() + ":"; }
  const std::string name() const override { return "qb-qdk"; }
  const std::string description() const override {
    return "The Quantum Brilliance Remote Accelerator interacts with the QB "
           "QDK to execute XACC quantum IR.";
  }

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

  // Methods
  const std::vector<std::string> configurationKeys() override {
    return {"command",           "init",       "n_qubits",    "shots",
            "request_id",        "poll_id",    "cycles",      "results",
            "hwbackend",         "remote_url", "post_path",   "over_request",
            "recursive_request", "resample",   "retries_get", "retries_post",
            "resample_above_percentage"};
  }
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

  void initialize(const HeterogeneousMap &params = {}) override {
    updateConfiguration(params);
  }

  // Prepare JSON
  const std::string processInput(
      std::shared_ptr<AcceleratorBuffer> buffer,
      std::vector<std::shared_ptr<CompositeInstruction>> functions) override;

  // HTTP POST with JSON to execute circuit(s) specified in XASM
  void execute(std::shared_ptr<AcceleratorBuffer> buffer,
               const std::vector<std::shared_ptr<CompositeInstruction>>
                   functions) override;

  // Handle the response to the initial POST (circuit submission)
  void processResponse(std::shared_ptr<AcceleratorBuffer> buffer,
                       const std::string &response) override;

  // HTTP GET to poll for results of the circuit execution
  int pollForResults(
      std::shared_ptr<AcceleratorBuffer> buffer,
      const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
      std::map<std::string, int> &counts, int polling_interval,
      int polling_attempts);

  const int POLLING_NOT_READY = 300;
  const int POLLING_PROCESS_FAILED = 500;
  const int POLLING_SUCCESS = 0;

protected:
  bool debug_qb_hw_;

  // Command
  // std::string command_ = "run";
  std::string command_ = "circuit";

  // Number of shots in a cycle
  int shots_ = 1024;

  // Request ID
  int request_id_ = 0;

  // Poll ID
  int poll_id_ = 0;

  // Number of qubits
  size_t n_qubits_ = 2;

  // Init (vector of qubits, value is the initial state)
  std::vector<int> init_ = {0, 0};

  // Number of cycles
  int cycles_ = 1;

  // Format for results
  std::string results_ = "normal";

  // Real or dummy backend
  std::string hwbackend_ = "gen1_canberra";

  std::string handleExceptionRestClientPost(
      const std::string &_url, const std::string &path,
      const std::string &postStr, std::map<std::string, std::string> headers);

  // HTTP POST retries allowed
  int retries_post_ = 1;

  // Order of measurements
  std::vector<int> order_of_m_ = {};

  // Over-request factor
  int over_request_ = 4;

  // Enable recursive request to fulfill the shots_
  bool recursive_request_ = true;

  // Enable sample-with-replacement when set to true
  bool resample_ = false;

  std::string handleExceptionRestClientGet(
      const std::string &_url, const std::string &path,
      std::map<std::string, std::string> headers =
          std::map<std::string, std::string>{},
      std::map<std::string, std::string> extraParams = {});

  // HTTP GET retries allowed
  int retries_get_ = 1;

  // % threshold for valid shot results (as a proportion of requested shots) 
  // above which we will force the use of sample-with-replacement
  int resample_above_percentage_ = 95;
};

} // namespace quantum
} // namespace xacc
#endif
