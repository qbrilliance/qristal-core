// Copyright (c) Quantum Brilliance Pty Ltd

// QB
#include "qb/core/backend_utils.hpp"

namespace qb
{

  /// Set QB hardware options
  void add_qb_hardware_options(xacc::HeterogeneousMap& m, 
                               YAML::Node& y, 
                               const run_i_j_config& run_config)
  {                       
    using namespace setting;

    // Add a trailing slash to the URL if it doesn't already have one
    if (y["url"])
    {
      std::string url = y["url"].as<std::string>();
      if (url.back() != '/') y["url"] = url + "/";
    }
    
    // Base settings 
    required<std::string>("url", y, m);
    required<double>("poll_secs", y, m);
    required<uint>("poll_retries", y, m);    
    required<uint>("over_request", y, m);
    required<bool>("recursive_request", y, m);
    required<bool>("resample", y, m);
    required<double>("resample_above_percentage", y, m);
    optional<std::string>("post_path", "", y,  m);
    optional<bool>("exclusive_access", false, y, m); 
    optional<std::vector<uint>>("init", std::vector<uint>(run_config.num_qubits, 0), y, m);
    optional<uint>("cycles", 1, y, m);
    optional<bool>("use_default_contrast_settings", true, y, m);

    // Options setting the balanced SSR contrast below which a shot will be ignored.
    if (not m.get<bool>("use_default_contrast_settings"))
    {
      const std::string why_required = "Initialisation and readout (qubit) contrasts must "
                                       "both be specified if use_default_contrast_settings = false.";
      // Applies during initialisation.  0.6 is the usable upper bound.  Hardware default is 0.1.
      required<double>("init_contrast_threshold", y, m);

      // Applies on a per-qubit basis during final readout. Best case is ~0.3, unusable when <0.05.  
      // Indexing of this list matches to the index of qubits.  Hardware defaults are 0.1.
      required<std::map<int,double>>("qubit_contrast_thresholds", y, m);

      // Make sure all contrasts are between 0 and 1 
      check_range("init_contrast_threshold", m.get<double>("init_contrast_threshold"), {0,1});
      for (const auto& contrast : m.get<std::map<int,double>>("qubit_contrast_thresholds"))
      {
        check_range("qubit_contrast_threshold index " + contrast.first, contrast.second, {0,1});
      }
    }

    // Additional option needed for successfully using exclusive access mode
    if (m.get<bool>("exclusive_access"))
    {
      const std::string why_required = "Required if exclusive_access = true.";
      required<std::string>("exclusive_access_token", y, m, why_required);
    }

  }

}
