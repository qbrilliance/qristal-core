// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Accelerator.hpp"
#include "json.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include <cpr/cpr.h>
#include <thread>
#include "CountGatesOfTypeVisitor.hpp"
#include "CommonGates.hpp"
#include <bitset>
#include <type_traits>

namespace {
#define IS_INTEGRAL(T)                                                         \
  typename std::enable_if<std::is_integral<T>::value>::type * = 0

template <class T>
std::string integral_to_binary_string(T byte, IS_INTEGRAL(T)) {
  std::bitset<sizeof(T) * 8> bs(byte);
  return bs.to_string();
}
} // namespace
namespace qbOS {
class LambdaRemoteAccelerator : public xacc::Accelerator, public xacc::Cloneable<xacc::Accelerator> {
private:
  std::string m_ipAddress = "127.0.0.1:5000";
  std::string m_device = "GPU";
  std::string m_noiseJson;
  int m_shots = 1024;
public:
  virtual const std::string name() const override { return "qb-lambda"; }
  virtual const std::string description() const override {
    return "Quantum Brilliance GPU-based Simulation Accelerator.";
  }
  virtual void initialize(const xacc::HeterogeneousMap &params = {}) override {
    m_noiseJson.clear();
    if (params.stringExists("noise-model")) {
      m_noiseJson = params.getString("noise-model");
    }
    updateConfiguration(params);
  }
  virtual void
  updateConfiguration(const xacc::HeterogeneousMap &params) override {
    if (params.stringExists("url")) {
      m_ipAddress = params.getString("url");
    }
    if (params.stringExists("device")) {
      m_device = params.getString("device");
    }

    if (params.keyExists<int>("shots")) {
      m_shots = params.get<int>("shots");
    }
  }

  virtual const std::vector<std::string> configurationKeys() override {
    return {};
  }

  virtual xacc::HeterogeneousMap getProperties() override { return {}; }

  virtual void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                       const std::shared_ptr<xacc::CompositeInstruction>
                           CompositeInstruction) override {
    const std::string openQASM = [&]() {
      auto staq = xacc::getCompiler("staq");
      xacc::storeBuffer(buffer);
      const auto openQASMSrc = staq->translate(CompositeInstruction);
      return openQASMSrc;
    }();
    std::map<std::string, std::string> headers{
        {"Content-type", "application/json"}};

    cpr::Header cprHeaders;
    for (auto &kv : headers) {
      cprHeaders.insert({kv.first, kv.second});
    }
    const std::string remoteUrl = m_ipAddress + "/job";
    nlohmann::json payload;
    payload["openqasm"] = openQASM;
    payload["device"] = m_device;
    payload["shots"] = m_shots;
    if (!m_noiseJson.empty()) {
      payload["noise_model"] = m_noiseJson;
    }
    // Max number of qubits that we can do state-vector sim with GPU
    constexpr int MAX_QUBITS_IDEAL = 31;
    constexpr int MAX_QUBITS_NOISE = 27;
    const auto nQubits = CompositeInstruction->nPhysicalBits();
    const bool canUseStateVec =
        (m_noiseJson.empty() && nQubits <= MAX_QUBITS_IDEAL) ||
        (!m_noiseJson.empty() && nQubits <= MAX_QUBITS_NOISE);

    if (!canUseStateVec) {
      payload["method"] = "matrix_product_state";
    }
    // std::cout << "Payload:\n" << payload.dump() << "\n";
    auto r =
        cpr::Put(cpr::Url{remoteUrl}, cpr::Body(payload.dump()), cprHeaders);

    if (r.status_code != 200) {
      std::cout << r.status_code << ", " << r.text << ", " << r.error.message
                << "\n";
      throw std::runtime_error("HTTP Error - status code " +
                               std::to_string(r.status_code) + ": " +
                               r.error.message + ": " + r.text);
    }

    // std::cout << "Response: " << r.text << "\n";
    auto response_json = nlohmann::json::parse(r.text);
    const auto job_id = response_json["job-id"].get<std::string>();
    const auto status = response_json["status"].get<std::string>();
    if (status != "SUBMITTED") {
      throw std::runtime_error("Failed to submit job to the backend: " +
                               r.text);
    }
    int dots = 1;
    // std::cout << "Job ID: " << job_id << "\n";
    std::string result_json_str;
    while (true) {
      auto get_job_status =
          cpr::Get(cpr::Url{remoteUrl + "/" + job_id},
                   cpr::Header{{"Content-Type", "application/json"}});
      // std::cout << "Response:\n" << get_job_status.text << "\n";
      auto get_job_status_json = nlohmann::json::parse(get_job_status.text);
      if (get_job_status_json["status"].get<std::string>() == "COMPLETED") {
        result_json_str = get_job_status_json["data"].get<std::string>();
        break;
      }

      // Add a status message...
      if (dots > 4) {
        dots = 1;
      }
      std::stringstream ss;
      ss << "\033[0;32m"
         << "Lambda Job "
         << "\033[0;36m" << job_id << " " << "\033[0;32m";
      const auto statusStr = get_job_status_json["status"].get<std::string>();

      if (statusStr != "RUNNING") {
        ss << "\033[1;33m" << statusStr;
      } else {
        ss << statusStr;
      }
      for (int i = 0; i < dots; i++) {
        ss << '.';
      }

      dots++;
      std::cout << '\r' << ss.str() << std::setw(20) << std::setfill(' ')
                << std::flush;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // End the color log
    std::cout << "\033[0m"
              << "\n";

    auto result_json = nlohmann::json::parse(result_json_str);
    // std::cout << "Result:\n" << result_json.dump() << "\n";
    const bool success = result_json["success"].get<bool>();
    if (!success) {
      throw std::runtime_error("Failed to execute: " +
                               result_json["status"].get<std::string>());
    }

    auto results = result_json["results"];
    auto counts = (*(results.begin()))["data"]["counts"]
                      .get<std::map<std::string, int>>();
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::Measure> cc(
        CompositeInstruction);
    int nMeasures = cc.countGates();
    const auto hex_to_bin = [](const std::string &hex_str) {
      return integral_to_binary_string(strtol(hex_str.c_str(), NULL, 0));
    };

    for (const auto &[hexStr, nOccurrences] : counts) {
      // std::cout << hexStr << " --> " << nOccurrences << "\n";
      const auto bitStr = hex_to_bin(hexStr);
      std::string actual(nMeasures, '0');
      for (int i = 0; i < nMeasures; i++) {
        actual[actual.length() - 1 - i] = bitStr[bitStr.length() - i - 1];
      }
      buffer->appendMeasurement(actual, nOccurrences);
    }
    // buffer->print();
  }

  virtual void
  execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
              CompositeInstructions) override {
    for (auto &f : CompositeInstructions) {
      auto tmpBuffer =
          std::make_shared<xacc::AcceleratorBuffer>(f->name(), buffer->size());
      execute(tmpBuffer, f);
      buffer->appendChild(f->name(), tmpBuffer);
    }
  }

  virtual std::shared_ptr<xacc::Accelerator> clone() override {
    return std::make_shared<qbOS::LambdaRemoteAccelerator>();
  }
};
} // namespace qbOS
REGISTER_ACCELERATOR(qbOS::LambdaRemoteAccelerator)
