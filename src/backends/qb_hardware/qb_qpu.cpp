// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include "qb/core/backends/qb_hardware/qb_qpu.hpp"

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

namespace qb
{

  // Hardware polling loop
  void polling_loop(
      double polling_interval,
      int polling_attempts,
      std::shared_ptr<xacc::quantum::qb_qpu> hardware_device,
      std::map<std::string, int> &counts,
      const std::vector<std::shared_ptr<xacc::CompositeInstruction>>& circuits,
      bool debug)
  {
    for (int i = 0; i < polling_attempts; i++)
    {
      std::this_thread::sleep_for(std::chrono::duration<double, std::chrono::seconds::period>(polling_interval));
      if (debug) std::cout << "# Waited for " << polling_interval << " seconds" << std::endl;
      bool success = hardware_device->resultsReady(circuits, counts, polling_interval, polling_attempts);
      if (debug) std::cout << "# Poll return: " << (success ? "": "not ") << "ready" << std::endl;
      if (success) break;
    }
  }    

  // Send a circuit for execution on QB hardware
  void execute_on_qb_hardware(
      std::shared_ptr<xacc::quantum::qb_qpu> hardware_device,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer,
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
      const run_i_j_config &run_config,
      bool debug)
  {   
    // Send circuit for execution to QB hardware
    hardware_device->setup_hardware();
    hardware_device->execute(buffer, circuits);

    // Poll QB hardware for circuit results
    auto hardware_settings = hardware_device->getProperties();
    double polling_interval = hardware_settings.get<double>("poll_secs");
    int polling_attempts = hardware_settings.get<uint>("poll_retries");
    std::map<std::string, int> counts;
    polling_loop(polling_interval, polling_attempts, hardware_device, counts, circuits, debug);
    
    // Store the counts in the buffer
    for (auto &kv : counts)
    {
      buffer->appendMeasurement(kv.first, kv.second);
      if (debug) std::cout << "State: " << kv.first << " has count: " << kv.second << std::endl;
    }
  }
}


namespace xacc
{
  namespace quantum
  {

    // Overrides of XACC RemoteAccelerator functionalities

    // Getters
    const std::string qb_qpu::get_qbjson() const
    {
      return qbjson;
    }

    const std::string qb_qpu::getSignature() 
    {
      return name() + ":";
    }

    const std::string qb_qpu::name() const 
    {
      return "QB hardware";
    }

    const std::string qb_qpu::description() const 
    {
      return "The QB QPU backend interacts with QB hardware.";
    }
       
    // Indicate that this is indeed a remote XACC accelerator
    bool qb_qpu::isRemote() 
    {
      return true;
    }
    
    // Retrieve the properties of the backend
    HeterogeneousMap qb_qpu::getProperties() 
    {
      HeterogeneousMap m;
      m.insert("command", command);
      m.insert("init", init);
      m.insert("n_qubits", n_qubits);
      m.insert("shots", shots);
      m.insert("poll_secs", poll_secs);
      m.insert("poll_retries", poll_retries);
      m.insert("use_default_contrast_settings", use_default_contrast_settings);
      m.insert("init_contrast_threshold", init_contrast_threshold);
      m.insert("qubit_contrast_thresholds", qubit_contrast_thresholds);
      m.insert("results", results);
      m.insert("url", remoteUrl);
      m.insert("recursive", recursive);
      m.insert("resample", resample);
      m.insert("exclusive_access", exclusive_access);
      m.insert("exclusive_access_token", exclusive_access_token);
      return m;
    }
        
    // Get the available configuration settings
    const std::vector<std::string> qb_qpu::configurationKeys() 
    {
      return
      {
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
        "recursive",
        "resample",
        "exclusive_access",
        "exclusive_access_token"
      };
    }

    // Change the configuration of QB hardware
    void qb_qpu::updateConfiguration(const HeterogeneousMap &config) 
    {
      auto update = [&]<typename T>(const std::string& key, T& var)
      {
        if (config.keyExists<T>(key)) var = config.get<T>(key);
      };
      update("command", command);
      update("init", init);
      update("shots", shots);
      update("n_qubits", n_qubits);
      update("poll_secs", poll_secs);
      update("poll_retries", poll_retries);
      update("use_default_contrast_settings", use_default_contrast_settings);
      update("init_contrast_threshold", init_contrast_threshold);
      update("qubit_contrast_thresholds", qubit_contrast_thresholds);
      update("results", results);
      update("url", remoteUrl);
      update("recursive", recursive);
      update("resample", resample);
      update("exclusive_access", exclusive_access);
      update("exclusive_access_token", exclusive_access_token);
    }

    // Initialize the configuration of QB hardware
    void qb_qpu::initialize(const HeterogeneousMap &params) 
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
        qb_qpu& qpu,
        bool debug)
    {
      std::string Response;
      bool succeeded = false;
    
      // Execute HTTP operation
      try
      {
        if (debug) std::cout << "* qb_qpu::" << operation << " to " << remoteUrl << path << std::endl;

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
          case 200:
            Response = (r.text == "null" ? response_is_status_code(r.status_code) : r.text);
            break;
          case 401:
            throw std::runtime_error("Token rejected: expired token or incorrect shared secret.");
            break;
          case 403:
            throw std::runtime_error("Token rejected: http header format invalid.");
            break;
          case 404:
            throw std::runtime_error("QB hardware received an invalid command");
            break;
          case 425: // Continue polling
            Response = response_is_status_code(r.status_code);
            break;
          case 500:
            throw std::runtime_error("QB hardware process failure");
            break;
          case 503:
            throw std::runtime_error("Token rejected. QB hardware already reserved using a different token.");
            break;
          default:
            qpu.cancel();
            throw std::runtime_error("Device " + qpu.name() + " failed HTTP " + 
             operation + ". Return code: " + std::to_string(r.status_code));
            break;
        }
      }
      catch (std::exception &e)
      {
        if (std::string(e.what()).find("Caught CTRL-C") != std::string::npos)
        {
          qpu.cancel();
          throw std::runtime_error(std::string(e.what()));
        }
        throw std::runtime_error(qpu.name() + " raised exception in " + 
         operation + ": " + std::string(e.what()));    
      }
      return Response;
    }

    // HTTP POST specialisation 
    std::string qb_qpu::Post(
        const std::string &remoteUrl, const std::string &path,
        const std::string &postStr, std::map<std::string, std::string> headers)
    {
      auto f = [&](cpr::Url a, cpr::Header b) { return cpr::Post(a, cpr::Body(postStr), b, cpr::VerifySsl(false)); };
      return HTTP("POST", f, remoteUrl, path, headers, *this, debug);
    }

    // HTTP GET specialisation 
    std::string qb_qpu::Get(
        const std::string &remoteUrl, const std::string &path,
        std::map<std::string, std::string> headers,
        std::map<std::string, std::string> extraParams)
    {
      cpr::Parameters cprParams;
      for (auto &kv : extraParams) cprParams.AddParameter({kv.first, kv.second});
      auto f = [&](cpr::Url a, cpr::Header b) { return cpr::Get(a, b, cprParams, cpr::VerifySsl(false)); };
      return HTTP("GET", f, remoteUrl, path, headers, *this, debug);
    }

    // HTTP PUT specialisation 
    std::string qb_qpu::Put(
        const std::string &remoteUrl, const std::string &path,
        const std::string &putStr, std::map<std::string, std::string> headers)
    {
      auto f = [&](cpr::Url a, cpr::Header b) { return cpr::Put(a, cpr::Body(putStr), b, cpr::VerifySsl(false)); };
      return HTTP("PUT", f, remoteUrl, path, headers, *this, debug);
    }


    // Initialise the QB hardware (reserve, get native gateset, etc.)
    void qb_qpu::setup_hardware()
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
        json fromqdk = json::parse(Get(remoteUrl, nativeGateEndpoint));
        if (debug) std::cout << "* Native gates query returned: " << fromqdk.dump() << "\n";

      }
      catch (std::exception& e)
      {
        throw std::runtime_error("Error raised during QB hardware initialisation: " + std::string(e.what()));
      }
    };


    void qb_qpu::execute(
        std::shared_ptr<AcceleratorBuffer> buffer,
        const std::vector<std::shared_ptr<CompositeInstruction>> functions)
    {
      try
      {
        int counter = 0;
        std::vector<std::shared_ptr<AcceleratorBuffer>> tmpBuffers;
        for (auto f : functions)
        {
          if (debug) std::cout << "* Executing circuit number " << counter + 1 << std::endl;
          xacc::info("QB QDK executing kernel: " + f->name());
          auto tmpBuffer = std::make_shared<AcceleratorBuffer>(buffer->name() + std::to_string(counter), buffer->size());
          qbjson = processInput(tmpBuffer, std::vector<std::shared_ptr<CompositeInstruction>>{f});
          // Output the JSON sent to the QB hardware if debug is turned on.
          if (debug) std::cout << "* JSON to be sent to QB hardware: " << std::endl << qbjson << std::endl;
          std::string responseStr = Post(remoteUrl, circuitEndpoint, qbjson, http_header);
          processResponse(tmpBuffer, responseStr);
          tmpBuffers.push_back(tmpBuffer);
          buffer->appendChild(tmpBuffer->name(), tmpBuffer);
          counter++;
        }
      }
      catch (std::exception& e)
      {
        throw std::runtime_error("Error raised: " + std::string(e.what()) + "\nThe execution on hardware of your input circuit failed.");
      }
      return;
    }
    
    // Convert a circuit to a representation that QB hardware accepts.
    //
    // Sets up QB specific metadata, visits XACC IR to construct JSON strings 
    // for the circuit +  required measurements, then combines both into the 
    // HTTP POST request body.
    const std::string qb_qpu::processInput(
        std::shared_ptr<AcceleratorBuffer> buffer,
        std::vector<std::shared_ptr<CompositeInstruction>> functions)
    {
      // QB metadata
      json jsel;
      jsel["command"] = command;
    
      // Safe operating limit enforced here
      if (shots <= QB_SAFE_LIMIT_SHOTS)
      {
          jsel["settings"]["shots"] = shots;
      }
      else
      {
          std::cout << "* The requested number of shots [" << shots << "] exceeds QB_SAFE_LIMIT_SHOTS [" << QB_SAFE_LIMIT_SHOTS
                    << "] - only QB_SAFE_LIMIT_SHOTS will be requested" << std::endl;
          jsel["settings"]["shots"] = QB_SAFE_LIMIT_SHOTS;
      }
    
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
      jsel["init"] = init;
    
      // Circuit
      // jsel["circuit"] is built from a visitor
      // Create the Instruction Visitor that is going to map the IR.
      auto visitor_no_meas = std::make_shared<xacc::quantum::qb_visitor>(buffer->size());
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
    void qb_qpu::processResponse(std::shared_ptr<AcceleratorBuffer> , const std::string &response)
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
    bool qb_qpu::resultsReady(
        const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
        std::map<std::string, int> &counts, int polling_interval,
        int polling_attempts)
    {         
      int acc_valid = 0;
      std::string path = circuitEndpoint + "/" + std::to_string(circuit_id);
      if (debug) std::cout << "* Poll for results at path: " << remoteUrl << path << std::endl;
      json fromqdk = json::parse(Get(remoteUrl, path, http_header));
      auto data = fromqdk["data"];                 
      if (data == nullptr) return false;
  
      if (not resample)
      // Tally up the results
      {
        for (auto &el : data)
        {
          if (acc_valid < shots)
          {
            std::stringstream current_state;
            for (auto &el_it : el) current_state << el_it;
            std::string bitString = current_state.str();
            if (counts.find(bitString) != counts.end())
            {
              counts[bitString]++;
              acc_valid++;
            }
            else
            {
              counts.insert(std::make_pair(bitString, 1));
              acc_valid++;
            }
          }
        }
      }
      else
      // Start of resample (sample-with-replacement) procedure
      {
        std::default_random_engine qb_rnd_gen(static_cast<long unsigned int>(time(0)));
        std::uniform_int_distribution<int> p_unif(0, data.size()-1);
        while (acc_valid < shots)
        {
          std::stringstream current_state;
          auto el = data[p_unif(qb_rnd_gen)];
          for (auto &el_it : el) current_state << el_it;
          std::string bitString = current_state.str();
          if (counts.find(bitString) != counts.end())
          {
            counts[bitString]++;
            acc_valid++;
          }
          else
          {
            counts.insert(std::make_pair(bitString, 1));
            acc_valid++;
          }
        }
      }

  
      // Start of recursive calls
      if (recursive and acc_valid != shots)
      {
        std::shared_ptr<xacc::quantum::qb_qpu> hardware_device = std::make_shared<xacc::quantum::qb_qpu>(true);
        auto next_properties = getProperties();
        next_properties.insert("shots", (shots - acc_valid));
        hardware_device->updateConfiguration(next_properties);
        if (debug)
        {
          std::cout << "# Recursive request: remote URL is "
                    << next_properties.get<std::string>("url")
                    << std::endl;
          std::cout << "# Recursive request: shots is "
                    << next_properties.get<int>("shots") << std::endl;
          std::cout << "# Recursive request: polling interval is "
                    << polling_interval << " seconds" << std::endl;
        }
        auto buffer = xacc::qalloc(n_qubits);
        hardware_device->execute(buffer, citargets);
        qb::polling_loop(polling_interval, polling_attempts, hardware_device, counts, citargets, debug);
      }
        
      return true;
    }
  
  }
}
