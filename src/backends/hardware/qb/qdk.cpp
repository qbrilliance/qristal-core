// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/backends/hardware/qb/qdk.hpp>
#include <qristal/core/backends/hardware/qb/visitor_CZ.hpp>
#include <qristal/core/backends/hardware/qb/visitor_ACZ.hpp>

// STL
#include <sstream>
#include <random>
#include <string>
#include <stdexcept>

// CPR
#include <cpr/cpr.h>

// JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;

const std::string circuitEndpoint = "api/v1/circuits";
const std::string nativeGateEndpoint = "api/v1/native-gates";
const std::string reservationEndpoint = "api/v1/reservations";

namespace {
  constexpr int32_t HTTP_TIMEOUT = 5000;
  constexpr int32_t HTTP_NUM_RETRIES = 10;
}

namespace qristal
{

  // Send a circuit for execution on QB hardware
  void execute_on_qb_hardware(
      std::shared_ptr<xacc::quantum::qdk> hardware_device,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer,
      std::shared_ptr<xacc::CompositeInstruction>& circuit,
      bool execute_circuit,
      bool debug)
  {
    hardware_device->setup_hardware(execute_circuit);
    hardware_device->execute(buffer, circuit, execute_circuit);
    if (execute_circuit)
    {
      std::map<std::string, int> counts = hardware_device->poll_for_results();
      // Store the counts in the buffer
      for (auto &kv : counts)
      {
        buffer->appendMeasurement(kv.first, kv.second);
        if (debug) std::cout << "State: " << kv.first << " has count: " << kv.second << std::endl;
      }
    }
  }
}


namespace xacc
{
  namespace quantum
  {

    // Overrides of XACC RemoteAccelerator functionalities

    // Getters
    const std::string qdk::get_qbjson() const
    {
      return qbjson;
    }

    const std::string qdk::getSignature()
    {
      return name() + ":";
    }

    const std::string qdk::name() const
    {
      return qpu_name;
    }

    const std::string qdk::description() const
    {
      return "The QB QPU backend interacts with QB hardware.";
    }

    // Indicate that this is indeed a remote XACC accelerator
    bool qdk::isRemote()
    {
      return true;
    }

    // Retrieve the properties of the backend
    HeterogeneousMap qdk::getProperties()
    {
      HeterogeneousMap m;
      m.insert("command", model);
      m.insert("command", command);
      m.insert("init", init);
      m.insert("n_qubits", static_cast<int>(n_qubits));
      m.insert("shots", static_cast<int>(shots));
      m.insert("poll_secs", poll_secs);
      m.insert("poll_retries", poll_retries);
      m.insert("use_default_contrast_settings", use_default_contrast_settings);
      m.insert("init_contrast_threshold", init_contrast_threshold);
      m.insert("qubit_contrast_thresholds", qubit_contrast_thresholds);
      m.insert("results", results);
      m.insert("url", remoteUrl);
      m.insert("exclusive_access", exclusive_access);
      m.insert("exclusive_access_token", exclusive_access_token);
      return m;
    }

    // Get the available configuration settings
    const std::vector<std::string> qdk::configurationKeys()
    {
      return
      {
        "model",
        "command",
        "init",
        "n_qubits",
        "shots",
        "poll_secs",
        "poll_retries",
        "use_default_contrast_settings",
        "init_contrast_threshold",
        "qubit_contrast_thresholds",
        "results",
        "url",
        "exclusive_access",
        "exclusive_access_token"
      };
    }

    // Change the configuration of QB hardware
    void qdk::updateConfiguration(const HeterogeneousMap &config)
    {
      auto update = [&]<typename T>(const std::string& key, T& var)
      {
        if (config.keyExists<T>(key)) var = config.get<T>(key);
      };
      update("model", model);
      update("command", command);
      update("init", init);
      update("poll_secs", poll_secs);
      update("poll_retries", poll_retries);
      update("use_default_contrast_settings", use_default_contrast_settings);
      update("init_contrast_threshold", init_contrast_threshold);
      update("qubit_contrast_thresholds", qubit_contrast_thresholds);
      update("results", results);
      update("url", remoteUrl);
      update("exclusive_access", exclusive_access);
      update("exclusive_access_token", exclusive_access_token);
      if (config.keyExists<int>("shots")) shots = config.get<int>("shots");
      if (config.keyExists<int>("n_qubits")) n_qubits = config.get<int>("n_qubits");
    }

    // Initialize the configuration of QB hardware
    void qdk::initialize(const HeterogeneousMap &params)
    {
      updateConfiguration(params);
    }

    // Generic wrapper around CPR HTTP operations
    std::string HTTP(
        const std::string& operation,
        std::function<cpr::Response(cpr::Url, cpr::Header)> f,
        const std::string& remoteUrl,
        const std::string& path,
        std::map<std::string, std::string>& headers,
        qdk& qpu,
        uint32_t retry_count,
        bool debug)
    {
      std::string Response;

      // Execute HTTP operation
      try
      {
        if (debug) std::cout << "* qdk::" << operation << " to " << remoteUrl << path << std::endl;

        if (headers.find("Content-type") == headers.end()) headers.insert(std::make_pair("Content-type", "application/json"));
        if (headers.find("Connection") == headers.end()) headers.insert(std::make_pair("Connection", "keep-alive"));
        if (headers.find("Accept") == headers.end()) headers.insert(std::make_pair("Accept", "*/*"));

        cpr::Header cprHeaders;
        for (auto &kv : headers) cprHeaders.insert({kv.first, kv.second});

        auto r = f(cpr::Url{remoteUrl + path}, cprHeaders);

        // Lambda for rolling status code into response
        auto response_is_status_code = [](auto status_code)
        {
          json temp;
          temp["status_code"] = status_code;
          return temp.dump();
        };

        if (debug) std::cout << "* Status code " << r.status_code << std::endl;
        switch (r.status_code)
        {
          case 0:  // No response. Automatically retry unless the retry count exceeds HTTP_NUM_RETRIES
            if (retry_count < HTTP_NUM_RETRIES) return HTTP(operation, f, remoteUrl, path, headers, qpu, retry_count + 1, debug);
            throw std::runtime_error("Device " + qpu.name() + " at " + remoteUrl + " did not respond to HTTP " +
             operation + " operation after " + std::to_string(HTTP_NUM_RETRIES+1) + " attempts.\nError: " + r.error.message);
            break;
          case 200: // Success
            Response = (r.text == "null" ? response_is_status_code(r.status_code) : r.text);
            break;
          case 425: // Continue polling
            Response = response_is_status_code(r.status_code);
            break;
          default:  // Error
            qpu.cancel();
            std::string detail = "not provided by hardware";
            try { detail = json::parse(r.text)["detail"]; } catch (const json::type_error&) {}
            throw std::runtime_error("\nDevice " + qpu.name() + " failed HTTP " + operation +
            ".\nReturn code: " + std::to_string(r.status_code) + "\nDetail: " + detail);
            break;
        }
      }
      catch (std::exception &e)
      {
        qpu.cancel();
        if (std::string(e.what()).find("Caught CTRL-C") != std::string::npos or retry_count > 0)
        {
          throw std::runtime_error(std::string(e.what()));
        }
        throw std::runtime_error(qpu.name() + " raised exception in " + operation + ". " + std::string(e.what()));
      }
      return Response;
    }

    // HTTP POST specialisation
    std::string qdk::Post(
        const std::string &remoteUrl, const std::string &path,
        const std::string &postStr, std::map<std::string, std::string> headers)
    {
      auto f = [&](cpr::Url a, cpr::Header b) {
        return cpr::Post(a, cpr::Body(postStr), b, cpr::VerifySsl(false), cpr::Timeout{HTTP_TIMEOUT});
      };
      return HTTP("POST", f, remoteUrl, path, headers, *this, 0, debug);
    }

    // HTTP GET specialisation
    std::string qdk::Get(
        const std::string &remoteUrl, const std::string &path,
        std::map<std::string, std::string> headers,
        std::map<std::string, std::string> extraParams)
    {
      cpr::Parameters cprParams;
      for (const auto& kv : extraParams) { cprParams.Add(cpr::Parameter{kv.first, kv.second}); }
      auto f = [&cprParams](cpr::Url a, cpr::Header b) {
        return cpr::Get(a, b, cprParams, cpr::VerifySsl(false), cpr::Timeout{HTTP_TIMEOUT});
      };
      return HTTP("GET", f, remoteUrl, path, headers, *this, 0, debug);
    }

    // HTTP PUT specialisation
    std::string qdk::Put(
        const std::string &remoteUrl, const std::string &path,
        const std::string &putStr, std::map<std::string, std::string> headers)
    {
      auto f = [&putStr](cpr::Url a, cpr::Header b) {
        return cpr::Put(a, cpr::Body(putStr), b, cpr::VerifySsl(false), cpr::Timeout{HTTP_TIMEOUT});
      };
      return HTTP("PUT", f, remoteUrl, path, headers, *this, 0, debug);
    }


    // Initialise the QB hardware (reserve, get native gateset, etc.)
    void qdk::setup_hardware(bool check_hardware_lifesigns)
    {
      try
      {
        // Set up any headers needed for exclusive access
        if (exclusive_access)
        {
          http_header = {{"Authorization", "Bearer " + exclusive_access_token}};
          Put(remoteUrl, reservationEndpoint, "", http_header);
        }

        // Get native gateset
        if (check_hardware_lifesigns)
        {
          json fromqdk = json::parse(Get(remoteUrl, nativeGateEndpoint));
          if (debug) std::cout << "* Native gates query returned: " << fromqdk.dump() << "\n";
        }
      }
      catch (std::exception& e)
      {
        throw std::runtime_error("Error raised during QB hardware initialisation: " + std::string(e.what()));
      }
    };


    void qdk::execute(
        std::shared_ptr<AcceleratorBuffer> buffer,
        const std::shared_ptr<CompositeInstruction> function,
        bool execute_circuit)
    {
      try
      {
        if (debug) std::cout << "QB QDK executing kernel: " + function->name() << std::endl;
        qbjson = processInput(buffer, std::vector<std::shared_ptr<CompositeInstruction>>{function});
        // Output the JSON sent to the QB hardware if debug is turned on.
        if (debug) std::cout << "* JSON to be sent to QB hardware: " << std::endl << qbjson << std::endl;
        if (execute_circuit)
        {
          std::string responseStr = Post(remoteUrl, circuitEndpoint, qbjson, http_header);
          processResponse(buffer, responseStr);
        }
      }
      catch (std::exception& e)
      {
        throw std::runtime_error(std::string(e.what()) + "\nThe execution on hardware of your input circuit failed.");
      }
      return;
    }

    /// Helper function for finding a visitor class (=set of transpilation rules) for a given hardware model.
    template<class first, class... others>
    std::shared_ptr<xacc::quantum::visitor> make_visitor(std::string model, const int qubits) {
      if (model == first::model) return std::make_shared<first>(qubits);
      if constexpr (sizeof...(others) > 0) return make_visitor<others...>(model, qubits);
      throw std::runtime_error("Unknown Quantum Brilliance hardware model: " + model);
    }

    // Convert a circuit to a representation that QB hardware accepts.
    //
    // Sets up QB specific metadata, visits XACC IR to construct JSON strings
    // for the circuit +  required measurements, then combines both into the
    // HTTP POST request body.
    const std::string qdk::processInput(
        std::shared_ptr<AcceleratorBuffer> buffer,
        std::vector<std::shared_ptr<CompositeInstruction>> functions)
    {
      // QB metadata
      json jsel;
      jsel["command"] = command;
      jsel["settings"]["shots"] = shots;
      jsel["settings"]["results"] = results; // default: "normal"
      if (!use_default_contrast_settings)
      {
        jsel["settings"]["readout_contrast_threshold"]["init"] = init_contrast_threshold;
        json qctjs;
        for (auto &qel : qubit_contrast_thresholds) {
          qctjs.push_back(qel.second);
        }
        jsel["settings"]["readout_contrast_threshold"]["qubits"] = qctjs;
      }
      jsel["settings"]["shot_fulfilment_strategy"] = "exact";
      jsel["init"] = init;

      // Circuit
      // jsel["circuit"] is built from a visitor
      // Create the Instruction Visitor that is going to map the IR.
      // Here we select the visitor according to the model of hardware, in particular its native gateset.
      auto visitor_no_meas = make_visitor<xacc::quantum::visitor_CZ, xacc::quantum::visitor_ACZ>(model, buffer->size());
      InstructionIterator it(functions[0]);
      order_of_m.clear();
      while (it.hasNext())
      {
        // Get the next node in the tree
        auto nextInst = it.next();
        if (nextInst->isEnabled())
        {
          if (nextInst->name() == "Measure")
          {
            order_of_m.push_back(nextInst->bits()[0]);
          }
          else
          {
            nextInst->accept(visitor_no_meas);
          }
        }
      }
      jsel["circuit"] = json::parse(visitor_no_meas->getXasmString());
      if (jsel["circuit"].is_null()) jsel["circuit"] = json::array({});

      // Measurements
      // jsel["measure"] is built from iterating the IR
      json measjs;
      for (uint ii = 0; ii < order_of_m.size(); ii++)
      {
        measjs.push_back({order_of_m.at(ii), ii});
      }
      jsel["measure"] = measjs;
      return jsel.dump();
    }

    // Handle the response to the initial POST (circuit submission)
    void qdk::processResponse(std::shared_ptr<AcceleratorBuffer> , const std::string &response)
    {
      if (debug) std::cout << "* Response from HTTP POST: " << response << std::endl;
      circuit_id = json::parse(response)["id"].get<uint>();
      if (debug)
      {
        std::string path = circuitEndpoint + "/" + std::to_string(circuit_id);
        std::cout << "* POST done - poll for results at path: " << remoteUrl << path << std::endl;
      }
      return;
    }


    // Polling for circuit execution results via HTTP GET
    bool qdk::resultsReady(std::map<std::string, int> &counts)
    {
      std::string path = circuitEndpoint + "/" + std::to_string(circuit_id);
      if (debug) std::cout << "* Poll for results at path: " << remoteUrl << path << std::endl;
      json fromqdk = json::parse(Get(remoteUrl, path, http_header));
      auto data = fromqdk["data"];
      if (data == nullptr) return false;

      // Tally up the results
      int shots_tallied = 0;
      for (auto &el : data)
      {
        if (shots_tallied < shots)
        {
          std::stringstream current_state;
          for (auto &el_it : el) current_state << el_it;
          std::string bitString = current_state.str();
          if (counts.find(bitString) != counts.end())
          {
            counts[bitString]++;
            shots_tallied++;
          }
          else
          {
            counts.insert(std::make_pair(bitString, 1));
            shots_tallied++;
          }
        }
      }
      return true;
    }


    // Poll QB hardware for circuit results
    std::map<std::string, int> qdk::poll_for_results()
    {
      std::map<std::string, int> counts;
      for (int i = 0; i < poll_retries; i++)
      {
        std::this_thread::sleep_for(std::chrono::duration<double, std::chrono::seconds::period>(poll_secs));
        if (debug) std::cout << "# Waited for " << poll_secs << " seconds" << std::endl;
        bool success = resultsReady(counts);
        if (debug) std::cout << "# Poll return: " << (success ? "": "not ") << "ready" << std::endl;
        if (success) break;
      }
      return counts;
    }

  }
}
