// Copyright (c) Quantum Brilliance Pty Ltd

// QB
#include "qb/core/backend_utils.hpp"

// STL
#include <cstdlib>
#include <regex>


namespace qb
{

  // Forward declarations of all backend option-setting functions
  void add_qb_hardware_options(xacc::HeterogeneousMap&, YAML::Node& be_info, const run_i_j_config&);
  void add_aws_braket_options(xacc::HeterogeneousMap&, YAML::Node& be_info, const run_i_j_config&);

  namespace setting
  {

    // Recursively dereference all environment variables in a string
    std::string substitute_environment_vars(std::string s)
    {
      // Keep looping until there are no more environment variables left
      while (s.find("$") != std::string::npos) 
      {
        // Extract the environment variable.  If the $ is followed by opening curly braces, read only to their close
        std::regex rgx1("\$\{(.+?)\}"), rgx2("\$(.+)");
        std::smatch matches;
        std::regex_match(s, matches, rgx1);
        bool has_braces = true;
        if (matches.empty())
        {
          has_braces = false;
          std::regex_match(s, matches, rgx2);
        }
        std::cout << matches[0].str() << std::endl;
        char* val = std::getenv(matches[0].str().c_str());
        if (val == NULL)
        {
          throw std::runtime_error("Environment variable "+ matches[0].str() +" referenced in backend database YAML file is not set.");
        }
        std::regex_replace(s, (has_braces ? rgx1 : rgx2), val);      
      }
      return s;
    }

  }

  // Combine all backend options into a dict (xacc::HeterogeneousMap)
  // Note: this dict is a 'kitchen sink' of all configurations.
  // The xacc::Accelerator may or may not use these configurations.
  xacc::HeterogeneousMap backend_config(const YAML::Node& rbdb, const run_i_j_config& run_config)
  { 
    xacc::HeterogeneousMap m;

    // Generic options
    m.insert("n_qubits", static_cast<size_t>(run_config.num_qubits));
    m.insert("noise-model", run_config.noise_model.to_json());
    m.insert("noise-model-name", run_config.noise_model.name);
    m.insert("m_connectivity", run_config.noise_model.get_connectivity());
    m.insert("shots", run_config.num_shots);
    m.insert("output_oqm_enabled", run_config.oqm_enabled);

    // User-provided random seed
    if (run_config.simulator_seed.has_value())
    {
      m.insert("seed", run_config.simulator_seed.value());
    }

    // Attempt to get the entry from the remote backend yaml file corresponding to the user's chosen backend
    YAML::Node be_info = rbdb[run_config.acc_name];

    // If successful, use it to populate the remote backend settings
    if (be_info)
    {     
      // Backend-specific options
      add_aws_braket_options(m, be_info, run_config);
      add_qb_hardware_options(m, be_info, run_config);
    }
    
    return m;
  }

}
