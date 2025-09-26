// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/backend_utils.hpp>


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
const std::unordered_map<std::string, std::pair<size_t, size_t>> AWS_QUBIT_LIMITS
{
  {"DM1", {1, 17}},
  {"SV1", {1, 34}},
  {"TN1", {1, 48}}
};

// Valid range of shots for AWS backends
const std::unordered_map<std::string, std::pair<size_t, size_t>> AWS_SHOT_LIMITS
{
  {"DM1", {1, 10000}},
  {"SV1", {1, 10000}},
  {"TN1", {1, 999}}
};

namespace qristal
{

  void add_aws_braket_options(xacc::HeterogeneousMap& m,
                              YAML::Node& y,
                              size_t num_qubits,
                              size_t num_shots)
  {
    using namespace setting;

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
      std::stringstream ss;
      ss << "Error in YAML snippet\n" << y.as<std::string>() << "\n"
         << "Noise cannot be set to True when using a hardware backend.";
      throw std::invalid_argument(ss.str());
      }
    }
    else required<bool>("noise", y, m);

    // Check that s3 starts with "amazon-braket"
    if (not m.get<std::string>("s3").starts_with("amazon-braket"))
    {
      std::stringstream ss;
      ss << "Error in YAML snippet\n" << y.as<std::string>() << "\n"
       << "The value of s3 must begin with \"amazon-braket\".";
      throw std::invalid_argument(ss.str());
    }

    // Check that requested number of qubits and shots is in range for the chosen device
    const std::string device = m.get<std::string>("device");
    check_range("qubits", num_qubits, device, AWS_QUBIT_LIMITS);
    check_range("shots", num_shots, device, AWS_SHOT_LIMITS);

  }

}
