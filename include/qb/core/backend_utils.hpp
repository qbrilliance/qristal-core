// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// STL
#include <string>
#include <optional>
#include <unordered_set>
#include <sstream>

// YAML
#include "yaml-cpp/yaml.h"

// XACC
#include "heterogeneous.hpp"

// QB
#include "qb/core/session_utils.hpp"


namespace qb
{

  /**
  * @brief Combine all backend options into a dict (xacc::HeterogeneousMap).
  * 
  * @return A xacc::HeterogeneousMap containing the settings for the backend in use. 
  */
  xacc::HeterogeneousMap backend_config(const YAML::Node& rbdb, const run_i_j_config &run_config);


  namespace setting
  {

      // longer term:
      //       - replace xacc::HeterogenousMap with std::map<std::string, std::variant<int,double,std::string>> 
  
    /**
    * @brief Recursively dereference environment variables in a string
    * 
    * @return A std::string with all environment variables expanded. 
    * 
    */
    std::string substitute_environment_vars(std::string s);
  
    /// Check that a given value is in range
    template<typename T>
    void check_range(const std::string& name, const T& val, const std::pair<T, T>& limits)
    {
      if (val < limits.first or val > limits.second)
      {
        std::ostringstream err;
        err << "Valid range for " << name << " not respected." << std::endl
            << name << ": " << val << std::endl << "Valid range: " << limits.first << "-" << limits.second;
        throw std::invalid_argument(err.str());
      }
    }

    /// Check that a given value is in range for a given component
    template<typename T>
    void check_range(const std::string& name, const T& val, const std::string& component, 
                     const std::unordered_map<std::string, std::pair<T, T>>& limits)
    {
      if (limits.find(component) != limits.end())
      {
        check_range(name + " with " + component, val, limits.at(component));
      }
    }

    template<typename T>
    T get_option_from_yaml(const std::string& key, YAML::Node& y)
    {
      // Attempt to extract the value as a string 
      std::optional<std::string> val_as_str = std::nullopt;
      try
      {
        val_as_str = y[key].as<std::string>();
      }
      // No error; if the value can't be converted to a string for some reason, we just don't bother with environment variable substitution.      
      catch(...) {}

      // Dereference any environment variables contained in the value, then write it back into the yaml node
      try
      {
        if (val_as_str) y[key] = substitute_environment_vars(val_as_str.value()); 
      }
      catch(std::runtime_error& e)
      {
        std::ostringstream err;
        err << e.what() << std::endl << "YAML entry for chosen backend: " << std::endl << y;
        throw std::runtime_error(err.str()); 
      }
      
      // Following environment variable expansion, attempt to re-extract the value with the right type
      T val;
      try
      {
        val = y[key].as<T>();
      }
      catch(...)
      {
        std::ostringstream err;
        err << "Unable to convert YAML entry " << key << " to requested type in backend YAML entry" << std::endl << y;
        throw std::runtime_error(err.str()); 
      }
  
      return val;     
    }
  
    template<typename T>
    void required(const std::string& key, 
                  YAML::Node& y, 
                  xacc::HeterogeneousMap& m, 
                  std::optional<std::reference_wrapper<const std::string>> explanation = std::nullopt)
    {
      // Check if the key is contained in the yaml node
      if (not y[key])
      {
        std::ostringstream err;
        err << "Required YAML parameter " << key << " not present in backend database yaml file for selected backend.";
        if (explanation) err << std::endl << explanation.value().get();
        err << std::endl << "YAML entry for chosen backend: " << std::endl << y;
        throw std::runtime_error(err.str()); 
      }   
      // Get the value from the yaml node and put it into the map
      m.insert(key, get_option_from_yaml<T>(key, y));
    }
  
    template<typename T>
    void restricted_required(const std::string& key, 
                  YAML::Node& y, 
                  xacc::HeterogeneousMap& m, 
                  const std::unordered_set<T> valid_values,
                  std::optional<std::reference_wrapper<const std::string>> explanation = std::nullopt)
    {
      required<T>(key, y, m, explanation);
      // Check that the resulting value is one of the valid ones
      if (valid_values.find(m.get<T>(key)) == valid_values.end())
      {
        std::ostringstream err;
        err << "Required YAML parameter " << key << " given invalid value: " << m.get<T>(key);
        if (explanation) err << std::endl << explanation.value().get();
        err << std::endl << "Allowed values:";
        for (auto val : valid_values) err << std::endl << "  " << val; 
        err << std::endl << "YAML entry for chosen backend: " << std::endl << y;
        throw std::runtime_error(err.str()); 
      }
    }
  
    template<typename T>
    void optional(const std::string& key, 
                  const T default_val, 
                  YAML::Node& y, 
                  xacc::HeterogeneousMap& m)
    {
      // Start off by checking if the key is contained in the yaml node
      if (y[key])
      {
        // Get the value from the yaml node and put it into the map
        m.insert(key, get_option_from_yaml<T>(key, y));
      }
      else
      {            
        // If not, just use the passed default
        m.insert(key, default_val);
      }
    }
  
  
    template<typename T>
    void restricted_optional(const std::string& key, 
                  const T default_val, 
                  YAML::Node& y, 
                  xacc::HeterogeneousMap& m,
                  const std::unordered_set<T> valid_values)
    {
      optional(key, default_val, y, m);
      // Check that the resulting value is one of the valid ones
      if (valid_values.find(m.get<T>(key)) == valid_values.end())
      {
        std::ostringstream err;
        err << "Optional YAML parameter " << key << " given invalid value: " << m.get<T>(key);
        err << std::endl << "Allowed values:";
        for (auto val : valid_values) err << std::endl << "  " << val; 
        err << std::endl << "YAML entry for chosen backend: " << std::endl << y;
        throw std::runtime_error(err.str()); 
      }
    }

  }

}
