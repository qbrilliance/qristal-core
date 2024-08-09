// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include <fstream>

namespace {
// Trim from left (remove leading white spaces, e.g., spaces, tab, empty lines)
inline std::string ltrim(const std::string &in_str,
                         const char *t = " \t\n\r\f\v") {
  auto s = in_str;
  s.erase(0, s.find_first_not_of(t));
  return s;
}
} // namespace
namespace qristal {

/// Core session methods: run/run_async executions and helpers for
/// compilation/post-processing...
session::circuit_input_types
session::validate_infiles_instrings_randoms_irtarget_ms_nonempty(
    const size_t ii, const size_t jj) {
  session::circuit_input_types returnval =
      session::circuit_input_types::INVALID;
  if (debug_) {
    std::cout << "[debug]:[start of "
                 "validate_infiles_instrings_randoms_irtarget_ms_nonempty]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: " << std::endl;
  }

  // Helper to check if a 2-D array is empty
  const auto check_empty_2d_array_at_ii = [ii](const auto &array_2d) -> bool {
    if ((!array_2d.empty()) &&
        (array_2d.size() >= static_cast<size_t>(ii + 1))) {
      if (!array_2d.at(ii).empty()) {
        // return array_2d.at(ii).at(0).empty();
        return false;
      }
    }
    return true;
  };

  const bool is_infiles_empty = check_empty_2d_array_at_ii(infiles_);
  if (debug_) {
    std::cout << "[debug]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: is_infiles_empty = " << is_infiles_empty << std::endl;
  }

  const bool is_instrings_empty = check_empty_2d_array_at_ii(instrings_);
  if (debug_) {
    std::cout << "[debug]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: is_instrings_empty = " << is_instrings_empty << std::endl;
  }

  const bool is_randoms_empty = check_empty_2d_array_at_ii(randoms_);
  if (debug_) {
    std::cout << "[debug]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: is_inrandoms_empty = " << is_randoms_empty << std::endl;
  }

  const bool is_irtarget_m_empty = check_empty_2d_array_at_ii(irtarget_ms_);
  if (debug_) {
    std::cout << "[debug]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: is_irtarget_m_empty = " << is_irtarget_m_empty
              << std::endl;
  }

  const bool is_cudaq_empty = cudaq_kernels_.empty();
  if (debug_) {
    std::cout << "[debug]:"
              << "[circuit: " << ii << ", condition: " << jj
              << "]: is_cudaq_empty = " << is_cudaq_empty << std::endl;
  }

  if (is_infiles_empty && is_instrings_empty && is_randoms_empty &&
      is_irtarget_m_empty && is_cudaq_empty) {
    throw std::invalid_argument(
        "session: at least one of these must have a "
        "value: infile | instring | random | irtarget_m | cudaq ");
  }

  // 1.1 Check if "__qpu" occurs at the start of instrings_
  if (!is_instrings_empty) {
    std::string temp_instr = (instrings_[ii])[0];
    // Remove leading/trailing white spaces
    xacc::trim(temp_instr);
    // Check if the string starts with "__qpu"
    if (temp_instr.find("__qpu") == 0) {
      // instring is a quantum circuit kernel
      if (debug_) {
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: __qpu detected - instring contains a quantum kernel"
                  << std::endl;
      }
      return session::circuit_input_types::VALID_INSTRING_QPU;
    } else if (ltrim((instrings_[ii])[0]).rfind("OPENQASM", 0) == 0) {
      // Check for raw OpenQASM string.
      // Source string starts with "OPENQASM" (not already being wrapped in
      // __qpu__)
      const std::string qbstr =
          convertRawOpenQasmToQBKernel((instrings_[ii])[0]);
      if (debug_) {
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: Raw OpenQASM source detected: \n"
                  << (instrings_[ii])[0]
                  << "\nConverted to a QB quantum kernel:\n"
                  << qbstr << std::endl;
      }
      (instrings_[ii])[0] = qbstr;
      return session::circuit_input_types::VALID_INSTRING_QPU;
    } else {
      // instring is a text description
      if (debug_) {
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: " << temp_instr << std::endl;
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: instrings_ will be used as description text"
                  << std::endl;
      }
    }
  }

  //  1.2 Check:   (0 < randoms_ <= RANDOMS_UPPERBOUND) ? proceed to random
  //  circuit generation, execution and transpilation.  Ignore infiles_
  if (!is_randoms_empty) {
    if (((randoms_[ii])[0] > 0) &&
        ((randoms_[ii])[0] <= session::RANDOMS_UPPERBOUND)) {
      returnval = session::circuit_input_types::VALID_RANDOM;
      return returnval;
    } else {
      // instring is a text description
      if (debug_) {
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: randoms_ = " << (randoms_[ii])[0]
                  << " is outside the valid range." << std::endl;
        throw std::range_error("session: random exceeds valid limits");
      }
    }
  }

  // 1.3 Check:   infiles_ exists as a file - try to open the path to the files
  // and check for errors
  if (!is_infiles_empty) {
    std::ifstream tifs((infiles_[ii])[0]);
    if (tifs.is_open()) {
      if (debug_) {
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: Opened input file named: " << (infiles_[ii])[0]
                  << std::endl;
        std::string outbuf(std::istreambuf_iterator<char>(tifs), {});
        std::cout << "[debug]:"
                  << "[circuit: " << ii << ", condition: " << jj
                  << "]: " << std::endl
                  << outbuf << std::endl;
      }
      return session::circuit_input_types::VALID_INFILE;
    } else {
      std::string error_message("Input file not found: ");
      throw std::invalid_argument(error_message += (infiles_[ii])[0]);
    }
  }

  if (!is_irtarget_m_empty) {
    if (debug_) {
      std::cout << "[debug]:"
                << "[circuit: " << ii << ", condition: " << jj
                << "]: " << std::endl
                << " has a XACC IR target" << std::endl;
    }
    return session::circuit_input_types::VALID_IR;
  }

  // CUDAQ input
  if (!is_cudaq_empty) {
    if (debug_) {
      std::cout << "[debug]:"
                << "[circuit: " << ii << ", condition: " << jj
                << "]: " << std::endl
                << " has a CUDAQ target" << std::endl;
    }
    return session::circuit_input_types::VALID_CUDAQ;
  }

  return returnval;
}

int session::validate_instrings() {
  const int VALID_ALL = 0;
  // const int VALID_PARTIAL = 1;
  // const int INVALID = -1;

  // bool is_instrings_empty;

  int returnval = VALID_ALL;
  if (debug_) {
    std::cout << "[debug]:"
              << "Checking for valid instrings..." << std::endl;
  }
  return returnval;
}

int session::validate_sns_nonempty() {
  const int VALID_ALL = 0;
  // const int VALID_PARTIAL = 1;
  const int INVALID = -1;

  bool is_sns_empty = true;

  int returnval = VALID_ALL;
  if (debug_) {
    std::cout << "[debug]:"
              << "Checking for valid settings for the number of shots..."
              << std::endl;
  }
  if (!sns_.empty()) {
    is_sns_empty =
        std::all_of(sns_.cbegin(), sns_.cend(),
                    [](std::vector<size_t> sn) { return sn.empty(); });
    if (debug_ && is_sns_empty) {
      std::cout << "[debug]:"
                << "sns_ is empty" << std::endl;
    }
  }
  if (is_sns_empty) {
    throw std::invalid_argument(
        "session: number of shots [sn] must have a value");
    returnval = INVALID;
  }
  return returnval;
}

int session::validate_qns_nonempty() {
  const int VALID_ALL = 0;
  // const int VALID_PARTIAL = 1;
  const int INVALID = -1;

  bool is_qns_empty = true;

  int returnval = VALID_ALL;
  if (debug_) {
    std::cout << "[debug]:"
              << "Checking for valid settings for the number of qubits..."
              << std::endl;
  }
  if (!qns_.empty()) {
    is_qns_empty =
        std::all_of(qns_.cbegin(), qns_.cend(),
                    [](std::vector<size_t> qn) { return qn.empty(); });
    if (debug_ && is_qns_empty) {
      std::cout << "[debug]:"
                << "qns_ is empty" << std::endl;
    }
  }
  if (is_qns_empty) {
    throw std::invalid_argument(
        "session: number of qubits [qn] must have a value");
    returnval = INVALID;
  }
  return returnval;
}

int session::is_ii_consistent() {
  const int INVALID = -1;
  // const int SINGLETON = 1;
  size_t N_ii = 0;

  // Find largest size amongst:
  //  infiles_,
  //  instrings_,
  //  randoms_,
  //  irtarget_ms_

  if (infiles_.size() >= 1) {
    N_ii = infiles_.size();
  }
  if (instrings_.size() >= 1) {
    if (instrings_.size() > N_ii) {
      N_ii = instrings_.size();
    }
  }
  if (randoms_.size() >= 1) {
    if (randoms_.size() > N_ii) {
      N_ii = randoms_.size();
    }
  }
  if (irtarget_ms_.size() >= 1) {
    if (irtarget_ms_.size() > N_ii) {
      N_ii = irtarget_ms_.size();
    }
  }

  if ((N_ii = singleton_or_eqlength(accs_, N_ii)) == INVALID) {
    std::cout << "[acc] shape is invalid" << std::endl;
    return INVALID;
  }
  if ((N_ii = singleton_or_eqlength(noises_, N_ii)) == INVALID) {
    std::cout << "[noise] shape is invalid" << std::endl;
    return INVALID;
  }

  return N_ii;
}

int session::is_jj_consistent() {
  const int INVALID = -1;
  const int SINGLETON = 1;
  int N_jj = SINGLETON;
  for (auto el : infiles_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[infile] shape is invalid" << std::endl;
      return INVALID;
    }
    if (N_jj > SINGLETON) {
      std::cout << "[infile] second dimension must be singleton" << std::endl;
      return INVALID;
    }
  }
  for (auto el : instrings_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[instring] shape is invalid" << std::endl;
      return INVALID;
    }
    if (N_jj > SINGLETON) {
      std::cout << "[instring] second dimension must be singleton" << std::endl;
      return INVALID;
    }
  }
  for (auto el : randoms_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[random] shape is invalid" << std::endl;
      return INVALID;
    }
    if (N_jj > SINGLETON) {
      std::cout << "[random] second dimension must be singleton" << std::endl;
      return INVALID;
    }
  }
  for (auto el : irtarget_ms_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[irtarget_m] shape is invalid" << std::endl;
      return INVALID;
    }
    if (N_jj > SINGLETON) {
      std::cout << "[irtarget_m] second dimension must be singleton"
                << std::endl;
      return INVALID;
    }
  }
  // The remaining shapes need not be singleton
  for (auto el : accs_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[acc] shape is invalid" << std::endl;
      return INVALID;
    }
  }
  for (auto el : noises_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[noise] shape is invalid" << std::endl;
      return INVALID;
    }
  }
  for (auto el : sns_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[sn] shape is invalid" << std::endl;
      return INVALID;
    }
  }
  for (auto el : qns_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[qn] shape is invalid" << std::endl;
      return INVALID;
    }
  }
  for (auto el : noise_models_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[noise_model] shape is invalid" << std::endl;
      return INVALID;
    }
  }

  return N_jj;
}

}
