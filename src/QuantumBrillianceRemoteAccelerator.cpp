/***
 *** Copyright (c) 2021 Quantum Brilliance Pty Ltd
 ***/
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include <memory>
#include <sstream>
#include <random>
#include <string>
#include <thread>
#include <cpr/cpr.h>

using json = nlohmann::json;

namespace xacc {

const std::string xacc::QCStackClient::post(const std::string &remoteUrl,
                               const std::string &path,
                               const std::string &postStr,
                               std::map<std::string, std::string> headers) {
  if (debug_qb_hw_) {
      std::cout << "* [DEBUG]: xacc::QCStackClient::post" << postStr << std::endl;
  }
  if (headers.empty()) {
    headers.insert(std::make_pair("Content-type", "application/json"));
    headers.insert(std::make_pair("Connection", "keep-alive"));
    headers.insert(std::make_pair("Accept", "*/*"));
  }

  cpr::Header cprHeaders;
  for (auto &kv : headers) {
    cprHeaders.insert({kv.first, kv.second});
  }

  auto r = cpr::Post(cpr::Url{remoteUrl + path}, cpr::Body(postStr), cprHeaders,
                     cpr::VerifySsl(false));
  if (r.status_code == 500) {
    xacc::info("* Error: QB hardware process failure");
  }
  if (r.status_code == 404) {
    xacc::info(
        "* Error: QB hardware received an invalid command");
  }
  if (r.status_code != 200)
    throw std::runtime_error("HTTP POST Error - status code " +
                             std::to_string(r.status_code) + ": " +
                             r.error.message + ": " + r.text);

  return r.text;
}

const std::string
xacc::QCStackClient::get(const std::string &remoteUrl, const std::string &path,
                         std::map<std::string, std::string> headers,
                         std::map<std::string, std::string> extraParams) {
  if (debug_qb_hw_) {
    std::cout << "* [DEBUG]: xacc::QCStackClient::get" << std::endl;
  }
  if (headers.empty()) {
    headers.insert(std::make_pair("Content-type", "application/json"));
    headers.insert(std::make_pair("Connection", "keep-alive"));
    headers.insert(std::make_pair("Accept", "*/*"));
  }

  cpr::Header cprHeaders;
  for (auto &kv : headers) {
    cprHeaders.insert({kv.first, kv.second});
  }

  cpr::Parameters cprParams;
  for (auto &kv : extraParams) {
    cprParams.AddParameter({kv.first, kv.second});
  }

  auto r = cpr::Get(cpr::Url{remoteUrl + path}, cprHeaders, cprParams,
                    cpr::VerifySsl(false));
  if (debug_qb_hw_) {
    std::cout << "* [DEBUG]: r.status_code: " << r.status_code << "\n";
  }
  if (r.status_code == 500) {
    xacc::info("* Error: QB hardware process failure");
  }
  if (r.status_code == 404) {
    xacc::info("* Error: QB hardware received an invalid command");
  }
  if (r.status_code != 200) {
    if (std::find(VALID_HTTP_RETURN_CODES_.begin(), VALID_HTTP_RETURN_CODES_.end(), r.status_code) != VALID_HTTP_RETURN_CODES_.end()) {
        json gr;
        gr["status_code"] = r.status_code;
        std::cout << "* [Debug]: r.status_code: " << r.status_code << "\n";
        return gr.dump();
    } else {
      throw std::runtime_error("HTTP GET Error - status code " +
                               std::to_string(r.status_code) + ": " +
                               r.error.message + ": " + r.text);
    }
  }
  return r.text;
}

} // namespace xacc

namespace xacc {
namespace quantum {

/*
 * processInput() :
 * 1. sets up HTTP POST request headers
 * 2. sets up QB specific metadata
 * 3. visits XACC IR to construct JSON strings for the circuit +  required measurements
 * 4. combines 2. and 3. into the HTTP POST request body (string)
 */
const std::string QuantumBrillianceRemoteAccelerator::processInput(
    std::shared_ptr<AcceleratorBuffer> buffer,
    std::vector<std::shared_ptr<CompositeInstruction>> functions) {

  // 1. HTTP POST request headers
  headers.clear();
  headers.insert(
      std::make_pair("Content-type", "application/json; charset=utf-8"));
  headers.insert(std::make_pair("Connection", "keep-alive"));
  // headers.insert(std::make_pair("Accept-Encoding", "gzip, deflate"));
  // headers.insert(std::make_pair("Accept", "application/octet-stream"));

  // 2. QB metadata
  json jsel;
  jsel["command"] = command_;             // default: "run"

  // 2.1 Safe operating limit enforced here
  int shots_ov = shots_*over_request_;
  if (shots_ov <= QB_SAFE_LIMIT_SHOTS) {
      jsel["settings"]["shots"] = shots_ov;
  } else {
      std::cout << "* The (over-)requested number of shots [" << shots_ov << "] exceeds QB_SAFE_LIMIT_SHOTS [" << QB_SAFE_LIMIT_SHOTS
                << "] - only QB_SAFE_LIMIT_SHOTS will be requested" << std::endl;
      jsel["settings"]["shots"] = QB_SAFE_LIMIT_SHOTS;
  }

  jsel["settings"]["cycles"] = cycles_;   // default: 1
  jsel["settings"]["results"] = results_; // default: "normal"
  if (!use_default_contrast_settings_) {
    jsel["settings"]["readout_contrast_threshold"]["init"] = init_contrast_thresholds_.at(0);  // assume this is a scalar (not a list) for now
    json qctjs;
    for (auto &qel : qubit_contrast_thresholds_) {
      qctjs.push_back(qel.second);
    }
    jsel["settings"]["readout_contrast_threshold"]["qubits"] = qctjs;
  }
  jsel["hwbackend"] = hwbackend_;         // default: "gen1_canberra"
  jsel["init"] = init_;

  // 3. Circuit
  // jsel["circuit"] is built from a visitor
  // Create the Instruction Visitor that is going to map the IR.
  auto visitor_no_meas =
      std::make_shared<xacc::quantum::QuantumBrillianceRemoteVisitor>(
          buffer->size());
  InstructionIterator it(functions[0]);
  order_of_m_.clear();
  while (it.hasNext()) {
    // Get the next node in the tree
    auto nextInst = it.next();
    if (nextInst->isEnabled()) {
      if (nextInst->name() == "Measure") {
        order_of_m_.push_back(nextInst->bits()[0]);
      } else {
        nextInst->accept(visitor_no_meas);
      }
    }
  }
  jsel["circuit"] = json::parse(visitor_no_meas->getXasmString());
  if (jsel["circuit"].is_null()) {
      jsel["circuit"] = json::array({});
  }

  // 3. Measurements
  // jsel["measure"] is built from iterating the IR
  json measjs;
  for (uint ii = 0; ii < order_of_m_.size(); ii++) {
    measjs.push_back({order_of_m_.at(ii), ii});
  }
  jsel["measure"] = measjs;
  return jsel.dump();
}

/* 
 * Validate the capabilities of the QB hardware against what the session requires
 */
int QuantumBrillianceRemoteAccelerator::validate_capability() {  
  int retval = QRISTAL_QB_NATIVE_GATES_SUCCESS;
  if (debug_qb_hw_) {
    std::cout << "* Query for native gates supported at path: " << remoteUrl << native_gates_get_path_
              << std::endl;
  }
  json fromqdk = json::parse(
      QuantumBrillianceRemoteAccelerator::handleExceptionRestClientGet(
      remoteUrl, native_gates_get_path_, headers));
  if (debug_qb_hw_) {
    std::cout << "* Native gates query returned: " << fromqdk.dump() << "\n";
  }

  /// Validation for session configuration against hardware capabilities

  /// Add more validations as required below this line:
  return retval;
};


std::string QuantumBrillianceRemoteAccelerator::getNativeCode(std::shared_ptr<CompositeInstruction> program,
                            const HeterogeneousMap &config) {
  size_t n_qubits = 0;

  if (config.keyExists<size_t>("n_qubits")) {
        n_qubits = config.get<size_t>("n_qubits");
        std::vector<std::shared_ptr<CompositeInstruction>> functions{program};
        auto buffer_b = xacc::qalloc(n_qubits);
        return processInput(buffer_b, functions);
  }
  else {
    throw std::range_error("The number of qubits [n_qubits] was not defined");
  }
}


std::string QuantumBrillianceRemoteAccelerator::handleExceptionRestClientPost(
    const std::string &_url, const std::string &path,
    const std::string &postStr, std::map<std::string, std::string> headers) {
  std::string postResponse;
  std::exception ex;
  bool succeeded = false;
  auto m = getProperties();
  int retries = m.get<int>("retries_post");

  // Execute HTTP Post
  do {
    try {
      postResponse = restClient->post(_url, path, postStr, headers);
      succeeded = true;
      break;
    } catch (std::exception &e) {
      ex = e;
      xacc::info("Remote Accelerator " + name() +
                 " caught exception while calling restClient->post() "
                 "- " +
                 std::string(e.what()));
      retries--;
      if (retries > 0) {
        xacc::info("Retrying HTTP Post.");
      }
    }
  } while (retries > 0);

  if (!succeeded) {
    cancel();
    xacc::error("Remote Accelerator " + name() +
                " failed HTTP Post for Job Response - " +
                std::string(ex.what()));
  }

  return postResponse;
}

std::string QuantumBrillianceRemoteAccelerator::handleExceptionRestClientGet(
    const std::string &_url, const std::string &path,
    std::map<std::string, std::string> headers,
    std::map<std::string, std::string> extraParams) {
  std::string getResponse;
  std::exception ex;
  bool succeeded = false;
  auto m = getProperties();
  int retries = m.get<int>("retries_get");

  // Execute HTTP Get
  do {
    try {
      getResponse = restClient->get( _url + ((_url.back() != '/') ? "/" : ""), path, headers, extraParams);
      succeeded = true;
      break;
    } catch (std::exception &e) {
      ex = e;
      xacc::info("Remote Accelerator " + name() +
                 " caught exception while calling restClient->get() "
                 "- " +
                 std::string(e.what()));
      // s1.find(s2) != std::string::npos) {
      if (std::string(e.what()).find("Caught CTRL-C") != std::string::npos) {
        cancel();
        xacc::error(std::string(e.what()));
      }
      retries--;
      if (retries > 0) {
        xacc::info("Retrying HTTP Get.");
      }
    }
  } while (retries > 0);

  if (!succeeded) {
    cancel();
    xacc::error("Remote Accelerator " + name() + 
                " failed HTTP Get for Job Response - " +
                std::string(ex.what()));
  }

  return getResponse;
}

void QuantumBrillianceRemoteAccelerator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<CompositeInstruction>> functions) {
  int counter = 0;
  std::vector<std::shared_ptr<AcceleratorBuffer>> tmpBuffers;
  for (auto f : functions) {
      if (debug_qb_hw_) {
              std::cout << "* [DEBUG]: execute counter: " << counter << std::endl;
      }
    xacc::info("QB QDK executing kernel: " + f->name());
    auto tmpBuffer = std::make_shared<AcceleratorBuffer>(
        buffer->name() + std::to_string(counter), buffer->size());
    RemoteAccelerator::execute(tmpBuffer, f);
    tmpBuffers.push_back(tmpBuffer);
    buffer->appendChild(tmpBuffer->name(), tmpBuffer);
    counter++;
  }
  return;
}

void QuantumBrillianceRemoteAccelerator::processResponse(
    std::shared_ptr<AcceleratorBuffer> , const std::string &response) {
  if (debug_qb_hw_) {
    std::cout << "* Response from HTTP POST: " << response << std::endl;
  }
  json respost = json::parse(response);
  auto idstr = respost["id"].get<int>();
  previous_post_path_ = postPath;
  postPath.append(std::to_string(idstr));

  if (debug_qb_hw_)
    std::cout << "* POST done - poll for results at path: " << remoteUrl 
              << postPath << std::endl;
  return;
}

int QuantumBrillianceRemoteAccelerator::pollForResults(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
    std::map<std::string, int> &counts, int polling_interval,
    int polling_attempts) {
  int retval = POLLING_NOT_READY;
  if (debug_qb_hw_) {
    std::cout << "* Poll for results at path: " << remoteUrl << postPath
              << std::endl;
  }
  json fromqdk = json::parse(
      QuantumBrillianceRemoteAccelerator::handleExceptionRestClientGet(
      remoteUrl, postPath, headers));

  std::default_random_engine qb_rnd_gen(
      static_cast<long unsigned int>(time(0)));
  int unif_lb = 0;
  int acc_valid = 0;
  auto m = getProperties();
  int requested_shots = m.get<int>("shots");
  int tmp_n = 0;

  // Accumulate counts in a map of string -> int
  // std::map<std::string, int> counts;

  if (fromqdk["data"] != nullptr) {
    int unif_ub = fromqdk["data"].size() - 1;
    if (unif_ub < 0) {
      retval = POLLING_PROCESS_FAILED;
    }
    std::uniform_int_distribution<int> p_unif(unif_lb, unif_ub);

    // Start of resample (sample-with-replacement) procedure
    if (m.get<bool>("resample")) {
      while (acc_valid < requested_shots) {
        tmp_n = p_unif(qb_rnd_gen);
        std::stringstream current_state;
        auto el = fromqdk["data"][tmp_n];
        for (auto &el_it : el) {
          current_state << el_it;
        }
        std::string bitString = current_state.str();
        if (counts.find(bitString) != counts.end()) {
          counts[bitString]++;
          acc_valid++;
        } else {
          counts.insert(std::make_pair(bitString, 1));
          acc_valid++;
        }
      }
    } else {
      for (auto &el : fromqdk["data"]) {
        if (acc_valid < requested_shots) {
          std::stringstream current_state;
          for (auto &el_it : el) {
            current_state << el_it;
          }
          std::string bitString = current_state.str();
          if (counts.find(bitString) != counts.end()) {
            counts[bitString]++;
            acc_valid++;
          } else {
            counts.insert(std::make_pair(bitString, 1));
            acc_valid++;
          }
        }
      }
    }

    // Start of recursive calls
    if (acc_valid == requested_shots) {
      retval = POLLING_SUCCESS;
    } else {
      if (m.get<bool>("recursive_request")) {
        // A QCStack client - provide argument 'true' for debug mode
        std::shared_ptr<xacc::Client> qcs_qdk =
            std::make_shared<xacc::QCStackClient>(true);
        std::shared_ptr<xacc::quantum::QuantumBrillianceRemoteAccelerator>
            tqdk = std::make_shared<
                xacc::quantum::QuantumBrillianceRemoteAccelerator>(qcs_qdk,
                                                                   true);
        auto next_properties = getProperties();
        next_properties.insert("shots", (requested_shots - acc_valid));

        // Threshold % above which to trigger resample procedure
        if (100*acc_valid/requested_shots >= resample_above_percentage_) {
            if (debug_qb_hw_) {
                std::cout << "# Recursive request: forced resampling procedure at " << (100*acc_valid/requested_shots) <<" % valid" << std::endl;
            }
            next_properties.insert("resample", true);
            // Increase the over_request factor for the final request to minimise the 
            // chance of an empty reply from the QDK
            next_properties.insert("over_request", m.get<int>("over_request")*8);
        }

        next_properties.insert("post_path", previous_post_path_);
        if (debug_qb_hw_) {
          std::cout << "# Recursive request: remote URL is "
                    << next_properties.get<std::string>("remote_url")
                    << std::endl;
          std::cout << "# Recursive request: post path is "
                    << next_properties.get<std::string>("post_path")
                    << std::endl;
          std::cout << "# Recursive request: shots is "
                    << next_properties.get<int>("shots") << std::endl;
        }
        tqdk->updateConfiguration(next_properties);
        auto buffer_b = xacc::qalloc(m.get<size_t>("n_qubits"));
        if (debug_qb_hw_) {
          std::cout << "# Recursive request: polling interval is "
                    << polling_interval << " seconds" << std::endl;
        }
        try {
          tqdk->execute(buffer_b, citargets);
        } catch (...) {
          throw std::invalid_argument(
              "The execution on hardware of your input circuit failed");
        }

        // Set up polling

        using namespace std::chrono_literals;
        for (int i = 0; i < polling_attempts; i++) {
          std::this_thread::sleep_for(std::chrono::seconds(polling_interval));
          if (debug_qb_hw_) {
            std::cout << "# Waited for " << polling_interval << " seconds"
                      << std::endl;
          }
          int poll_return = POLLING_NOT_READY;
          poll_return = tqdk->pollForResults(
              buffer_b, citargets, counts, polling_interval, polling_attempts);
          if (debug_qb_hw_) {
            std::cout << "# Poll return: " << poll_return << std::endl;
          }
          retval = poll_return;

          if (retval == POLLING_SUCCESS) {
            break;
          }
        }
      } else {
        retval = POLLING_SUCCESS;
      }
    }

    // Returned from recursive call...
    // now proceed to store the counts in the buffer
    for (auto &kv : counts) {
      buffer->appendMeasurement(kv.first, kv.second);
      if (debug_qb_hw_) {
        std::cout << "State: " << kv.first << " has count: " << kv.second
                  << std::endl;
      }
    }
    retval = POLLING_SUCCESS;
  } else {
    std::cout << "* No 'data' found..." << std::endl;
    retval = POLLING_NOT_READY;
  }
  return retval;
}

}  // namespace quantum
}  // namespace xacc
