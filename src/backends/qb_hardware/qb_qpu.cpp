// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include "qb/core/backends/qb_hardware/qb_qpu.hpp"
#include "qb/core/backends/qb_hardware/qcstack_client.hpp"

// STL
#include <memory>
#include <sstream>
#include <random>
#include <string>
#include <thread>

// JSON
#include <nlohmann/json.hpp>
using json = nlohmann::json;

const int POLLING_NOT_READY = 300;
const int POLLING_PROCESS_FAILED = 500;
const int POLLING_SUCCESS = 0;
const int NATIVE_GATES_SUCCESS = 0;

namespace qb
{
  void execute_on_qb_hardware(
      std::shared_ptr<xacc::quantum::qb_qpu> tqdk,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
      const run_i_j_config &run_config,
      bool debug)
  {
    try
    {
      tqdk->validate_capability();
    }
    catch (...)
    {
      throw std::runtime_error("Please recheck your hardware settings");
    }

    try
    {
      tqdk->execute(buffer_b, circuits);
    }
    catch (...)
    {
      throw std::invalid_argument("The execution on hardware of your input circuit failed");
    }

    // Set up polling
    auto m = tqdk->getProperties();
    double polling_interval = m.get<double>("poll_secs");
    int polling_attempts = m.get<int>("poll_retries");
    using namespace std::chrono_literals;
    for (int i = 0; i < polling_attempts; i++)
    {
      std::this_thread::sleep_for(std::chrono::duration<double, std::chrono::seconds::period>(polling_interval));
      if (debug) std::cout << "# Waited for " << polling_interval << " seconds" << std::endl;
      int poll_return = POLLING_NOT_READY;

      // Accumulate counts in a map of string -> int
      std::map<std::string, int> counts;
      poll_return = tqdk->pollForResults(buffer_b, circuits, counts, polling_interval, polling_attempts);

      if (debug) std::cout << "# Polling returned status: " << poll_return << std::endl;

      if (poll_return == POLLING_SUCCESS) break;
    }
  }
}


namespace xacc
{
  namespace quantum
  {

    // 1. sets up HTTP POST request headers
    // 2. sets up QB specific metadata
    // 3. visits XACC IR to construct JSON strings for the circuit +  required measurements
    // 4. combines 2. and 3. into the HTTP POST request body (string)
    const std::string qb_qpu::processInput(
        std::shared_ptr<AcceleratorBuffer> buffer,
        std::vector<std::shared_ptr<CompositeInstruction>> functions)
    {
    
      // 1. HTTP POST request headers
      headers.clear();
      headers.insert(std::make_pair("Content-type", "application/json; charset=utf-8"));
      headers.insert(std::make_pair("Connection", "keep-alive"));
      // headers.insert(std::make_pair("Accept-Encoding", "gzip, deflate"));
      // headers.insert(std::make_pair("Accept", "application/octet-stream"));
    
      // 2. QB metadata
      json jsel;
      jsel["command"] = command_;
    
      // 2.1 Safe operating limit enforced here
      int shots_ov = shots_*over_request_;
      if (shots_ov <= QB_SAFE_LIMIT_SHOTS)
      {
          jsel["settings"]["shots"] = shots_ov;
      }
      else
      {
          std::cout << "* The (over-)requested number of shots [" << shots_ov << "] exceeds QB_SAFE_LIMIT_SHOTS [" << QB_SAFE_LIMIT_SHOTS
                    << "] - only QB_SAFE_LIMIT_SHOTS will be requested" << std::endl;
          jsel["settings"]["shots"] = QB_SAFE_LIMIT_SHOTS;
      }
    
      jsel["settings"]["cycles"] = cycles_;   // default: 1
      jsel["settings"]["results"] = results_; // default: "normal"
      if (!use_default_contrast_settings_)
      {
        jsel["settings"]["readout_contrast_threshold"]["init"] = init_contrast_threshold_;
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
      auto visitor_no_meas = std::make_shared<xacc::quantum::qb_visitor>(buffer->size());
      InstructionIterator it(functions[0]);
      order_of_m_.clear();
      while (it.hasNext())
      {
        // Get the next node in the tree
        auto nextInst = it.next();
        if (nextInst->isEnabled())
        {
          if (nextInst->name() == "Measure")
          {
            order_of_m_.push_back(nextInst->bits()[0]);
          }
          else
          {
            nextInst->accept(visitor_no_meas);
          }
        }
      }
      jsel["circuit"] = json::parse(visitor_no_meas->getXasmString());
      if (jsel["circuit"].is_null()) jsel["circuit"] = json::array({});
    
      // 3. Measurements
      // jsel["measure"] is built from iterating the IR
      json measjs;
      for (uint ii = 0; ii < order_of_m_.size(); ii++)
      {
        measjs.push_back({order_of_m_.at(ii), ii});
      }
      jsel["measure"] = measjs;
      return jsel.dump();
    }
    
    // Validate the capabilities of the QB hardware against what the session requires
    int qb_qpu::validate_capability()
    {  
      debug_qb_hw_ = true;
      int retval = NATIVE_GATES_SUCCESS;
      if (debug_qb_hw_)
      {
        std::cout << "* Query for native gates supported at path: " << remoteUrl << native_gates_get_path_
                  << std::endl;
      }
      json fromqdk = json::parse(qb_qpu::handleExceptionRestClientGet(remoteUrl, native_gates_get_path_, headers));
      if (debug_qb_hw_) std::cout << "* Native gates query returned: " << fromqdk.dump() << "\n";
    
      /// Validation for session configuration against hardware capabilities
    
      /// Add more validations as required below this line:
      return retval;
    };
    
    
    // Getters
    
    const std::string qb_qpu::getSignature() 
    {
      return name() + ":";
    }

    const std::string qb_qpu::name() const 
    {
      return "QB-hardware";
    }

    const std::string qb_qpu::description() const 
    {
      return "The QB QPU backend interacts with QB hardware.";
    }

    std::string qb_qpu::getNativeCode(std::shared_ptr<CompositeInstruction> program,
                                      const HeterogeneousMap &config)
    {
      size_t n_qubits = 0;
    
      if (config.keyExists<size_t>("n_qubits"))
      {
        n_qubits = config.get<size_t>("n_qubits");
        std::vector<std::shared_ptr<CompositeInstruction>> functions{program};
        auto buffer_b = xacc::qalloc(n_qubits);
        return processInput(buffer_b, functions);
      }
      else
      {
        throw std::range_error("The number of qubits [n_qubits] was not defined");
      }
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
      m.insert("command", command_);
      m.insert("init", init_);
      m.insert("n_qubits", n_qubits_);
      m.insert("shots", shots_);
      m.insert("request_id", request_id_);
      m.insert("poll_id", poll_id_);
      m.insert("poll_secs", poll_secs_);
      m.insert("poll_retries", poll_retries_);
      m.insert("use_default_contrast_settings", use_default_contrast_settings_);
      m.insert("init_contrast_threshold", init_contrast_threshold_);
      m.insert("qubit_contrast_thresholds", qubit_contrast_thresholds_);
      m.insert("cycles", cycles_);
      m.insert("results", results_);
      m.insert("hwbackend", hwbackend_);
      m.insert("url", remoteUrl);
      m.insert("post_path", postPath);
      m.insert("over_request", over_request_);
      m.insert("recursive_request", recursive_request_);
      m.insert("resample", resample_);
      m.insert("resample_above_percentage", resample_above_percentage_);
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
        "request_id",
        "poll_id",
        "poll_secs",
        "poll_retries",
        "use_default_contrast_settings",
        "init_contrast_threshold",
        "qubit_contrast_thresholds",
        "cycles",
        "results",
        "hwbackend",
        "url",
        "post_path",
        "over_request",
        "recursive_request",
        "resample",
        "resample_above_percentage"
      };
    }

    // Change the configuration of QB hardware
    void qb_qpu::updateConfiguration(const HeterogeneousMap &config) 
    {
      auto update = [&]<typename T>(const std::string& key, T& var)
      {
        if (config.keyExists<T>(key)) var = config.get<T>(key);
      };
      update("command", command_);
      update("init", init_);
      update("shots", shots_);
      update("n_qubits", n_qubits_);
      update("request_id", request_id_);
      update("poll_id", poll_id_);
      update("poll_secs", poll_secs_);
      update("poll_retries", poll_retries_);
      update("use_default_contrast_settings", use_default_contrast_settings_);
      update("init_contrast_threshold", init_contrast_threshold_);
      update("qubit_contrast_thresholds", qubit_contrast_thresholds_);
      update("cycles", cycles_);
      update("results", results_);
      update("hwbackend", hwbackend_);
      update("url", remoteUrl);
      update("post_path", postPath);
      update("over_request", over_request_);
      update("recursive_request", recursive_request_);
      update("resample", resample_);
      update("request_id", request_id_);
      update("poll_id", poll_id_);
      update("cycles", cycles_);
      update("results", results_);
      update("hwbackend", hwbackend_);
      update("resample_above_percentage", resample_above_percentage_);
    }

    // Initialize the configuration of QB hardware
    void qb_qpu::initialize(const HeterogeneousMap &params) 
    {
      updateConfiguration(params);
    }

    std::string qb_qpu::handleExceptionRestClientPost(
        const std::string &postUrl, const std::string &path,
        const std::string &postStr, std::map<std::string, std::string> headers)
    {
      std::string postResponse;
      std::exception ex;
      bool succeeded = false;
      auto m = getProperties();
      int retries = m.get<int>("poll_retries");
    
      // Execute HTTP Post
      do
      {
        try
        {
          postResponse = restClient->post(postUrl, path, postStr, headers);
          succeeded = true;
          break;
        }
        catch (std::exception &e)
        {
          ex = e;
          xacc::info("Remote Accelerator " + name() +
                     " caught exception while calling restClient->post() "
                     "- " + std::string(e.what()));
          retries--;
          if (retries > 0) xacc::info("Retrying HTTP Post.");
        }
      } while (retries > 0);
    
      if (!succeeded)
      {
        cancel();
        xacc::error("Remote Accelerator " + name() +
                    " failed HTTP Post for Job Response - " +
                    std::string(ex.what()));
      }
    
      return postResponse;
    }
    
    std::string qb_qpu::handleExceptionRestClientGet(
        const std::string &postUrl, const std::string &path,
        std::map<std::string, std::string> headers,
        std::map<std::string, std::string> extraParams)
    {
      std::string getResponse;
      std::exception ex;
      bool succeeded = false;
      auto m = getProperties();
      int retries = m.get<int>("poll_retries");
    
      // Execute HTTP Get
      do
      {
        try
        {
          getResponse = restClient->get(postUrl, path, headers, extraParams);
          succeeded = true;
          break;
        }
        catch (std::exception &e)
        {
          ex = e;
          xacc::info("Remote Accelerator " + name() +
                     " caught exception while calling restClient->get() "
                     "- " + std::string(e.what()));
          if (std::string(e.what()).find("Caught CTRL-C") != std::string::npos)
          {
            cancel();
            xacc::error(std::string(e.what()));
          }
          retries--;
          if (retries > 0) xacc::info("Retrying HTTP Get.");
        }
      } while (retries > 0);
    
      if (!succeeded)
      {
        cancel();
        xacc::error("Remote Accelerator " + name() + 
                    " failed HTTP Get for Job Response - " +
                    std::string(ex.what()));
      }
    
      return getResponse;
    }
    
    void qb_qpu::execute(
        std::shared_ptr<AcceleratorBuffer> buffer,
        const std::vector<std::shared_ptr<CompositeInstruction>> functions)
    {
      int counter = 0;
      std::vector<std::shared_ptr<AcceleratorBuffer>> tmpBuffers;
      for (auto f : functions)
      {
        if (debug_qb_hw_) std::cout << "* [DEBUG]: execute counter: " << counter << std::endl;
        xacc::info("QB QDK executing kernel: " + f->name());
        auto tmpBuffer = std::make_shared<AcceleratorBuffer>(buffer->name() + std::to_string(counter), buffer->size());
        RemoteAccelerator::execute(tmpBuffer, f);
        tmpBuffers.push_back(tmpBuffer);
        buffer->appendChild(tmpBuffer->name(), tmpBuffer);
        counter++;
      }
      return;
    }
    
    void qb_qpu::processResponse(std::shared_ptr<AcceleratorBuffer> , const std::string &response)
    {
      if (debug_qb_hw_) std::cout << "* Response from HTTP POST: " << response << std::endl;
      json respost = json::parse(response);
      auto idstr = respost["id"].get<int>();
      previous_post_path_ = postPath;
      postPath.append(std::to_string(idstr));   
      if (debug_qb_hw_) std::cout << "* POST done - poll for results at path: " << remoteUrl << postPath << std::endl;
      return;
    }
    
    int qb_qpu::pollForResults(
        std::shared_ptr<AcceleratorBuffer> buffer,
        const std::vector<std::shared_ptr<CompositeInstruction>> citargets,
        std::map<std::string, int> &counts, int polling_interval,
        int polling_attempts)
    {
      int retval = POLLING_NOT_READY;
      if (debug_qb_hw_) std::cout << "* Poll for results at path: " << remoteUrl << postPath << std::endl;
      json fromqdk = json::parse(qb_qpu::handleExceptionRestClientGet(remoteUrl, postPath, headers));
    
      std::default_random_engine qb_rnd_gen(static_cast<long unsigned int>(time(0)));
      int unif_lb = 0;
      int acc_valid = 0;
      auto m = getProperties();
      int requested_shots = m.get<int>("shots");
      int tmp_n = 0;
    
      // Accumulate counts in a map of string -> int
      // std::map<std::string, int> counts;
    
      if (fromqdk["data"] != nullptr)
      {
        int unif_ub = fromqdk["data"].size() - 1;
        if (unif_ub < 0) retval = POLLING_PROCESS_FAILED;
        std::uniform_int_distribution<int> p_unif(unif_lb, unif_ub);
    
        // Start of resample (sample-with-replacement) procedure
        if (m.get<bool>("resample"))
        {
          while (acc_valid < requested_shots)
          {
            tmp_n = p_unif(qb_rnd_gen);
            std::stringstream current_state;
            auto el = fromqdk["data"][tmp_n];
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
        else
        {
          for (auto &el : fromqdk["data"])
          {
            if (acc_valid < requested_shots)
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
    
        // Start of recursive calls
        if (acc_valid == requested_shots)
        {
          retval = POLLING_SUCCESS;
        }
        else
        {
          if (not m.get<bool>("recursive_request"))
          {
            retval = POLLING_SUCCESS;
          }
          else
          {
            // A QCStack client - provide argument 'true' for debug mode
            std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>(true);
            std::shared_ptr<xacc::quantum::qb_qpu> tqdk = std::make_shared<xacc::quantum::qb_qpu>(qcs_qdk, true);
            auto next_properties = getProperties();
            next_properties.insert("shots", (requested_shots - acc_valid));
    
            // Threshold % above which to trigger resample procedure
            if (100*acc_valid/requested_shots >= resample_above_percentage_)
            {
              if (debug_qb_hw_) std::cout << "# Recursive request: forced resampling procedure at " << (100*acc_valid/requested_shots) <<" % valid" << std::endl;
              next_properties.insert("resample", true);
              // Increase the over_request factor for the final request to minimise the 
              // chance of an empty reply from the QDK
              next_properties.insert("over_request", m.get<int>("over_request")*8);
            }
    
            next_properties.insert("post_path", previous_post_path_);
            if (debug_qb_hw_)
            {
              std::cout << "# Recursive request: remote URL is "
                        << next_properties.get<std::string>("url")
                        << std::endl;
              std::cout << "# Recursive request: post path is "
                        << next_properties.get<std::string>("post_path")
                        << std::endl;
              std::cout << "# Recursive request: shots is "
                        << next_properties.get<int>("shots") << std::endl;
            }
            tqdk->updateConfiguration(next_properties);
            auto buffer_b = xacc::qalloc(m.get<size_t>("n_qubits"));
            if (debug_qb_hw_) std::cout << "# Recursive request: polling interval is "
                                        << polling_interval << " seconds" << std::endl;

            try
            {
              tqdk->execute(buffer_b, citargets);
            }
            catch (...)
            {
              throw std::invalid_argument("The execution on hardware of your input circuit failed");
            }
    
            // Set up polling
            using namespace std::chrono_literals;
            for (int i = 0; i < polling_attempts; i++)
            {
              std::this_thread::sleep_for(std::chrono::seconds(polling_interval));
              if (debug_qb_hw_) std::cout << "# Waited for " << polling_interval << " seconds" << std::endl;
              int poll_return = POLLING_NOT_READY;
              poll_return = tqdk->pollForResults(buffer_b, citargets, counts, polling_interval, polling_attempts);
              if (debug_qb_hw_) std::cout << "# Poll return: " << poll_return << std::endl;
              retval = poll_return;
              if (retval == POLLING_SUCCESS) break;
            }
          }
        }
    
        // Returned from recursive call...
        // now proceed to store the counts in the buffer
        for (auto &kv : counts)
        {
          buffer->appendMeasurement(kv.first, kv.second);
          if (debug_qb_hw_) std::cout << "State: " << kv.first << " has count: " << kv.second << std::endl;
        }
        retval = POLLING_SUCCESS;
      }
      else
      {
        std::cout << "* No 'data' found..." << std::endl;
        retval = POLLING_NOT_READY;
      }
      return retval;
    }
  
  }
}
