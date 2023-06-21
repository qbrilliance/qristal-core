// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <fstream>

namespace qb {
/// Parse QPU configuration JSON file.
void session::parse_qpu_config_json(const std::string &in_file_path) {
  if (debug_)
    std::cout << "[debug]: JSON configuration for QB hardware in file: " << in_file_path
              << std::endl;
  std::ifstream tifs(in_file_path);
  if (tifs.is_open()) {
    std::string config_buf(std::istreambuf_iterator<char>(tifs), {});
    json config = json::parse(config_buf);
    if (config.count("accs")) {
      VALID_QB_HARDWARE_URLS.clear();
      VALID_QB_HARDWARE_POLLING_SECS.clear();
      VALID_QB_HARDWARE_POLLING_RETRY_LIMITS.clear();
      VALID_QB_HARDWARE_OVER_REQUEST_FACTORS.clear();
      VALID_QB_HARDWARE_RESAMPLE_ENABLEDS.clear();
      VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS.clear();
      VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES.clear();
      // Add more hardware config fields here

      for (auto iit = config["accs"].begin(); iit != config["accs"].end();
           ++iit) {
        json tkv = iit.value();
        VALID_QB_HARDWARE_URLS[tkv["acc"]] = tkv["url"];
        VALID_QB_HARDWARE_POLLING_SECS[tkv["acc"]] = tkv["poll_secs"];
        VALID_QB_HARDWARE_POLLING_RETRY_LIMITS[tkv["acc"]] = tkv["poll_retrys"];
        VALID_QB_HARDWARE_OVER_REQUEST_FACTORS[tkv["acc"]] =
            tkv["over_request"];
        VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS[tkv["acc"]] =
            tkv["recursive_request"];
        VALID_QB_HARDWARE_RESAMPLE_ENABLEDS[tkv["acc"]] = tkv["resample"];
        VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES[tkv["acc"]] =
            tkv["resample_above_percentage"];
        // Add more hardware config fields (similar to above) here
      }
      if (debug_) {
        std::cout << "* Final VALID_QB_HARDWARE_URLS:" << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_URLS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_POLLING_SECS:" << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_POLLING_SECS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_POLLING_RETRY_LIMITS:"
                  << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_POLLING_RETRY_LIMITS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_OVER_REQUEST_FACTORS:"
                  << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_OVER_REQUEST_FACTORS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS:"
                  << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_RESAMPLE_ENABLEDS:"
                  << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_RESAMPLE_ENABLEDS) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout << "* Final VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES:"
                  << std::endl;
        for (const auto &it : VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES) {
          std::cout << it.first << " : " << it.second << std::endl;
        }
      }
    }

    if (debug_)
      std::cout << "[debug]: " << std::endl << std::endl;
  } else {
    if (debug_)
      std::cout << "[debug]: Could not find the JSON configuration file for QB "
                   "hardware named: "
                << in_file_path << std::endl;
  }
}
} // namespace qb