// Copyright (c) Quantum Brilliance Pty Ltd

// QB
#include "qb/core/backend_utils.hpp"

// Valid AWS backend strings
const std::unordered_set<std::string> VALID_AWS_DEVICES =
{
  "SV1",
  "DM1",
  "TN1",
  "Rigetti"
};

// Valid AWS backend formats
const std::unordered_set<std::string> VALID_AWS_FORMATS =
{
  "braket",
  "openqasm3"
};

// Valid range of qubits for AWS backends 
const std::unordered_map<std::string, std::pair<int, int>> AWS_QUBIT_LIMITS
{
  {"DM1", {1, 17}},
  {"SV1", {1, 34}},
  {"TN1", {1, 48}}
};

// Valid range of shots for AWS backends 
const std::unordered_map<std::string, std::pair<int, int>> AWS_SHOT_LIMITS
{
  {"DM1", {1, 10000}},
  {"SV1", {1, 10000}},
  {"TN1", {1, 999}}
};

namespace qb
{

  void add_aws_braket_options(xacc::HeterogeneousMap& m, 
                              YAML::Node& y, 
                              const run_i_j_config& run_config)
  {                         
    using namespace setting;

    if (run_config.acc_name == "aws-braket")
    {
      // Read in the options from the yaml file
      restricted_required("format", y, m, VALID_AWS_FORMATS);
      restricted_required("device", y, m, VALID_AWS_DEVICES);
      required<std::string>("path", y, m);
      required<std::string>("s3", y, m);
      optional<bool>("verbatim", false, y, m);

      // Permit noise option only if using a simulator backend
      if (m.get<std::string>("device").starts_with("Rigetti"))
      {
        optional<bool>("noise", false, y, m);
        if (m.get<bool>("noise")) 
        {
          std::ostringstream err;
          err << "Error in YAML snippet " << std::endl << y << std::endl
              << "Noise cannot be set to True when using a hardware backend.";
          throw std::invalid_argument(err.str()); 
        }
      }
      else required<bool>("noise", y, m);

      // Check that s3 starts with "amazon-braket"
      if (not m.get<std::string>("s3").starts_with("amazon-braket"))
      {
        std::ostringstream err;
        err << "Error in YAML snippet " << std::endl << y << std::endl
            << "The value of s3 must begin with \"amazon-braket\".";
        throw std::invalid_argument(err.str()); 
      }

      // Check that requested number of qubits and shots is in range for the chosen device
      const std::string device = m.get<std::string>("device");
      check_range("qubits", run_config.num_qubits, device, AWS_QUBIT_LIMITS);
      check_range("shots", run_config.num_shots, device, AWS_SHOT_LIMITS);
    }

  }

}
