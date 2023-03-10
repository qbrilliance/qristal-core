// Copyright (c) 2022 Quantum Brilliance Pty Ltd

// STL
#include <algorithm>
#include <memory>
#include <regex>
#include <random>
#include <sstream>
#include <stdexcept>

// Boost
#include <boost/dynamic_bitset.hpp>

// QB
#include "qb/core/session.hpp"
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include "qb/core/noise_model/noise_model_factory.hpp"

// XACC
#include "CompositeInstruction.hpp"
#include "AcceleratorBuffer.hpp"
#include "xacc.hpp"

// Helper functions
namespace
{
  // Wrap raw OpenQASM string in a QB Kernel:
  // - Move qreg to a kernel argument
  // - Denote the kernel name as 'QBCIRCUIT'
  inline std::string convertRawOpenQasmToQBKernel(const std::string &in_rawQasm)
  {
    // Pattern:
    // - start with qreg
    // - follow by any number of spaces
    // - a valid variable name (first character is alpha or _, followed by alphanumeric)
    // - register size
    // - spaces or no spaces, then ';'
    const std::regex qregDeclarationRegex(R"(qreg[ \t]+([a-zA-Z_$][a-zA-Z_$0-9]*)\[\d+\]\s*;)");
    std::smatch matches;
    if (std::regex_search(in_rawQasm, matches, qregDeclarationRegex))
    {
      // Note: first match is the entire string, second match is the qreg
      // variable name grouping.
      const std::string qRegDeclararion = matches[0].str();
      const std::string qRegName = matches[1].str();
      // Erase the declaration line from the body and construct the kernel
      // signature accordingly.
      std::string q2oqm = in_rawQasm;
      q2oqm.erase(q2oqm.find(qRegDeclararion), qRegDeclararion.length());
      const std::string qbstr =
          "__qpu__ void QBCIRCUIT(qreg " + qRegName + ") {\n" + q2oqm + '}';
      return qbstr;
    }
    else
    {
      // There is no qreg declaration, e.g., an empty OpenQASM source with no
      // body. Just handle it graciously.
      const std::string qbstr =
          "__qpu__ void QBCIRCUIT(qreg q) {\n" + in_rawQasm + '}';
      return qbstr;
    }
  }

  // Trim from left (remove leading white spaces, e.g., spaces, tab, empty lines)
  inline std::string ltrim(const std::string &in_str,
                           const char *t = " \t\n\r\f\v")
  {
    auto s = in_str;
    s.erase(0, s.find_first_not_of(t));
    return s;
  }

  const double getExpectationValueZ(std::shared_ptr<xacc::AcceleratorBuffer> buffer)
  {
    double aver = 0.0;
    auto has_even_parity = [](const std::string &x) -> bool
    {
      int c = std::count(x.begin(), x.end(), '1');
      return c%2 == 0;
    };
    if (buffer->hasExtraInfoKey("ro-fixed-exp-val-z"))
    {
      aver = mpark::get<double>(buffer->getInformation("ro-fixed-exp-val-z"));
    }
    else if (buffer->hasExtraInfoKey("exp-val-z"))
    {
      aver = mpark::get<double>(buffer->getInformation("exp-val-z"));
    }
    else
    {
      if (buffer->getMeasurementCounts().empty())
      {
        xacc::error("called getExpectationValueZ() on an AcceleratorBuffer with "
                    "no measurements!");
        return 0;
      }

      for (auto &kv : buffer->getMeasurementCounts())
      {
        auto par = has_even_parity(kv.first);
        auto p = buffer->computeMeasurementProbability(kv.first);
        if (!par) p = -p;
        aver += p;
      }
    }
    return aver;
  }

} // namespace


namespace qb
{

  /// Core session methods: run/run_async executions and helpers for compilation/post-processing...
  session::circuit_input_types session::validate_infiles_instrings_randoms_irtarget_ms_nonempty(const int &ii,
                                                        const int &jj) {
    session::circuit_input_types returnval = session::circuit_input_types::INVALID;
    if (debug_) {
      std::cout
          << "[debug]:[start of validate_infiles_instrings_randoms_irtarget_ms_nonempty]:"
          << "[circuit: " << ii << ", condition: " << jj << "]: " << std::endl;
    }

    // Helper to check if a 2-D array (qb::VectorString, qb::VectorN) is empty
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
                << "]: is_irtarget_m_empty = " << is_irtarget_m_empty << std::endl;
    }

    const bool is_qoda_empty = qoda_kernels_.empty();
    if (debug_) {
      std::cout << "[debug]:"
                << "[circuit: " << ii << ", condition: " << jj
                << "]: is_qoda_empty = " << is_qoda_empty << std::endl;
    }

    if (is_infiles_empty && is_instrings_empty && is_randoms_empty &&
        is_irtarget_m_empty && is_qoda_empty) {
      throw std::invalid_argument(
          "session: at least one of these must have a "
          "value: infile | instring | random | irtarget_m | qoda ");
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

    // QODA input
    if (!is_qoda_empty) {
        if (debug_) {
          std::cout << "[debug]:"
                    << "[circuit: " << ii << ", condition: " << jj
                    << "]: " << std::endl
                    << " has a QODA target" << std::endl;
        }
        return session::circuit_input_types::VALID_QODA;
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
      throw std::invalid_argument("session: number of shots [sn] must have a value");
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

  int session::validate_rns_nonempty() {
    const int VALID_ALL = 0;
    // const int VALID_PARTIAL = 1;
    const int INVALID = -1;

    bool is_rns_empty = true;

    int returnval = VALID_ALL;
    if (debug_) {
      std::cout << "[debug]:"
                << "Checking for valid settings for the number of repetitions..."
                << std::endl;
    }

    if (!rns_.empty()) {
      is_rns_empty =
          std::all_of(rns_.cbegin(), rns_.cend(),
                      [](std::vector<size_t> rn) { return rn.empty(); });
      if (debug_ && is_rns_empty) {
        std::cout << "[debug]:"
                  << "rns_ is empty" << std::endl;
      }
    }
    if (is_rns_empty) {
      throw std::invalid_argument(
          "session: number of repetitions [rn] must have a value");
      returnval = INVALID;
    }
    return returnval;
  }

  int session::validate_thetas_option() {
    const int VALID_ALL = 0;
    const int NOT_SET = -1;
    // const int INVALID = -2;

    bool is_thetas_empty = true;

    int returnval = VALID_ALL;
    if (debug_) {
      std::cout << "[debug]:"
                << "Checking if the optional thetas_ is set..." << std::endl;
    }

    if (!thetas_.empty()) {
      is_thetas_empty = std::all_of(thetas_.cbegin(), thetas_.cend(),
                                    [](MapND theta) { return theta.empty(); });
      if (debug_ && is_thetas_empty) {
        std::cout << "[debug]:"
                  << "thetas_ is empty" << std::endl;
      }
    }
    if (is_thetas_empty) {
      returnval = NOT_SET;
    }
    return returnval;
  }

  template <class TT>
  int session::singleton_or_eqlength(const TT &in_d, const int N_ii) {
    const int INVALID = -1;
    const int SINGLETON = 1;
    if (in_d.size() > 0) {
      if (N_ii == SINGLETON) {
        return in_d.size();
      } else {
        if ((in_d.size() == N_ii) || (in_d.size() == SINGLETON)) {
          return N_ii;
        } else {
          return INVALID;
        }
      }
    } else {
      return N_ii;
    }
  }

  template <class TT> int session::eqlength(const TT &in_d, const int N_ii) {
    const int INVALID = -1;
    if (in_d.size() == N_ii) {
      return N_ii;
    } else {
      return INVALID;
    }
  }

  int session::is_ii_consistent() {
    const int INVALID = -1;
    // const int SINGLETON = 1;
    int N_ii = 0;

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
        N_ii =irtarget_ms_.size();
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
    if ((N_ii = singleton_or_eqlength(thetas_, N_ii)) == INVALID) {
      std::cout << "[theta] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(betas_, N_ii)) == INVALID) {
      std::cout << "[beta] shape is invalid" << std::endl;
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
        std::cout << "[irtarget_m] second dimension must be singleton" << std::endl;
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
    for (auto el : thetas_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[theta] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : betas_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[beta] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : aws_device_names_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[aws_device_name] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : aws_s3s_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[aws_s3] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : aws_s3_paths_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[aws_s3_path] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : noise_models_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[noise_model] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    // add more shape checks here

    return N_jj;
  }

  /**
    Desc:
    Generates random circuits of arbitrary size and form.

    Notes:
    - The user needs to study and eventually insert large enough circuit depth for
  proper random behaviour. This would allow for the appearance of all basic gates
  and reaching Porter-Thomas distribution
      ([Boixo2018] argues sub-linear scaling is enough to achieve this). Note some
  basic gates are not available through XACC framework but all can be implemented
  in terms of others gates we list here. In addition if the optimisations and
  placement features can make the actual depth larger than this fixed amount,
  however, it always atays as O(n_q).
    - We include only maximally two-operand gates.
    - Currently does not include any middle measurements and conditional
  operations.
    - Currently does not include any middle resets.
    - Inputting a fixed random seed is not yet implemented.
    - Currently we are not (weakly) conditioning any of the quantum wires to the
  classical bit values.

    Args:
    [n_q (int)] number of quantum registers -- must be equal or greater than 3
    [seed (int)] sets the random seed

    Returns:
    [RandomQcircuit (string)] generated random quantum circuit in OpenCasm format

    Raises:
    NONE
  **/
  std::string session::random_circuit(const int &n_q, const int &depth) {
    std::vector<std::string> one_q_ops = {"id", "u1",  "u2", "u3", "x",
                                          "y",  "z",   "h",  "s",  "sdg",
                                          "t",  "tdg", "rx", "ry", "rz"};
    std::vector<std::string> two_q_ops = {"cx",  "cy",  "cz",  "swap", "rzz",
                                          "cu1", "crz", "cu3", "ch"};
    // Note that three-operand gates are not available in XACC accelerators yet,
    // but can be considered as sequentially implemented in terms of others here.

    std::vector<std::string> one_params = {"u1",  "rx",  "ry", "rz",
                                           "rzz", "crz", "cu1"};

    std::vector<std::string> two_params = {"u2"};
    std::vector<std::string> three_params = {"u3", "cu3"};

    std::default_random_engine re{static_cast<long unsigned int>(time(0))};

    // initialising the quantum circuit
    std::stringstream RandomQcircuit;
    RandomQcircuit << "__qpu__ void QBCIRCUIT(qreg q) {" << std::endl
                   << "  OPENQASM 2.0;" << std::endl
                   << "  include \"qelib1.inc\";" << std::endl;

    RandomQcircuit << "  creg c[" << n_q << "];" << std::endl;

    for (int level = 1; level <= depth; level++) {

      std::vector<int> remaining_qubits;
      for (int i = 0; i < n_q; i++) {
        remaining_qubits.push_back(i);
      }

      while (!remaining_qubits.empty()) {

        int remaining_size = remaining_qubits.size();
        int max_possible_operands = std::min(remaining_size, 2);
        int num_operands = std::rand() % max_possible_operands + 1;

        // shuffle remaining_qubits if needed ...
        std::vector<int> operands;
        for (int j = 1; j <= num_operands; j++) {
          int this_int = choose_random(remaining_qubits);
          operands.push_back(this_int);
          remaining_qubits.erase(std::remove(remaining_qubits.begin(),
                                             remaining_qubits.end(), this_int),
                                 remaining_qubits.end());
        }

        // create operands_str ...
        std::stringstream operands_stream;
        operands_stream << " ";
        for (int i : operands) {
          operands_stream << "q[" << i << "],";
        }
        std::string operands_str = operands_stream.str();
        operands_str.pop_back();

        // update remaining_qubits with its last remaining members if needed ...
        std::string operation;
        if (num_operands == 1)
          operation = choose_random(one_q_ops);
        else if (num_operands == 2)
          operation = choose_random(two_q_ops);

        int num_angles = 0;
        if (std::find(one_params.begin(), one_params.end(), operation) !=
            one_params.end())
          num_angles = 1;
        else if (std::find(two_params.begin(), two_params.end(), operation) !=
                 two_params.end())
          num_angles = 2;
        else if (std::find(three_params.begin(), three_params.end(), operation) !=
                 three_params.end())
          num_angles = 3;

        // setting the angles randomly ...
        // we can create angles_str at the same time ...
        std::stringstream angles_stream;
        std::string angles_str;
        if (num_angles > 0) {
          angles_stream << "(";
          for (int j = 1; j <= num_angles; j++) {
            std::uniform_real_distribution<double> unif(-((j / 2) % 2) * M_PI,
                                                        M_PI);
            double angle = unif(re);
            angles_stream << angle << ",";
          }
          angles_str = angles_stream.str();
          angles_str.pop_back();
          angles_str += ")";
        }

        RandomQcircuit << "  " << operation << angles_str << operands_str << ";"
                       << std::endl;
      }
    }

    // adding the measurement part to the circuit ...
    for (int i = 0; i < n_q; i++) {
      RandomQcircuit << "  measure q[" << i << "] -> c[" << i << "];"
                     << std::endl;
    }

    // finalising RandomQcircuit string
    RandomQcircuit << "}" << std::endl;
    return RandomQcircuit.str();
  }

  /**
   * get_jensen_shannon - calculate the divergence between two discrete
   *probability distributions supported in the same space
   *
   * Input:
   *
   *     [std::map<std::string, int>]        in_q : the counts from a quantum
   *simulation.  The string key is assumed to be a BCD index for in_p
   *
   *     [std::vector<std::complex<double>>] in_p : the amplitudes for the
   *theoretical distribution of states from which in_q has been sampled.  begin()
   *corresponds to |00...0>. end() corresponds to |111...1>.  Increments in the
   *state label correspond to increments in the iterator.
   *
   * Output:
   *
   *     [double]  divergence : 0.5*(D_KL(in_p||m) + D_KL(in_q||m))
   *
   * where:
   *
   *   m = 0.5*(in_p + in_q)
   *
   *   D_KL(P||Q) = P' * (log(P)-log(Q))  : P,Q are column vectors.  Exclude all
   *elements of Q that are zero
   **/
  template <typename TP> double session::get_probability(const TP &in_elem) {
    return in_elem;
  }

  // Specialise for complex double
  template <> double session::get_probability(const std::complex<double> &in_elem) {
    return std::norm(in_elem);
  }

  template <>
  double session::get_probability(
      const std::pair<const int, std::complex<double>> &in_pair) {
    return std::norm(in_pair.second);
  }

  template <typename TQ, class TV>
  TQ session::get_key(const std::map<TQ, TV> &in_q, const int &key_in_q) {
    return (TQ)key_in_q;
  }

  template <class TT, class TV>
  int session::get_key(const std::map<int, TT> &in_q,
                    const std::pair<const int, TV> &el, const int &key_in_q) {
    return el.first;
  }

  // Specialise for std::string keys
  template <>
  std::string session::get_key(const std::map<std::string, int> &in_q,
                            const int &key_in_q) {
    std::string n_str = in_q.begin()->first;
    size_t sz_label = n_str.size();
    boost::dynamic_bitset<> statelabel_v(sz_label, key_in_q);
    std::string tkey;
    to_string(statelabel_v, tkey);
    return tkey;
  }

  template <class TQ, typename TP>
  double session::get_jensen_shannon_divergence(const std::map<TQ, int> &in_q,
                                             const TP &in_p,
                                             const bool &in_use_lsb) {
    double divergence = 0.0;
    int sum_in_q = 0;
    for (auto in_q_elem = in_q.begin(); in_q_elem != in_q.end(); in_q_elem++) {
      sum_in_q += in_q_elem->second;
    }

    typename std::map<TQ, int>::const_iterator in_q_elem;
    int i_iter = 0;
    for (auto in_p_elem = in_p.begin(); in_p_elem != in_p.end(); in_p_elem++) {
      auto statelabel = get_key(in_q, *in_p_elem, i_iter);
      double nipe = get_probability(in_p_elem->second);
      if (debug_) {
        std::cout << "[debug]: i_iter: " << i_iter
                  << " , statelabel: " << statelabel << " , nipe: " << nipe
                  << std::endl;
      }
      in_q_elem = in_q.find(statelabel);
      if (in_q_elem != in_q.end()) {
        double rfq = (1.0 / sum_in_q) * (in_q_elem->second);
        if (debug_) {
          std::cout << "[debug]: rfq: " << rfq << std::endl;
        }
        double m = 0.5 * (rfq + nipe);
        if ((in_q_elem->second > 0) && (nipe > 0)) {
          divergence += 0.5 * (nipe * (std::log(nipe) - std::log(m)) +
                               rfq * (std::log(rfq) - std::log(m)));
        } else if ((in_q_elem->second > 0) && (nipe == 0)) {
          divergence += 0.5 * rfq * (std::log(rfq) - std::log(m));
        } else if ((in_q_elem->second == 0) && (nipe > 0)) {
          divergence += 0.5 * nipe * (std::log(nipe) - std::log(m));
        } else {
          if (debug_) {
            std::cout
                << "Ignoring a state that has zero probability in both P and Q"
                << std::endl;
          }
        }
      } else {
        divergence +=
            0.5 * nipe * std::log(2); // divergence += 0.5*nipe*(std::log(nipe) -
                                      // std::log(m)); m = 0.5*nipe
      }
      i_iter++;
    }
    // Check for entries of in_q that are sparse (zero) for in_p
    i_iter = 0;
    for (auto in_q_elem = in_q.begin(); in_q_elem != in_q.end(); in_q_elem++) {
      auto statelabel = get_key(in_p, *in_q_elem, i_iter);
      double rfq = (1.0 / sum_in_q) * (in_q_elem->second);
      auto in_p_el = in_p.find(statelabel);
      if (in_p_el == in_p.end()) {
        divergence +=
            0.5 * rfq * std::log(2); // divergence += 0.5*rfq*(std::log(rfq) -
                                     // std::log(m)); m = 0.5*rfq
      }
      i_iter++;
    }

    return divergence;
  }

  void session::get_jensen_shannon(const size_t &ii, const size_t &jj) {
    // Validations
    if (debug_) {
      std::cout << "Invoked get_jensen_shannon on circuit: " << ii
                << ", setting:" << jj << std::endl;
    }
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_ii = SINGLETON;
    int N_jj = SINGLETON;

    if ((N_ii = singleton_or_eqlength(out_counts_, N_ii)) == INVALID) {
      throw std::range_error("[out_counts] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(output_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[output_amplitudes] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(acc_uses_lsbs_, N_ii)) == INVALID) {
      throw std::range_error("[acc_uses_lsbs] shape is invalid");
    }
    for (auto el : out_counts_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_counts] shape is invalid");
      }
    }
    for (auto el : output_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[output_amplitudes] shape is invalid");
      }
    }
    for (auto el : acc_uses_lsbs_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[acc_uses_lsbs] shape is invalid");
      }
    }

    if (out_divergences_.size() < (ii + 1)) {
      if (debug_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_divergences_.resize(ii + 1);
    }
    if (out_divergences_.at(ii).size() < (jj + 1)) {
      if (debug_) {
        std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
      }
      out_divergences_.at(ii).resize(jj + 1);
    }

    double jsdivergence = get_jensen_shannon_divergence(
        out_counts_.at(ii).at(jj), output_amplitudes_.at(ii).at(jj),
        acc_uses_lsbs_.at(ii).at(jj) );
    ND jsdivergence_nd;
    jsdivergence_nd.insert(std::make_pair(0, jsdivergence));
    out_divergences_.at(ii).at(jj) = jsdivergence_nd;
  }

  void session::get_jensen_shannon() {
    if (debug_) std::cout << "Invoked get_jensen_shannon()" << std::endl;

    // Shape consistency
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_ii = SINGLETON;
    int N_jj = SINGLETON;

    if ((N_ii = singleton_or_eqlength(out_counts_, N_ii)) == INVALID) {
      throw std::range_error("[out_counts] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(output_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[output_amplitudes] shape is invalid");
    }
    for (auto el : out_counts_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_counts] shape is invalid");
      }
    }
    for (auto el : output_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[output_amplitudes] shape is invalid");
      }
    }

    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    if (debug_) {
      std::cout << "N_ii: " << N_ii << std::endl;
    }
    if (debug_) {
      std::cout << "N_jj: " << N_jj << std::endl;
    }

    for (int ii = 0; ii < N_ii; ii++) {
      for (int jj = 0; jj < N_jj; jj++) {
        get_jensen_shannon(ii, jj);
      }
    }
  }

  std::string session::aer_circuit_transpiler(std::string &circuit) {

    std::string NEWcircuit = circuit;
    std::istringstream stream{circuit};
    std::string current_line;

    // Repeat till end is reached
    while (std::getline(stream, current_line)) {
      size_t pos = NEWcircuit.find(current_line);

      size_t pos_cy = current_line.find("cy q[");
      size_t pos_rzz = current_line.find("rzz(");

      // size_t pos_q = current_line.find("q");
      size_t open1 = current_line.find("[");
      size_t open2 = current_line.find("[", open1 + 1);
      size_t close1 = current_line.find("]");
      size_t close2 = current_line.find("]", close1 + 1);
      size_t par1 = current_line.find("(");
      size_t par2 = current_line.find(")");
      // size_t com1 = current_line.find(",");
      // size_t com2 = current_line.find(",", com1 + 1);
      // size_t com3 = current_line.find(",", com2 + 1);

      if (pos_cy != std::string::npos) {
        // find the qubit numbering
        std::string q_i = current_line.substr(open1 + 1, close1 - open1 - 1);
        std::string q_f = current_line.substr(open2 + 1, close2 - open2 - 1);
        // building up the replacement circuit
        std::stringstream repl_circuit;
        repl_circuit << "  sdg q[" << q_f << "];" << std::endl;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
        repl_circuit << "  s q[" << q_f << "];";
        // Replace this occurrence
        NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
      } else if (pos_rzz != std::string::npos) {
        // std::string angle = current_line.substr(pos_rzz+4,8);
        std::string angle = current_line.substr(par1 + 1, par2 - par1 - 1);
        // std::string q_i = current_line.substr(pos_q+2,1);
        std::string q_i = current_line.substr(open1 + 1, close1 - open1 - 1);
        std::string q_f = current_line.substr(open2 + 1, close2 - open2 - 1);
        // building up the replacement circuit
        std::stringstream repl_circuit;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
        repl_circuit << "  rz(" << angle << ") q[" << q_f << "];" << std::endl;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];";
        // Replace this occurrence
        NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
      }
    }

    return NEWcircuit;
  }

  void session::profile(const size_t &ii, const size_t &jj) {}

  /// Retrieve and validate run configurations for index pair (ii, jj) using the table index convention.
  session::run_i_j_config session::get_run_config(size_t ii, size_t jj)
  {
    session::run_i_j_config config;
    /// Number of shots
    {
      ValidatorTwoDim<VectorN, size_t> sns_valid(sns_, session::SNS_LOWERBOUND, session::SNS_UPPERBOUND,
                                                 " number of shots [sn] ");
      if (sns_valid.is_data_empty())
      {
        throw std::range_error("Number of shots [sn] cannot be empty");
      }

      config.num_shots = sns_valid.get(ii, jj);
    }

    /// Number of qubits
    {
      ValidatorTwoDim<VectorN, size_t> qns_valid(qns_, session::QNS_LOWERBOUND, session::QNS_UPPERBOUND,
                                                 " number of qubits [qn] ");
      if (qns_valid.is_data_empty())
      {
        throw std::range_error("Number of qubits [qn] cannot be empty");
      }
      config.num_qubits = qns_valid.get(ii, jj);
    }

    /// Number of repetitions
    {
      ValidatorTwoDim<VectorN, size_t> rns_valid(rns_, session::RNS_LOWERBOUND, session::RNS_UPPERBOUND,
                                                 " number of repetitions [rn] ");
      if (rns_valid.is_data_empty())
      {
        throw std::range_error("Number of repetitions [rn] cannot be empty");
      }
      config.num_repetitions = rns_valid.get(ii, jj);
    }

    /// Enable output transpilation and resource estimation
    {
      ValidatorTwoDim<VectorBool, bool> output_oqm_enableds_valid(
          output_oqm_enableds_, false, true, " enable output of transpiled circuit [output_oqm_enabled] ");
      if (output_oqm_enableds_valid.is_data_empty())
      {
        throw std::range_error("Enable output of transpiled circuit "
                               "[output_oqm_enabled] cannot be empty");
      }
      config.oqm_enabled = output_oqm_enableds_valid.get(ii, jj);
    }

    /// Name of the accelerator
    {
      ValidatorTwoDim<VectorString, std::string> accs_valid(accs_, VALID_ACCS, " name of back-end simulator [acc] ");
      if (accs_valid.is_data_empty())
      {
        throw std::range_error("A back-end simulator [acc] must be specified");
      }
      config.acc_name = accs_valid.get(ii, jj);
    }

    /// AWS device name
    {
      ValidatorTwoDim<VectorString, std::string> aws_device_names_valid(
          aws_device_names_, VALID_AWS_DEVICES, " name of AWS back-end simulator/QPU [aws_backend] ");
      if (aws_device_names_valid.is_data_empty())
      {
        throw std::range_error("An AWS back-end simulator/QPU [aws_backend] must be specified");
      }
      config.aws_device_name = aws_device_names_valid.get(ii, jj);
    }

    /// AWS S3 bucket
    {
      ValidatorTwoDim<VectorString, std::string> aws_s3_valid(aws_s3s_);
      if (aws_s3_valid.is_data_empty())
      {
        throw std::range_error("An AWS bucket [aws_s3] must be specified");
      }
      config.aws_s3 = aws_s3_valid.get(ii, jj);
    }

    /// AWS S3 path
    {
      ValidatorTwoDim<VectorString, std::string> aws_s3_path_valid(aws_s3_paths_);
      if (aws_s3_path_valid.is_data_empty())
      {
        throw std::range_error("An AWS path inside aws_s3 [aws_s3_path] must be specified");
      }
      config.aws_s3_path = aws_s3_path_valid.get(ii, jj);
    }

    /// AWS format for submission
    {
      ValidatorTwoDim<VectorString, std::string> aws_format_valid(aws_formats_, VALID_AWS_FORMATS,
                                                                  " AWS language format [aws_format] ");
      if (aws_format_valid.is_data_empty())
      {
        throw std::range_error(" AWS language format [aws_format] must be specified");
      }
      config.aws_format = aws_format_valid.get(ii, jj);
    }

    /// QB custom OpenQASM include file
    {
      ValidatorTwoDim<VectorString, std::string> include_qbs_valid(include_qbs_);
      if (include_qbs_valid.is_data_empty())
      {
        throw std::range_error("A file for custom OpenQASM gates [include_qb] must be specified");
      }
      config.openqasm_qb_include_filepath = include_qbs_valid.get(ii, jj);
    }

    /// QPU configuration JSON file
    {
      ValidatorTwoDim<VectorString, std::string> qpu_configs_valid(qpu_configs_);
      if (qpu_configs_valid.is_data_empty())
      {
        throw std::range_error(
            "A JSON configuration file for Quantum Brilliance hardware [qpu_config] must be specified");
      }
      config.qpu_config_json_filepath = qpu_configs_valid.get(ii, jj);
    }

    /// Input source string type (OpenQASM/XASM/Quil)
    {
      // Default is OpenQASM
      config.source_type = source_string_type::OpenQASM;
      ValidatorTwoDim<VectorBool, bool> xasms_valid(xasms_, false, true, " interpret circuit in XASM format [xasm] ");
      if (xasms_valid.is_data_empty())
      {
        throw std::range_error("Flag for XASM [xasm] cannot be empty");
      }

      ValidatorTwoDim<VectorBool, bool> quil1s_valid(quil1s_, false, true,
                                                     " interpret circuit in Quil v1 format [quil1] ");
      if (quil1s_valid.is_data_empty())
      {
        throw std::range_error("Flag for Quil v1 [quil1] cannot be empty");
      }

      const bool is_xasm = xasms_valid.get(ii, jj);
      const bool is_quil = quil1s_valid.get(ii, jj);
      // Users shouldn't set both (invalid)
      if (is_xasm && is_quil)
      {
        throw std::logic_error("XASM and Quil cannot be enabled at the same time.");
      }
      if (is_xasm)
      {
        config.source_type = source_string_type::XASM;
      }
      if (is_quil)
      {
        config.source_type = source_string_type::Quil;
      }
    }

    /// Disable circuit placement flag
    {
      ValidatorTwoDim<VectorBool, bool> noplacements_valid(noplacements_, false, true,
                                                           " disable placement mapping [noplacement] ");
      if (noplacements_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling placement mapping [noplacement] cannot be empty");
      }
      config.no_placement = noplacements_valid.get(ii, jj);
    }

    /// Disable circuit optimisation flag
    {
      ValidatorTwoDim<VectorBool, bool> nooptimises_valid(nooptimises_, false, true,
                                                          " disable circuit optimiser [nooptimise] ");
      if (nooptimises_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit optimiser [nooptimise] cannot be empty");
      }
      config.no_optimise = nooptimises_valid.get(ii, jj);
    }

    /// Skip simulation flag
    {
      ValidatorTwoDim<VectorBool, bool> nosims_valid(nosims_, false, true, " disable circuit simulator [nosim] ");
      if (nosims_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit simulator [nosim] cannot be empty");
      }
      config.no_sim = nosims_valid.get(ii, jj);
    }

    /// Enable/disable noise flag
    {
      ValidatorTwoDim<VectorBool, bool> noises_valid(noises_, false, true, " enable the QB noise model [noise] ");
      if (noises_valid.is_data_empty())
      {
        throw std::range_error("Enable the QB noise model [noise] cannot be empty");
      }
      config.noise = noises_valid.get(ii, jj);
    }

    /// AWS verbatim mode flag
    {
      ValidatorTwoDim<VectorBool, bool> verbatim_valid(aws_verbatims_, false, true, " enable the verbatim [noise] ");
      if (verbatim_valid.is_data_empty())
      {
        throw std::range_error("Enable the verbatim mode [verbatim] cannot be empty");
      }
      config.aws_verbatim = verbatim_valid.get(ii, jj);
    }

    /// TNQVM-MPS simulator max bond dimension
    {
      ValidatorTwoDim<VectorN, size_t> max_bond_dimensions_valid(
          max_bond_dimensions_, session::MAX_BOND_DIMENSION_LOWERBOUND, session::MAX_BOND_DIMENSION_UPPERBOUND,
          " MPS maximum bond dimension [max-bond-dimension] ");
      if (max_bond_dimensions_valid.is_data_empty())
      {
        throw std::range_error("MPS maximum bond dimension [max-bond-dimension] cannot be empty");
      }
      config.max_bond_tnqvm = max_bond_dimensions_valid.get(ii, jj);
    }

    /// TNQVM-MPS SVD cutoff limit
    {
      ND svd_lowerbound{{0, -1.0e9}}; // This limit is currently ignored
      ND svd_upperbound{{0, 1.0e9}};  // This limit is currently ignored
      ValidatorTwoDim<VectorMapND, ND> svd_cutoffs_valid(svd_cutoffs_, svd_lowerbound, svd_upperbound,
                                                         " MPS SVD cutoff [svd-cutoff] ");

      if (svd_cutoffs_valid.is_data_empty())
      {
        throw std::range_error("MPS SVD cutoff [svd-cutoff] cannot be empty");
      }
      config.svd_cutoff_tnqvm = svd_cutoffs_valid.get(ii, jj).at(0);
    }

    /// Name of the noise model
    {
      ValidatorTwoDim<VectorString, std::string> noise_models_valid(noise_models_);
      if (noise_models_valid.is_data_empty())
      {
        throw std::range_error("A valid noise model [noise_model] must be specified");
      }
      config.noise_model = noise_models_valid.get(ii, jj);
    }

    /// QB hardware: prevent contrast thresholds from being sent by Qristal
    {
      ValidatorTwoDim<VectorBool, bool> use_default_contrast_settings_valid(use_default_contrast_settings_, false, true, " prevent contrast thresholds from being sent [use_default_contrast_setting] ");
      if (use_default_contrast_settings_valid.is_data_empty())
      {
        throw std::range_error("Indicator to prevent sending contrast thresholds [use_default_contrast_setting] cannot be empty");
      }
      config.use_default_contrast_setting = use_default_contrast_settings_valid.get(ii, jj);
    }

    /// QB hardware: contrast threshold during init
    {
      ND init_contrast_threshold_lowerbound{{0, CONTRAST_LOWERBOUND}};
      ND init_contrast_threshold_upperbound{{0, CONTRAST_UPPERBOUND}}; 
      ValidatorTwoDim<VectorMapND, ND> init_contrast_thresholds_valid(init_contrast_thresholds_,
                                                                      init_contrast_threshold_lowerbound,
                                                                      init_contrast_threshold_upperbound,
                                                                      " init contrast threshold [init_contrast_threshold] ");

      if (init_contrast_thresholds_valid.is_data_empty())
      {
        throw std::range_error("Contrast threshold for init [init_contrast_threshold] cannot be empty");
      }
      config.init_contrast_thresholds = init_contrast_thresholds_valid.get(ii, jj);
    }

    /// QB hardware: contrast threshold for each qubit during final readout 
    {
      ND qubit_contrast_threshold_lowerbound{{0, CONTRAST_LOWERBOUND}};
      ND qubit_contrast_threshold_upperbound{{0, CONTRAST_UPPERBOUND}}; 
      ValidatorTwoDim<VectorMapND, ND> qubit_contrast_thresholds_valid(qubit_contrast_thresholds_,
                                                                      qubit_contrast_threshold_lowerbound,
                                                                      qubit_contrast_threshold_upperbound,
                                                                      " qubit final readout contrast threshold [qubit_contrast_threshold] ");

      if (qubit_contrast_thresholds_valid.is_data_empty())
      {
        throw std::range_error("Contrast threshold for qubit final readout [qubit_contrast_threshold] cannot be empty");
      }
      config.qubit_contrast_thresholds = qubit_contrast_thresholds_valid.get(ii, jj);
    }

    return config;
  }

  /// Ensure that all result tables are resized/expanded to accommodate (ii, jj) experiment index.
  void session::ensure_results_table_size(size_t ii, size_t jj)
  {
    // Helper to resize the rows and columns (as nested vectors) to make sure it can accommodate (i, j) index
    const auto resize_for_i_j = [=](auto& table_to_resize, const std::string& table_name)
    {
      if (table_to_resize.size() < (ii + 1))
      {
        if (debug_)
        {
          std::cout << "# " << table_name << " - Resizing ii: " << (ii + 1) << "\n";
        }
        table_to_resize.resize(ii + 1);
      }
      if (table_to_resize.at(ii).size() < (jj + 1))
      {
        if (debug_)
        {
          std::cout << "# " << table_name << " - ii: " << ii << ", resizing jj: " << (jj + 1) << "\n";
        }
        table_to_resize.at(ii).resize(jj + 1);
      }
    };

    // Resize all tables:
    resize_for_i_j(acc_uses_lsbs_, "acc_uses_lsbs_");
    resize_for_i_j(acc_uses_n_bits_, "acc_uses_n_bits_");
    resize_for_i_j(output_amplitudes_, "output_amplitudes_");
    resize_for_i_j(out_raws_, "out_raws_");
    resize_for_i_j(out_counts_, "out_counts_");
    resize_for_i_j(out_divergences_, "out_divergences_");
    resize_for_i_j(out_transpiled_circuits_, "out_transpiled_circuits_");
    resize_for_i_j(out_qobjs_, "out_qobjs_");
    resize_for_i_j(out_qbjsons_, "out_qbjsons_");
    resize_for_i_j(out_single_qubit_gate_qtys_, "out_single_qubit_gate_qtys_");
    resize_for_i_j(out_double_qubit_gate_qtys_, "out_double_qubit_gate_qtys_");
    resize_for_i_j(out_total_init_maxgate_readout_times_, "out_total_init_maxgate_readout_times_");
    resize_for_i_j(out_z_op_expects_, "out_z_op_expects_");
  }

  /// Retrieve the target circuit string for (i, j) task
  std::string session::get_target_circuit_qasm_string(size_t ii, size_t jj, const run_i_j_config& run_config) {
   std::string target_circuit;
   const auto file_or_string_or_random_or_ir =
        validate_infiles_instrings_randoms_irtarget_ms_nonempty(ii, jj);
    // Invalid input
    if (file_or_string_or_random_or_ir == session::circuit_input_types::INVALID) {
      throw std::invalid_argument("Please check your settings again.");
    }

    if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_INFILE) {
        // File input: load from file
        std::ifstream tifs((infiles_[ii])[0]);
        if (tifs.is_open()) {
          std::string outbuf(std::istreambuf_iterator<char>(tifs), {});
          if (ltrim(outbuf).rfind("OPENQASM", 0) == 0) {
            // Check for raw OpenQASM string.
            // Source string starts with "OPENQASM" (not already being wrapped in
            // __qpu__)
            target_circuit = convertRawOpenQasmToQBKernel(outbuf);
          } else {
            target_circuit = outbuf;
          }
        }
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_INSTRING_QPU) {
      // String input
      target_circuit = (instrings_[ii])[0];
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_RANDOM) {
      // Random input: generate random circuit
      target_circuit = random_circuit(run_config.num_qubits, (randoms_[ii])[0]);
      {
        std::scoped_lock lock(m_);
        // Thread-safe increase the size of instrings_ to cache the random circuit
        if (instrings_.size() < (ii + 1))
        {
          if (debug_)
          {
            std::cout << "Resizing ii: " << (ii + 1) << std::endl;
          }
          instrings_.resize(ii + 1);
        }
        if (debug_)
        {
          std::cout << "Recording the random circuit to instring" << std::endl;
        }
        if (instrings_.at(ii).size() == 0)
        {
          instrings_.at(ii).resize(1);
        }

        instrings_.at(ii).at(0) += "\n# Random circuit created:\n\n";
        instrings_.at(ii).at(0) += target_circuit;
      }
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_IR) {
      if (debug_) {
        std::cout << "Using a directly created XACC IR" << std::endl;
      }
    } else {
      throw std::invalid_argument("This circuit type is not implemented");
    }

    if (file_or_string_or_random_or_ir != session::circuit_input_types::VALID_IR) {
      // Pre-transpile
      // Note: does not apply to XASM and Quil1 input formats.
      if (debug_) {
        std::cout << "[debug]: Circuit before pre-transpile:" << std::endl
                  << target_circuit << std::endl;
      }

      const std::string acc_pre = run_config.acc_name;
      const bool xasm     = (run_config.source_type == source_string_type::XASM);
      const bool quil1    = (run_config.source_type == source_string_type::Quil);
      if (!xasm && !quil1 && (acc_pre.compare("tnqvm") == 0)) {
        // target_circuit = exatn_circuit_transpiler(target_circuit);
        if (debug_) {
          std::cout << "[debug]: No pre-transpiling will be performed:"
                    << std::endl
                    << target_circuit << std::endl;
        }
      } else if (!xasm && !quil1 && (acc_pre.compare("aer") == 0)) {
        target_circuit = aer_circuit_transpiler(target_circuit);
        if (debug_) {
          std::cout << "[debug]: Circuit after aer pre-transpile:" << std::endl
                    << target_circuit << std::endl;
        }
      } else {
        if (debug_) {
          std::cout << "[debug]: No pre-transpiling will be performed"
                    << std::endl;
        }
      }

      // QB multi-control gates
      // Note: Does not apply to XASM and Quil1 formats
      if (!xasm && !quil1) {
        if (debug_) {
          std::cout << "[debug]: Start to insert QB specific gates"
                    << std::endl;
        }
        Pretranspile qbgpre = Pretranspile("QB specific gates inserted");
        qbgpre.add_n_control_gates(target_circuit);
        if (debug_) {
          std::cout << "[debug]: Circuit after inserting QB specific gates:"
                    << std::endl;
          std::cout << target_circuit << std::endl;
        }
      }

      // Replace QBTHETA_n with theta[n]
      // Note: Does not apply to XASM and Quil1 formats
      if (!xasm && !quil1 && (validate_thetas_option() >= 0)) {
        ND lowerbound{{0, -1.0e9}}; // Currently this limit is ignored
        ND upperbound{{0, 1.0e9}};  // Currently this limit is ignored

        ValidatorTwoDim<VectorMapND, ND> thetas_valid(
            thetas_, lowerbound, upperbound,
            " values for circuit parameters QBTHETA_n ");
        Pretranspile tpre =
            Pretranspile("Substituting QBTHETA_n with theta[n]");
        ND theta = thetas_valid.get(ii, jj);
        std::vector<double> qbtheta;
        if (!theta.empty()) {
          map_to_vec<ND, std::vector<double>>(theta, qbtheta);
        }
        for (int j = 0; j < qbtheta.size(); j++) {
          std::stringstream starget;
          std::stringstream sval;

          starget << "QBTHETA_" << j;
          sval << qbtheta.at(j);
          if (debug_) {
            std::cout << starget.str() << " = " << sval.str() << std::endl;
          }
          tpre.set_parameter(starget.str(), sval.str());
        }
        tpre.run(target_circuit);
      }

      // Insert include file for QB: include_qb
      if (!xasm && !quil1) {
        std::string incqb = run_config.openqasm_qb_include_filepath;
        if (debug_)
          std::cout << "[debug]: Include file for QB: " << incqb << std::endl;
        std::ifstream tifs(incqb);
        if (tifs.is_open()) {
          std::string target_includeqb(std::istreambuf_iterator<char>(tifs),
                                        {});
          std::stringstream anchor_second;
          anchor_second << "include \"qelib1.inc\";" << std::endl
                        << target_includeqb;
          target_circuit = std::regex_replace(
              target_circuit, std::regex("include \"qelib1.inc\";"),
              anchor_second.str());
          if (debug_)
            std::cout << "[debug]: Circuit after custom include file for QB:"
                      << std::endl
                      << target_circuit << std::endl;
        } else {
          if (debug_)
            std::cout
                << "[debug]: Could not find the QB custom include file named: "
                << incqb << std::endl;
        }
      }
    }
    return target_circuit;
  }

  /// Run (execute) task specified by the (ii, jj) index pair
  void session::run(const size_t &ii, const size_t &jj) {
    // Construct validations
    if (debug_) {
      std::cout << "Invoked run on circuit: " << ii << ", setting:" << jj
                << std::endl;
    }

    // Retrieve and validate run configuration for this (ii, jj) task
    auto run_config = get_run_config(ii, jj);

    // Check shape consistency
    int N_ii = is_ii_consistent();
    int N_jj = is_jj_consistent();
    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    // Resize result tables if necessary.
    ensure_results_table_size(ii, jj);

    const auto file_or_string_or_random_or_ir =
        validate_infiles_instrings_randoms_irtarget_ms_nonempty(ii, jj);
    if (file_or_string_or_random_or_ir == session::circuit_input_types::INVALID) {
      throw std::invalid_argument("Please check your settings again.");
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_QODA) {
#ifdef WITH_QODA
      // Execute QODA kernel input
      run_qoda(ii, jj, run_config);
#else
      throw std::runtime_error(
          "QODA is not supported. Please build qb::core with QODA.");
#endif
    }
    else {
      // xacc::setIsPyApi();
      // xacc::set_verbose(true);
      xacc::HeterogeneousMap mqbacc;
      bool noplacement      = run_config.no_placement;
      bool nooptimise       = run_config.no_optimise;
      bool nosim            = run_config.no_sim;
      bool xasm             = (run_config.source_type == source_string_type::XASM);
      bool quil1            = (run_config.source_type == source_string_type::Quil);
      if (debug_)
      {
        switch (run_config.source_type)
        {
        case source_string_type::XASM:
          std::cout << "# XASM compiler: xasm" << std::endl;
          break;
        case source_string_type::Quil:
          std::cout << "# Quil v1 compiler: quil" << std::endl;
          break;
        case source_string_type::OpenQASM:
          std::cout << "# OpenQASM compiler: staq" << std::endl;
        }
      }

      const std::string noise_model_generator_name = run_config.noise_model;
      auto noise_factory = qb::get_noise_model_factory(noise_model_generator_name);
      if (noise_factory == nullptr) {
         throw std::invalid_argument("The requested noise model of '" + noise_model_generator_name + "' is not implemented");
      }
      auto noiseModel = noise_factory->Create(run_config.num_qubits);

      size_t n_qubits = run_config.num_qubits;
      mqbacc.insert("n_qubits", n_qubits);

      if (debug_) std::cout << "# Qubits: " << n_qubits << std::endl;
      mqbacc.insert("noise-model", noiseModel.to_json());
      mqbacc.insert("m_connectivity", noiseModel.get_connectivity());
      int shots = run_config.num_shots;
      mqbacc.insert("shots", shots);
      if (debug_) std::cout << "# Shots: " << shots << std::endl;

      size_t n_samples = run_config.num_repetitions;
      if (debug_) std::cout << "# Repetitions: " << n_samples << std::endl;

      bool output_oqm_enableds = run_config.oqm_enabled;
      mqbacc.insert("output_oqm_enabled", output_oqm_enableds);
      if (debug_) {
        if (output_oqm_enableds) {
          std::cout << "# Output transpiled circuit: enabled" << std::endl;
        } else {
          std::cout << "# Output transpiled circuit: disabled" << std::endl;
        }
      }

      // Optional random seed: randomized by default.
      int random_seed = []() {
        static std::random_device dev;
        static std::mt19937 rng(dev());
        static std::uniform_int_distribution<std::mt19937::result_type> dist(
            0, std::numeric_limits<int>::max());
        return dist(rng);
      }();
      // User-provided random seed
      if (!seeds_.empty()) {
        ValidatorTwoDim<VectorN, size_t> seeds_valid(
            seeds_, 0, std::numeric_limits<int>::max(), " random seed [seed] ");
        const int seed = seeds_valid.get(ii, jj);
        mqbacc.insert("seed", seed);
        random_seed = seed;
        if (debug_) {
          std::cout << "# Seed value: " << seed << std::endl;
        }
      }

      std::string accs = run_config.acc_name;
      bool exec_on_hardware = false;

      // Read JSON configuration qpu_config
      std::string qcv = run_config.qpu_config_json_filepath;
      if (debug_)
        std::cout << "[debug]: JSON configuration for QB hardware in file: " << qcv
                  << std::endl;
      std::ifstream tifs(qcv);
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
            VALID_QB_HARDWARE_OVER_REQUEST_FACTORS[tkv["acc"]] = tkv["over_request"];
            VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS[tkv["acc"]] = tkv["recursive_request"];
            VALID_QB_HARDWARE_RESAMPLE_ENABLEDS[tkv["acc"]] = tkv["resample"];
            VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES[tkv["acc"]] = tkv["resample_above_percentage"];
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
            std::cout << "* Final VALID_QB_HARDWARE_POLLING_RETRY_LIMITS:" << std::endl;
            for (const auto &it : VALID_QB_HARDWARE_POLLING_RETRY_LIMITS) {
              std::cout << it.first << " : " << it.second << std::endl;
            }
            std::cout << "* Final VALID_QB_HARDWARE_OVER_REQUEST_FACTORS:" << std::endl;
            for (const auto &it : VALID_QB_HARDWARE_OVER_REQUEST_FACTORS) {
              std::cout << it.first << " : " << it.second << std::endl;
            }
            std::cout << "* Final VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS:" << std::endl;
            for (const auto &it : VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS) {
              std::cout << it.first << " : " << it.second << std::endl;
            }
            std::cout << "* Final VALID_QB_HARDWARE_RESAMPLE_ENABLEDS:" << std::endl;
            for (const auto &it : VALID_QB_HARDWARE_RESAMPLE_ENABLEDS) {
              std::cout << it.first << " : " << it.second << std::endl;
            }
            std::cout << "* Final VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES:" << std::endl;
            for (const auto &it : VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES) {
              std::cout << it.first << " : " << it.second << std::endl;
            }
          }
        }

        if (debug_)
          std::cout << "[debug]: " << std::endl << std::endl;
      } else {
        if (debug_)
          std::cout
              << "[debug]: Could not find the JSON configuration file for QB "
                 "hardware named: "
              << qcv << std::endl;
      }

      // Check the list of hardware accelerators...
      // If a hardware accelerator was selected...
      // then we keep acc = "tnqvm"
      // and we also set exec_on_hardware = true;

      if (VALID_QB_HARDWARE_ACCS.find(accs) == VALID_QB_HARDWARE_ACCS.end()) {
          exec_on_hardware = false;
          if (debug_) {
              std::cout << "# QB hardware accelerator: disabled" << std::endl;
          }
      } else {
          exec_on_hardware = true;
          if (debug_) {
              std::cout << "# QB hardware accelerator: enabled" << std::endl;
          }
          // We force accs = "tnqvm" when QB hardware execution is enabled
          accs = "tnqvm";
      }

      bool noises = run_config.noise;
      bool verbatim = run_config.aws_verbatim;

      int max_bond_dimension = run_config.max_bond_tnqvm;
      double svd_cutoff = run_config.svd_cutoff_tnqvm;

      // A QCStack client - provide argument 'true' for debug mode
      std::shared_ptr<xacc::Client> qcs_qdk = std::make_shared<xacc::QCStackClient>();

      std::shared_ptr<xacc::quantum::QuantumBrillianceRemoteAccelerator> tqdk =
       std::make_shared<xacc::quantum::QuantumBrillianceRemoteAccelerator>(qcs_qdk);

      auto qpu = xacc::getAccelerator(accs, {{"seed", random_seed}});
      if (accs.compare("tnqvm") == 0) {
        xacc::set_verbose(false);
        qpu = xacc::getAccelerator(
            "tnqvm", {
                         std::make_pair("tnqvm-visitor", "exatn-mps"),
                         std::make_pair("max-bond-dim", max_bond_dimension),
                         std::make_pair("svd-cutoff", svd_cutoff)
                         // , std::make_pair("shots", shots)
                     });
      } else if (accs.compare("aer") == 0) {
        xacc::HeterogeneousMap aer_options{{"seed", random_seed},
                                           {"shots", shots}};
        if (!aer_sim_types_.empty()) {
          ValidatorTwoDim<VectorString, std::string> aer_sim_types_valid(
              aer_sim_types_, VALID_AER_SIM_TYPES,
              " name of AER simulation method [aer_sim_type] ");
          const std::string sim_type = aer_sim_types_valid.get(ii, jj);
          aer_options.insert("sim-type", sim_type);
          if (debug_)
            std::cout << "# Using AER simulation method: " << sim_type
                      << std::endl;
        } else {
          if (debug_)
            std::cout << "# Using default AER simulation method." << std::endl;
        }
        if (noises > 0) {
          aer_options.insert("noise-model", noiseModel.to_json());
          if (debug_)
            std::cout << "# Noise model: enabled" << std::endl;
        } else {
          if (debug_)
            std::cout << "# Noise model: disabled" << std::endl;
        }
        // Get AER and initialize it with proper settings.
        qpu = xacc::getAccelerator(accs, aer_options);
      } else if (accs == "aws_acc") {
        // AWS
        const std::string aws_format = run_config.aws_format;
        const std::string aws_device_names = run_config.aws_device_name;
        const std::string aws_s3 = run_config.aws_s3;
        const std::string aws_s3_path = run_config.aws_s3_path;
        //
        // Extra validation for AWS Braket
        //
        // Revalidate some limits that are conditional on the AWS Device selected:
        // Handle limits for AWS SV1, DM1 and TN1 for shots and available qubits
        if (aws_device_names.compare("DM1") == 0) {
          ValidatorTwoDim<VectorN, size_t> qns_dm1_valid(
              qns_, session::QNS_DM1_LOWERBOUND, session::QNS_DM1_UPPERBOUND,
              " number of qubits, AWS Braket DM1 [qn] ");
          if (qns_dm1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_dm1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_dm1_valid(
              sns_, session::SNS_DM1_LOWERBOUND, session::SNS_DM1_UPPERBOUND,
              " number of shots, AWS Braket DM1 [sn] ");
          if (sns_dm1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_dm1_valid.get(ii, jj);
        }
        if (aws_device_names.compare("SV1") == 0) {
            ValidatorTwoDim<VectorN, size_t> qns_sv1_valid(
              qns_, session::QNS_SV1_LOWERBOUND, session::QNS_SV1_UPPERBOUND,
              " number of qubits, AWS Braket SV1 [qn] ");
          if (qns_sv1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_sv1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_sv1_valid(
              sns_, session::SNS_SV1_LOWERBOUND, session::SNS_SV1_UPPERBOUND,
              " number of shots, AWS Braket SV1 [sn] ");
          if (sns_sv1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_sv1_valid.get(ii, jj);
        }
        if (aws_device_names.compare("TN1") == 0) {
            ValidatorTwoDim<VectorN, size_t> qns_tn1_valid(
              qns_, session::QNS_TN1_LOWERBOUND, session::QNS_TN1_UPPERBOUND,
              " number of qubits, AWS Braket TN1 [qn] ");
          if (qns_tn1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_tn1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_tn1_valid(
              sns_, session::SNS_TN1_LOWERBOUND, session::SNS_TN1_UPPERBOUND,
              " number of shots, AWS Braket TN1 [sn] ");
          if (sns_tn1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_tn1_valid.get(ii, jj);
        }
        mqbacc.insert("format", aws_format);
        mqbacc.insert("device", aws_device_names);
        mqbacc.insert("s3", aws_s3);
        mqbacc.insert("path", aws_s3_path);
        mqbacc.insert("verbatim", verbatim);
        mqbacc.insert("noise", noises);
        mqbacc.insert("shots", shots);
        //
      } else if (accs.compare("qb-lambda") == 0) {
        std::string lambda_url =
            "ec2-3-26-79-252.ap-southeast-2.compute.amazonaws.com";  // Default AWS reverse proxy server for the QB Lambda workstation
            //"10.10.8.50:4000"; // internal address
        if (VALID_QB_HARDWARE_URLS.find("qb-lambda") !=
            VALID_QB_HARDWARE_URLS.end()) {
          lambda_url = VALID_QB_HARDWARE_URLS["qb-lambda"];
        }
        if (debug_) {
          std::cout << "Execute on Lambda workstation @ " << lambda_url
                    << std::endl;
        }

        if (noises) {
          qpu = xacc::getAccelerator("qb-lambda",
                                     {{"device", "GPU"},
                                      {"url", lambda_url},
                                      {"noise-model", noiseModel.to_json()}});
          if (debug_) {
            std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
          }
        } else {
          qpu = xacc::getAccelerator("qb-lambda",
                                     {{"device", "GPU"}, {"url", lambda_url}});
          if (debug_) {
            std::cout << "# Noise model: disabled" << std::endl;
          }
        }
      } else if (accs == "qsim" && noises) {
        // Use qsim via Cirq wrapper to handle noise if requested
        // The "cirq-qsim" backend is part of the external emulator package
        // to be used with the qb emulator noise models only.
        if (run_config.noise_model != "qb-nm1" && run_config.noise_model != "qb-nm2") {
          // We don't support arbitrary noise model in qsim yet.
          // Hence, ignore the noise request. Log info to let users know.
          std::cout << "# 'qsim' doesn't support the request noise configuration. Disable noise." << std::endl;
          std::cout << "# Please use one of QB noise models (e.g., 'qb-nm1' or 'qb-nm2') from the emulator package (extra installation)." << std::endl;
        } else {
          // Switch to "cirq-qsim" from emulator package
          qpu = xacc::getAccelerator("cirq-qsim");
          mqbacc.insert("noise-model-name", run_config.noise_model);
          if (debug_) {
            std::cout << "# Noise model for qsim (from emulator package): enabled" << std::endl;
          }
        }
      }

      if (exec_on_hardware) {
        // Extra information for the JSON payload sent to QB hardware
        std::string hwacc = run_config.acc_name;

        auto in_hw_urls = VALID_QB_HARDWARE_URLS.find(hwacc);
        if (in_hw_urls != VALID_QB_HARDWARE_URLS.end()) {
          if (debug_) {
            std::cout << "# QB hardware accelerator URL: " << in_hw_urls->second
                      << std::endl;
          }
          mqbacc.insert("remote_url", in_hw_urls->second);
        }

        auto in_hw_postpaths = VALID_QB_HARDWARE_POSTPATHS.find(hwacc);
        if (in_hw_postpaths != VALID_QB_HARDWARE_POSTPATHS.end()) {
          if (debug_) {
            std::cout << "# QB hardware accelerator POST path: "
                      << in_hw_postpaths->second << std::endl;
          }
          mqbacc.insert("post_path", in_hw_postpaths->second);
        }

        auto in_hw_over_request_factor = VALID_QB_HARDWARE_OVER_REQUEST_FACTORS.find(hwacc);
        if (in_hw_over_request_factor != VALID_QB_HARDWARE_OVER_REQUEST_FACTORS.end()) {
            if (debug_) {
                std::cout << "# QB hardware over request shots by factor: "
                << in_hw_over_request_factor->second << std::endl;
            }
            mqbacc.insert("over_request", in_hw_over_request_factor->second);
        }

        auto in_hw_recursive_request = VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS.find(hwacc);
        if (in_hw_recursive_request != VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS.end()) {
            if (debug_) {
                std::cout << "# QB hardware recursive request enabled: "
                << in_hw_recursive_request->second << std::endl;
            }
            mqbacc.insert("recursive_request", in_hw_recursive_request->second);
        }

        auto in_hw_resample = VALID_QB_HARDWARE_RESAMPLE_ENABLEDS.find(hwacc);
        if (in_hw_resample != VALID_QB_HARDWARE_RESAMPLE_ENABLEDS.end()) {
            if (debug_) {
                std::cout << "# QB hardware sample-with-replacement enabled: "
                << in_hw_resample->second << std::endl;
            }
            mqbacc.insert("resample", in_hw_resample->second);
        }

        auto in_hw_resample_above_percentage = VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES.find(hwacc);
        if (in_hw_resample_above_percentage != VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES.end()) {
            if (debug_) {
                std::cout << "# QB hardware sample-with-replacement enforced above threshold (%): "
                << in_hw_resample_above_percentage->second << std::endl;
            }
            mqbacc.insert("resample_above_percentage", in_hw_resample_above_percentage->second);
        }

        std::vector<int> init_qubits(n_qubits, VALID_QB_HARDWARE_INIT);
        mqbacc.insert("init", init_qubits);
        mqbacc.insert("request_id", VALID_QB_HARDWARE_REQUEST_ID);
        mqbacc.insert("poll_id", VALID_QB_HARDWARE_POLL_ID);
        mqbacc.insert("cycles", VALID_QB_HARDWARE_CYCLES);
        mqbacc.insert("use_default_contrast_settings", run_config.use_default_contrast_setting);
        mqbacc.insert("init_contrast_thresholds", run_config.init_contrast_thresholds);
        mqbacc.insert("qubit_contrast_thresholds", run_config.qubit_contrast_thresholds);
        if (hwacc.compare("qdk_gen1") == 0) {
          // QDK specific fields
          
        } else if (hwacc.compare("dqc_gen1") == 0) {
          // DQC specific fields
        
        }
      }
      qpu->updateConfiguration(mqbacc);
      if (exec_on_hardware) {
          tqdk->updateConfiguration(mqbacc);
          if (debug_)
            std::cout << "# " << run_config.acc_name << " accelerator: initialised" << std::endl;
      }
      if (debug_)
        std::cout << "# " << accs << " accelerator: initialised" << std::endl;

      std::shared_ptr<xacc::quantum::QuantumBrillianceAccelerator> acc =
          std::make_shared<xacc::quantum::QuantumBrillianceAccelerator>();
      acc->updateConfiguration(mqbacc);
      if (debug_) std::cout << "# Quantum Brilliance accelerator: initialised" << std::endl;

      // Execute on simulator (qpu)
      auto buffer_b = xacc::qalloc(n_qubits);

      // Get the composite IR to run:
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> citargets;
      if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_IR)
      {
        // Direct IR input (e.g., circuit builder)
        citargets.push_back(irtarget_ms_.at(ii).at(0));
      }
      else
      {
        // String input -> compile
        const std::string target_circuit = get_target_circuit_qasm_string(ii, jj, run_config);
        citargets.push_back(compile_input(target_circuit, n_qubits, run_config.source_type));
      }

      xacc::HeterogeneousMap m;
      // Transform the target to account for QB topology: XACC
      // "swap-shortest-path"
      if (!noplacement) {
        if (debug_)
          std::cout << "# Quantum Brilliance topology placement: enabled"
                    << std::endl;
        std::string placement = "swap-shortest-path";
        if (!placements_.empty()) {
          ValidatorTwoDim<VectorString, std::string> placements_valid(
              placements_, VALID_HARDWARE_PLACEMENTS,
              " name of placement module");
          const std::string placement_opt = placements_valid.get(ii, jj);

          if (debug_) {
            std::cout << "# Placement: " << placement_opt << std::endl;
          }
          if (xacc::getIRTransformation(placement_opt)) {
            placement = placement_opt;
          } else {
            std::cout << "Placement module '" << placement_opt
                      << "' cannot be located. Please check your installation.\n";
          }
        }
        if (accs == "aws_acc") {
          m.merge(qpu->getProperties());
        }
        auto A = xacc::getIRTransformation(placement);
        A->apply(citargets.at(0), acc, m);
      }

        // Perform circuit optimisation: XACC "circuit-optimizer"
        if (!nooptimise) {
          if (debug_)
            std::cout << "# Quantum Brilliance circuit optimiser: enabled" << std::endl;
          auto O = xacc::getIRTransformation("circuit-optimizer");
          O->apply(citargets.at(0), acc, m);
        }

        buffer_b->resetBuffer();

        xacc::ScopeTimer timer_for_qpu(
            "Walltime, in ms, for simulator to execute quantum circuit", false);
        if (!nosim && !exec_on_hardware) {
          try {
            // There were error mitigation settings
            if (!error_mitigations_.empty()) {
              if (noises) {
                ValidatorTwoDim<VectorString, std::string>
                    error_mitigations_valid(
                        error_mitigations_, VALID_ERROR_MITIGATIONS,
                        " name of error mitigation module [error_mitigation] ");
                const std::string noise_mitigation =
                    error_mitigations_valid.get(ii, jj);
                if (debug_) {
                  std::cout << "# Noise mitigation: " << noise_mitigation
                            << std::endl;
                }
                auto noise_mitigated_acc = [&]() {
                  if (noise_mitigation == "rich-extrap") {
                    // Noise scaling factors that we use
                    const std::vector<int> noise_scalings{1, 3, 5};
                    return xacc::getAcceleratorDecorator(
                        noise_mitigation, qpu,
                        {{"scale-factors", noise_scalings}});
                  }

                  return xacc::getAcceleratorDecorator(noise_mitigation, qpu);
                }();
                if (noise_mitigated_acc) {
                  qpu = noise_mitigated_acc;
                } else {
                  std::cout << "Noise mitigation module '" << noise_mitigation
                            << "' was not properly initialized. Please check "
                               "your installation.\n";
                }
              } else {
                // Noise was disabled, nothing to do
                if (debug_) {
                  std::cout
                      << "# Noise was set to False. Error mitigation is skipped."
                      << std::endl;
                }
              }
            }

            qpu->execute(buffer_b, citargets.at(0));
          } catch (...) {
            throw std::invalid_argument(
                "The simulation of your input circuit failed");
          }
        }
        if (exec_on_hardware) {
          // Store the JSON sent to the QB hardware for the user to inspect
          if (out_qbjsons_.size() < (ii + 1)) {
            if (debug_) {
              std::cout << "Resizing ii: " << (ii + 1) << std::endl;
            }
            out_qbjsons_.resize(ii + 1);
          }
          if (out_qbjsons_.at(ii).size() < (jj + 1)) {
            if (debug_) {
              std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1)
                        << std::endl;
            }
            out_qbjsons_.at(ii).resize(jj + 1);
          }
          out_qbjsons_.at(ii).at(jj) =
              tqdk->getNativeCode(citargets.at(0), mqbacc);

          try {
            tqdk->validate_capability();
          } catch (...) {
            throw std::runtime_error("Please recheck your hardware settings");
          }

          try {
            tqdk->execute(buffer_b, citargets);
          } catch (...) {
            throw std::invalid_argument(
                "The execution on hardware of your input circuit failed");
          }

          // Set up polling
          std::string exhwacc = run_config.acc_name;
          int polling_interval = 1;
          int polling_attempts = 1;
          auto k_polling_interval = VALID_QB_HARDWARE_POLLING_SECS.find(exhwacc);
          if (k_polling_interval != VALID_QB_HARDWARE_POLLING_SECS.end()) {
            if (debug_) {
              std::cout
                  << "# QB hardware accelerator polling interval, in seconds: "
                  << k_polling_interval->second << std::endl;
            }
            polling_interval = k_polling_interval->second;
          }
          auto k_polling_attempts =
              VALID_QB_HARDWARE_POLLING_RETRY_LIMITS.find(exhwacc);
          if (k_polling_attempts != VALID_QB_HARDWARE_POLLING_RETRY_LIMITS.end()) {
            if (debug_) {
              std::cout << "# QB hardware accelerator polling max. attempts: "
                        << k_polling_attempts->second << std::endl;
            }
            polling_attempts = k_polling_attempts->second;
          }
          using namespace std::chrono_literals;
          for (int i = 0; i < polling_attempts; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(polling_interval));
            if (debug_) {
              std::cout << "# Waited for " << polling_interval << " seconds"
                        << std::endl;
            }
            int poll_return = POLLING_NOT_READY;

            // Accumulate counts in a map of string -> int
            std::map<std::string, int> counts;

            poll_return = tqdk->pollForResults(buffer_b, citargets, counts, polling_interval, polling_attempts);

            if (debug_) {
              std::cout << "# Polling returned status: " << poll_return << std::endl;
            }
            if (poll_return == POLLING_SUCCESS) {
              break;
            }
          }
        }

        double xacc_scope_timer_qpu_ms = timer_for_qpu.getDurationMs();
        if (debug_) {
          std::cout << std::endl;
          buffer_b->print();
          std::cout << std::endl;
          std::cout << "Walltime elapsed for the simulator to perform the "
                       "requested number of shots of the quantum circuit, in ms: "
                    << xacc_scope_timer_qpu_ms << std::endl;
          std::cout << std::endl;
          std::cout << "Bit order [0=>LSB, 1=>MSB]: " << qpu->getBitOrder()
              << " : Important note : TNQVM reports incorrect bit order - it "
                 "uses LSB"
              << std::endl;
        }

        // Store indicator of LSB pattern
        // 0 => LSB
        acc_uses_lsbs_.at(ii).at(jj) = (qpu->getBitOrder() == 0);
        // Workaround for incorrect TNQVM reporting of LSB/MSB ordering
        if (accs.compare("tnqvm") == 0) {
          acc_uses_lsbs_.at(ii).at(jj) = false;
        }
        // Workaround for aer reverse ordering
        // Also for aer, keep the qobj so that a user can call Aer standalone later
        if (accs.compare("aer") == 0) {
          acc_uses_lsbs_.at(ii).at(jj) = true;
          out_qobjs_.at(ii).at(jj) = qpu->getNativeCode(citargets.at(0), mqbacc);
        }

        // Get counts
        std::map<std::string, int> qpu_counts = buffer_b->getMeasurementCounts();
        // Get Z operator expectation:
        double z_expectation_val = -99.99;
        if (!nosim) {
          if (!exec_on_hardware) {   // temporary until correct state strings are returned by QDK Server - 211029
            z_expectation_val = getExpectationValueZ(buffer_b);
            if (debug_)
              std::cout << "* Z-operator expectation value: " << z_expectation_val << std::endl;
          }
        }
        
        // Save Z-operator expectation value to VectorMapND
        ND res_z{{0, z_expectation_val}};
        out_z_op_expects_.at(ii).at(jj) = res_z;

        // Save the counts to VectorMapNN in out_counts_ and raw map data in
        // out_raws
        populate_measure_counts_data(ii, jj, qpu_counts);

        // Transpile to QB native gates (acc)
        if (run_config.oqm_enabled) {
          auto buffer_qb = xacc::qalloc(n_qubits);

          buffer_qb->resetBuffer();
          try {
            // acc->execute(buffer_qb, irtarget->getComposite("QBCIRCUIT"));
            acc->execute(buffer_qb, citargets.at(0));
          } catch (...) {
            throw std::invalid_argument(
                "Transpiling to QB native gates for your input circuit failed");
          }

          // Save the transpiled circuit string
          out_transpiled_circuits_.at(ii).at(jj) = acc->getTranspiledResult();

          // Invoke the Profiler
          Profiler timing_profile(acc->getTranspiledResult(), n_qubits,
                                  debug_);

          // Save single qubit gate qtys to VectorMapNN
          out_single_qubit_gate_qtys_.at(ii).at(jj) =
              timing_profile.get_count_1q_gates_on_q();

          // Save two-qubit gate qtys to VectorMapNN
          out_double_qubit_gate_qtys_.at(ii).at(jj) =
              timing_profile.get_count_2q_gates_on_q();

          // Save timing results to VectorMapND
          out_total_init_maxgate_readout_times_.at(ii).at(jj) =
              timing_profile.get_total_initialisation_maxgate_readout_time_ms(
                  xacc_scope_timer_qpu_ms, shots);
        }
      }
  }

  void session::run(const size_t &ii) {}

  void session::run() {
    if (debug_) std::cout << "Invoked run()" << std::endl;

    // Shape consistency
    const int N_ii = is_ii_consistent();
    const int N_jj = is_jj_consistent();
    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    if (debug_) {
      std::cout << "N_ii: " << N_ii << std::endl;
    }
    if (debug_) {
      std::cout << "N_jj: " << N_jj << std::endl;
    }

    // Clear all stored results:
    out_raws_.clear();
    out_raws_.resize(N_ii);
    for (auto el : out_raws_) {
      el.resize(N_jj);
    }
    out_counts_.clear();
    out_counts_.resize(N_ii);
    for (auto el : out_counts_) {
      el.resize(N_jj);
    }
    out_divergences_.clear();
    out_divergences_.resize(N_ii);
    for (auto el : out_divergences_) {
      el.resize(N_jj);
    }
    out_transpiled_circuits_.clear();
    out_transpiled_circuits_.resize(N_ii);
    for (auto el : out_transpiled_circuits_) {
      el.resize(N_jj);
    }
    out_qobjs_.clear();
    out_qobjs_.resize(N_ii);
    for (auto el : out_qobjs_) {
      el.resize(N_jj);
    }
    out_qbjsons_.clear();
    out_qbjsons_.resize(N_ii);
    for (auto el : out_qbjsons_) {
      el.resize(N_jj);
    }
    out_single_qubit_gate_qtys_.clear();
    out_single_qubit_gate_qtys_.resize(N_ii);
    for (auto el : out_single_qubit_gate_qtys_) {
      el.resize(N_jj);
    }
    out_double_qubit_gate_qtys_.clear();
    out_double_qubit_gate_qtys_.resize(N_ii);
    for (auto el : out_double_qubit_gate_qtys_) {
      el.resize(N_jj);
    }
    out_total_init_maxgate_readout_times_.clear();
    out_total_init_maxgate_readout_times_.resize(N_ii);
    for (auto el : out_total_init_maxgate_readout_times_) {
      el.resize(N_jj);
    }
    acc_uses_lsbs_.clear();
    acc_uses_lsbs_.resize(N_ii);
    for (auto el : acc_uses_lsbs_) {
      el.resize(N_jj);
    }
    acc_uses_n_bits_.clear();
    acc_uses_n_bits_.resize(N_ii);
    for (auto el : acc_uses_n_bits_) {
      el.resize(N_jj);
    }

    for (int ii = 0; ii < N_ii; ii++) {
      for (int jj = 0; jj < N_jj; jj++) {
        run(ii, jj);
      }
    }
  }

  void session::qb12() {
    if (debug_) {
      std::cout
          << "Setting defaults for Quantum Brilliance 12 qubit, 1024-shot, "
             "noiseless simulation, MPS bond dimension = 256, SVD cutoff = 1.0e-8"
          << std::endl;
    }
    set_qn(12);
    set_sn(1024);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
  }

  void session::aws32dm1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket DM1 with 32 asynchronous workers, "
             "noiseless, 17-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 32; // 32 workers
    set_qn(17);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    set_aws_device_name("DM1");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  void session::aws32sv1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket SV1 with 32 asynchronous workers, "
             "noiseless, 34-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 32; // 32 workers
    set_qn(34);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    set_aws_device_name("SV1");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  void session::aws8tn1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket TN1 with 8 asynchronous workers, "
             "noiseless, 48-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 8; // 8 workers
    set_qn(48);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    set_aws_device_name("TN1");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  /// Setter for QB hardware contrast thresholds globally in a Qristal session
  void session::set_contrasts(const double &init_ct, const double &q0_ct, const double &q1_ct) {
    if (debug_) {
      std::cout
          << "Setting QB hardware init contrast threshold: " << init_ct << "\n"
          << "Setting QB hardware qubit[0] final readout contrast threshold: " << q0_ct << "\n"
          << "Setting QB hardware qubit[1] final readout contrast threshold: " << q1_ct << "\n"
          << std::endl;
    }
    bool use_default_ct = false;
    ND qubit_ct_nd{{0, q0_ct}, {1, q1_ct}};
    set_init_contrast_threshold(init_ct);
    set_qubit_contrast_threshold(qubit_ct_nd);
    use_default_contrast_settings_ = {{use_default_ct}};
  }

  /// Setter that clears all contrast thresholds
  void session::reset_contrasts() {
    use_default_contrast_settings_ = {{true}};
    ((init_contrast_thresholds_.at(0)).at(0)).clear();
    ((qubit_contrast_thresholds_.at(0)).at(0)).clear();
  }

  /// Util method to compile input source string into IR
  /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
  std::shared_ptr<xacc::CompositeInstruction> session::compile_input(const std::string& in_source_string, int in_num_qubits,
                                                                  source_string_type in_source_type)
  {
    // Retrieve the compiler instance for the QASM dialect
    auto compiler = [&]() -> std::shared_ptr<xacc::Compiler>
    {
      switch (in_source_type)
      {
      case source_string_type::OpenQASM:
        return xacc::getCompiler("staq");
      case source_string_type::XASM:
        return xacc::getCompiler("xasm");
      case source_string_type::Quil:
        return xacc::getCompiler("quil");
      default:
        __builtin_unreachable();
      }
    }();

    // Get buffer name from the kernel signature
    const std::string buffer_name = [](const std::string& kernel_src)
    {
      const std::regex kernelSignatureRegex(R"(__qpu__[ \t]void[ \t]QBCIRCUIT\(qreg[ \t]([a-zA-Z_$][a-zA-Z_$0-9]*)\))");
      std::smatch matches;
      std::string qRegName = "q";
      if (std::regex_search(kernel_src, matches, kernelSignatureRegex))
      {
        qRegName = matches[1].str();
      }
      return qRegName;
    }(in_source_string);

    auto src_str = in_source_string;
    // For OpenQASM: inject the buffer declaration into the source code so that the compiler doesn't need to do global
    // look-up into the static buffer map. This is to guarantee thread-safety in case multiple kernels using the same
    // qubit register name.
    if (in_source_type == source_string_type::OpenQASM && in_source_string.find("__qpu__") != std::string::npos)
    {
      std::string tmp            = "";
      const auto first_brace_pos = in_source_string.find_first_of("{");
      const auto last_brace_pos  = in_source_string.find_last_of("}");
      const auto sub             = in_source_string.substr(first_brace_pos + 1, last_brace_pos - first_brace_pos - 1);
      auto lines                 = xacc::split(sub, '\n');
      // Flag to insert register declaration only once after the first include.
      bool added_reg_decl = false;
      for (auto& l : lines)
      {
        xacc::trim(l);
        tmp += l + "\n";
        if (l.find("include") != std::string::npos)
        {
          // Add the qreg declaration with the proper size
          tmp += "qreg " + buffer_name + "[" + std::to_string(in_num_qubits) + "];\n";
          added_reg_decl = true;
        }
      }
      // Replace the source string
      src_str = tmp;
    }


    // Compile source string to IR
    auto irtarget = compiler->compile(src_str);
    // Get the kernel composite instruction (quantum circuit)
    return irtarget->getComposites().front();
  }

  void session::set_parallel_run_config(const std::string &in_config) {
    // Check if this is a file:
    std::ifstream config_file(in_config);
    if (config_file) {
      const std::string config_str((std::istreambuf_iterator<char>(config_file)),
                                   std::istreambuf_iterator<char>());
      executor_->initialize(config_str);
    } else {
      executor_->initialize(in_config);
    }
  }

  Executor &session::get_executor() { return *executor_; }

  // !IMPORTANT! Make sure this function stays threadsafe
  std::shared_ptr<async_job_handle> session::run_async(const std::size_t ii, const std::size_t jj,
                       std::shared_ptr<xacc::Accelerator> accelerator) {

    if (debug_) {
      std::stringstream msg;
      msg << "thread " << std::this_thread::get_id() << " run_async (ii,jj): (" << ii << ","<< jj<<") with acc: " << accelerator << std::endl; std::cout << msg.str(); msg.str("");
    }
    // Declare the variables that persist across locked scopes
    bool output_oqm_enableds;
    size_t n_qubits;
    std::shared_ptr<xacc::quantum::QuantumBrillianceAccelerator> acc;
    std::vector<std::shared_ptr<xacc::CompositeInstruction>> citargets;
    std::shared_ptr<xacc::AcceleratorBuffer> buffer_b;
    int shots;
    // Retrieve and validate run configuration for this (ii, jj) task
    auto run_config = get_run_config(ii, jj);
    auto &qpu = accelerator;
    {
      // Lock up until the circuit is executed
      std::scoped_lock lock(m_);

      int N_ii = is_ii_consistent();
      int N_jj = is_jj_consistent();

      if (N_ii < 0) {
        throw std::range_error("Leading dimension has inconsistent length");
      }

      if (N_jj < 0) {
        throw std::range_error("Second dimension has inconsistent length");
      }

      // Resize result tables if necessary.
      ensure_results_table_size(ii, jj);
      // xacc::setIsPyApi();
      // xacc::set_verbose(true);
      xacc::HeterogeneousMap mqbacc;
      // bool noplacement = noplacements_valid.get(ii, jj);
      // bool nooptimise = nooptimises_valid.get(ii, jj);
      bool nosim = run_config.no_sim;

      if (debug_)
      {
        switch (run_config.source_type)
        {
        case source_string_type::XASM:
          std::cout << "# XASM compiler: xasm" << std::endl;
          break;
        case source_string_type::Quil:
          std::cout << "# Quil v1 compiler: quil" << std::endl;
          break;
        case source_string_type::OpenQASM:
          std::cout << "# OpenQASM compiler: staq" << std::endl;
        }
      }

      n_qubits = run_config.num_qubits;
      const std::string noise_model_generator_name = run_config.noise_model;
      auto noise_factory = qb::get_noise_model_factory(noise_model_generator_name);
      if (noise_factory == nullptr) {
         throw std::invalid_argument("The requested noise model of '" + noise_model_generator_name + "' is not implemented");
      }
      auto noiseModel = noise_factory->Create(run_config.num_qubits);

      mqbacc.insert("n_qubits", n_qubits);

      if (debug_) {
        std::cout << "# Qubits: " << n_qubits << std::endl;
      }

      mqbacc.insert("noise-model", noiseModel.to_json());
      mqbacc.insert("m_connectivity", noiseModel.get_connectivity());
      shots = run_config.num_shots;
      mqbacc.insert("shots", shots);
      if (debug_){
        std::cout << "# Shots: " << shots << std::endl;
      }

      size_t n_samples = run_config.num_repetitions;
      if (debug_){
        std::cout << "# Repetitions: " << n_samples << std::endl;
      }

      output_oqm_enableds = run_config.oqm_enabled;
      mqbacc.insert("output_oqm_enabled", output_oqm_enableds);
      if (debug_) {
        if (output_oqm_enableds) {
          std::cout << "# Output transpiled circuit: enabled" << std::endl;
        } else {
          std::cout << "# Output transpiled circuit: disabled" << std::endl;
        }
      }

      // Not implemented here, but present in sequential run():
      //
      // Optional random seed: randomised by default
      //
      // User-provided random seed
      //
      // exec_on_hardware
      //

      // Check the list of hardware accelerators...
      // If a hardware accelerator was selected...
      // then we keep acc = "tnqvm"
      // and we also set exec_on_hardware = true;
      bool noises = run_config.noise;
      int max_bond_dimension = run_config.max_bond_tnqvm;
      bool verbatim = run_config.aws_verbatim;

      double svd_cutoff = run_config.svd_cutoff_tnqvm;

      // Not implemented here, but present in sequential run():
      //
      // QCStack client
      //
      // xacc::quantum::QuantumBrillianceRemoteAccelerator
      //
      // random_seed


      if (qpu->name() == "tnqvm") {
        xacc::set_verbose(false);
        qpu->updateConfiguration({{"tnqvm-visitor", "exatn-mps"},
                                  {"max-bond-dim", max_bond_dimension},
                                  {"svd-cutoff", svd_cutoff},
                                  {"shots", shots}});
      } else if (qpu->name() == "aer") {
        //
        // Not implemented here but present in sequential run()
        //
        // aer_options + random_seed
        //
        // aer_sim_types + sim-type
        //

        if (noises > 0) {
          qpu->updateConfiguration(
              {{"noise-model", noiseModel.to_json()}, {"shots", shots}});
          if (debug_){
            std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
          }
        } else {
          if (debug_){
            std::cout << "# Noise model: disabled" << std::endl;
          }
        }
      } else if (qpu->name() == "aws_acc") {
        // AWS
        const std::string aws_format = run_config.aws_format;
        const std::string aws_device_names = run_config.aws_device_name;
        const std::string aws_s3 = run_config.aws_s3;
        const std::string aws_s3_path = run_config.aws_s3_path;
        //
        // Extra validation for AWS Braket
        //
        // Revalidate some limits that are conditional on the AWS Device selected:
        // Handle limits for AWS SV1, DM1 and TN1 for shots and available qubits
        if (aws_device_names.compare("DM1") == 0) {
          ValidatorTwoDim<VectorN, size_t> qns_dm1_valid(
              qns_, session::QNS_DM1_LOWERBOUND, session::QNS_DM1_UPPERBOUND,
              " number of qubits, AWS Braket DM1 [qn] ");
          if (qns_dm1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_dm1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_dm1_valid(
              sns_, session::SNS_DM1_LOWERBOUND, session::SNS_DM1_UPPERBOUND,
              " number of shots, AWS Braket DM1 [sn] ");
          if (sns_dm1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_dm1_valid.get(ii, jj);
        }
        if (aws_device_names.compare("SV1") == 0) {
            ValidatorTwoDim<VectorN, size_t> qns_sv1_valid(
              qns_, session::QNS_SV1_LOWERBOUND, session::QNS_SV1_UPPERBOUND,
              " number of qubits, AWS Braket SV1 [qn] ");
          if (qns_sv1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_sv1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_sv1_valid(
              sns_, session::SNS_SV1_LOWERBOUND, session::SNS_SV1_UPPERBOUND,
              " number of shots, AWS Braket SV1 [sn] ");
          if (sns_sv1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_sv1_valid.get(ii, jj);
        }
        if (aws_device_names.compare("TN1") == 0) {
            ValidatorTwoDim<VectorN, size_t> qns_tn1_valid(
              qns_, session::QNS_TN1_LOWERBOUND, session::QNS_TN1_UPPERBOUND,
              " number of qubits, AWS Braket TN1 [qn] ");
          if (qns_tn1_valid.is_data_empty()) {
            throw std::range_error("Number of qubits [qn] cannot be empty");
          }
          n_qubits = qns_tn1_valid.get(ii, jj);

          ValidatorTwoDim<VectorN, size_t> sns_tn1_valid(
              sns_, session::SNS_TN1_LOWERBOUND, session::SNS_TN1_UPPERBOUND,
              " number of shots, AWS Braket DM1 [sn] ");
          if (sns_tn1_valid.is_data_empty()) {
            throw std::range_error("Number of shots [sn] cannot be empty");
          }
          shots = sns_tn1_valid.get(ii, jj);
        }

        mqbacc.insert("format", aws_format);
        mqbacc.insert("device", aws_device_names);
        mqbacc.insert("s3", aws_s3);
        mqbacc.insert("path", aws_s3_path);
        mqbacc.insert("verbatim", verbatim);
        mqbacc.insert("noise", noises);
        mqbacc.insert("shots", shots);
        qpu->updateConfiguration(mqbacc);
        //
      } else if (qpu->name() == "qb-lambda") {
        //
        // Not implemented here but present in sequential run():
        //
        // qb-lambda
        //
        // Default AWS reverse proxy server for the QB Lambda workstation
        if (noises > 0) {
          qpu->updateConfiguration(
              {{"noise-model", noiseModel.to_json()}, {"shots", shots}});
          if (debug_)
            std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
        }
      } else {
        qpu->updateConfiguration({{"shots", shots}});
      }
      //
      // Not implemented here but present in sequential run():
      //
      // accs == "qsim" && noises
      //
      // exec_on_hardware
      //
      acc = std::make_shared<xacc::quantum::QuantumBrillianceAccelerator>();
      acc->updateConfiguration(mqbacc);
      if (debug_) std::cout << "# Quantum Brilliance accelerator: initialised" << std::endl;

      if (debug_) {
        std::cout << "# " << qpu->name() << " accelerator: initialised"
                  << std::endl;
      }

      // Execute on simulator (qpu)
      // auto buffer_b = xacc::qalloc(n_qubits);

      // Not implemented here but present in sequential run():
      //
      // file_or_string_or_random_or ir != session:VALID_IR

      buffer_b = std::make_shared<xacc::AcceleratorBuffer>(n_qubits);
      const auto file_or_string_or_random_or_ir = validate_infiles_instrings_randoms_irtarget_ms_nonempty(ii, jj);
      if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_IR)
      {
        // Direct IR input (e.g., circuit builder)
        citargets.push_back(irtarget_ms_.at(ii).at(0));
      }
      else
      {
        // String input -> compile
        const std::string target_circuit = get_target_circuit_qasm_string(ii, jj, run_config);
        auto composite_ir                = compile_input(target_circuit, n_qubits, run_config.source_type);
        citargets.push_back(composite_ir);
        if (irtarget_ms_.size() < (ii + 1))
        {
          if (debug_)
          {
            std::cout << "Resizing ii: " << (ii + 1) << std::endl;
          }
          irtarget_ms_.resize(ii + 1);
        }
        irtarget_ms_.at(ii) = {composite_ir};
      }

      if (debug_) {
        std::cout << "# " << " IR target: created" << "\n";
      }

      xacc::HeterogeneousMap m;

      //
      // Removed from run_async due to thread-safety issues:
      //
      // if (!noplacement) - in the upcoming SDK this is replaced with LLVM passes over QIR
      //
      // if (!nooptimise) - in the upcoming SDK this is replaced with LLVM passes over QIR
      //
      // error_mitigations - in the upcoming SDK this is replaced with LLVM passes over QIR
      //


      if (debug_)
      {
        std::cout << "Bit order [0=>LSB, 1=>MSB]: " << qpu->getBitOrder()
                  << " : Important note : TNQVM reports incorrect bit order - it "
                     "uses LSB"
                  << std::endl;
      }

      // Store indicator of LSB pattern
      // 0 => LSB
      acc_uses_lsbs_.at(ii).at(jj) = (qpu->getBitOrder() == 0);

      // Workaround for incorrect TNQVM reporting of LSB/MSB ordering
      if (qpu->name().compare("tnqvm") == 0)
      {
        acc_uses_lsbs_.at(ii).at(jj) = false;
      }
      // Workaround for aer reverse ordering
      // Also for aer, keep the qobj so that a user can call Aer standalone later
      if (qpu->name().compare("aer") == 0)
      {
        acc_uses_lsbs_.at(ii).at(jj) = true;
        out_qobjs_.at(ii).at(jj)     = qpu->getNativeCode(citargets.at(0), mqbacc);
      }
    } // End scoped_lock

    buffer_b->resetBuffer();
    xacc::ScopeTimer timer_for_qpu(
        "Walltime, in ms, for simulator to execute quantum circuit", false);
    if (!run_config.no_sim)
    {
      try
      {
        if (debug_)
        {
          std::cout << "# "
                    << " Prior to qpu->execute..."
                    << "\n";
        }
        if (qpu->name() == "aws_acc" && std::dynamic_pointer_cast<qb::remote_accelerator>(qpu))
        {
          auto as_remote_acc = std::dynamic_pointer_cast<qb::remote_accelerator>(qpu);
          // Asynchronous offload the circuit.
          auto aws_job_handle = as_remote_acc->async_execute(citargets.at(0));
          aws_job_handle->add_done_callback(
              [=](auto& handle)
              {
                auto buffer_temp = std::make_shared<xacc::AcceleratorBuffer>(buffer_b->size());
                handle.load_result(buffer_temp);
                auto qb_transpiler = std::make_shared<xacc::quantum::QuantumBrillianceAccelerator>();
                this->process_run_result(ii, jj, run_config, buffer_temp, timer_for_qpu.getDurationMs(), qb_transpiler);
              });
          return aws_job_handle;
        }
        else
        {
          // Blocking execution of a local simulator instance
          qpu->execute(buffer_b, citargets.at(0));
        }
      }
      catch (...)
      {
        throw std::invalid_argument("The simulation of your input circuit failed");
      }
    }

    /// Post-processing results for run_async with local simulators:
    /// i.e., simulation occurs on this thread.
    const double xacc_scope_timer_qpu_ms = timer_for_qpu.getDurationMs();
    // Thread-locking the post-processing step to guarantee thread safety.
    std::scoped_lock lock(m_);
    this->process_run_result(ii, jj, run_config, buffer_b, xacc_scope_timer_qpu_ms, acc);
    return nullptr;
  }


    void session::process_run_result(const std::size_t ii, const std::size_t jj, const run_i_j_config& run_config,
                                  std::shared_ptr<xacc::AcceleratorBuffer> buffer_b, double runtime_ms,
                                  std::shared_ptr<xacc::quantum::QuantumBrillianceAccelerator> qb_transpiler)
  {
    auto ir_target = irtarget_ms_.at(ii).at(0);
    // Get counts
    std::map<std::string, int> qpu_counts = buffer_b->getMeasurementCounts();
    // Get Z operator expectation:
    double z_expectation_val = -99.99;
    if (!run_config.no_sim)
    {
      z_expectation_val = buffer_b->getExpectationValueZ();
      if (debug_)
        std::cout << "* Z-operator expectation value: " << z_expectation_val << std::endl;
    }
    // Save the counts to VectorMapNN
    NN qpu_counts_nn;
    std::string keystring;
    for (auto qpu_counts_elem = qpu_counts.begin(); qpu_counts_elem != qpu_counts.end(); qpu_counts_elem++)
    {
      keystring = qpu_counts_elem->first;
      if (acc_uses_lsbs_.at(ii).at(jj))
      {
        // 0 => LSB
        std::reverse(keystring.begin(), keystring.end());
      }
      qpu_counts_nn.insert(std::make_pair(std::stoi(keystring, 0, 2), qpu_counts_elem->second));
    }

    // Store measurement counts
    out_counts_.at(ii).at(jj) = qpu_counts_nn;

    // Save results to JSON
    nlohmann::json qpu_counts_js = qpu_counts;
    out_raws_.at(ii).at(jj) = qpu_counts_js.dump(4);

    // Save Z-operator expectation value to VectorMapND

    ND res_z{{0, z_expectation_val}};
    out_z_op_expects_.at(ii).at(jj) = res_z;

    // Transpile to QB native gates (acc)
    if (run_config.oqm_enabled)
    {
      auto buffer_qb = xacc::qalloc(run_config.num_qubits);

      buffer_qb->resetBuffer();
      try
      {
        // acc->execute(buffer_qb, irtarget->getComposite("QBCIRCUIT"));
        qb_transpiler->execute(buffer_qb, ir_target);
      }
      catch (...)
      {
        throw std::invalid_argument("Transpiling to QB native gates for your input circuit failed");
      }
      out_transpiled_circuits_.at(ii).at(jj) = qb_transpiler->getTranspiledResult();

      // Invoke the Profiler
      Profiler timing_profile(qb_transpiler->getTranspiledResult(), run_config.num_qubits, debug_);

      // Save single qubit gate qtys to VectorMapNN
      out_single_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_1q_gates_on_q();

      // Save two-qubit gate qtys to VectorMapNN
      out_double_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_2q_gates_on_q();

      // Save timing results to VectorMapND
      out_total_init_maxgate_readout_times_.at(ii).at(jj) =
          timing_profile.get_total_initialisation_maxgate_readout_time_ms(runtime_ms, run_config.num_shots);
    }
  }

} // namespace qb
