// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/backend_utils.hpp>

// STL
#include <cstdlib>
#include <regex>


namespace qristal
{

  namespace setting
  {

    // Recursively dereference all environment variables in a string
    std::string substitute_environment_vars(std::string s)
    {
      // Keep looping until there are no more environment variables left
      while (s.find("$") != std::string::npos)
      {
        // Extract the environment variable.  If the $ is followed by opening curly braces, read only to their close
        const std::regex rgx1("\\$\\{([^\\s\\$]+?)\\}"), rgx2("\\$(\\S+)");
        std::smatch matches;
        // Try the brace-enclosed form first
        std::regex_search(s, matches, rgx1);
        // No hits with braces.  Next try without braces.
        if (matches.empty())
        {
          std::regex_match(s, matches, rgx2);
          if (matches.empty()) throw std::runtime_error("Badly formed expression in backend database YAML file: " + s);
        }
        // Dereference the environment variable
        char* val = std::getenv(matches.str(1).c_str());
        if (val == NULL)
        {
          throw std::runtime_error("Environment variable " + matches.str(1) + " referenced in backend database YAML file is not set.");
        }
        // Replace the environment variable with its value in the input string
        s.replace(matches.position(0), matches.length(0), val);
      }
      return s;
    }

  }

}
