// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/session.hpp"

namespace qb {
//
// Getter/Setter methods for qb::session
//
void session::set_infile(const std::string &infile) {
  session::infiles_.clear();
  session::infiles_.push_back({infile});
}
void session::set_infiles(const VectorString &infiles) { session::infiles_ = infiles; }
const VectorString & session::get_infiles() const { return session::infiles_; }
//
void session::set_instring(const std::string &instring) {
  session::instrings_.clear();
  session::instrings_.push_back({instring});
}
void session::set_instrings(const VectorString &instrings) { session::instrings_ = instrings; }
const VectorString & session::get_instrings() const { return session::instrings_; }
//
void session::set_irtarget_m(const std::shared_ptr<xacc::CompositeInstruction> &irtarget_m) {
    session::irtarget_ms_.clear();
    session::irtarget_ms_.push_back({irtarget_m});
}
void session::set_irtarget_ms(const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &irtarget_ms) {
    session::irtarget_ms_ = irtarget_ms;
}
const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> & session::get_irtarget_ms() const {
    return session::irtarget_ms_;
}

//
void session::set_include_qb(const std::string &include_qb) {
  session::include_qbs_.clear();
  session::include_qbs_.push_back({include_qb});
}
void session::set_include_qbs(const VectorString &include_qbs) { session::include_qbs_ = include_qbs; }
const VectorString & session::get_include_qbs() const { return session::include_qbs_; }
//
void session::set_remote_backend_database_path(const std::string &path) {
  session::remote_backend_database_path_ = path;
}
const std::string& session::get_remote_backend_database_path() const { return session::remote_backend_database_path_; }
//
void session::set_acc(const std::string &acc) {
  session::validate_acc(acc);
  session::accs_.clear();
  session::accs_.push_back({acc});
}
void session::set_accs(const VectorString &accs) {
  for (auto item : accs) {
    for (auto im : item) {
      session::validate_acc(im);
    }
  }
  session::accs_ = accs;
}

void session::validate_acc(const std::string &acc) {
  if (VALID_ACCS.find(acc) == VALID_ACCS.end()) {
      std::stringstream listaccs;
      listaccs << "QB SDK: valid settings for acc: " << std::endl;
      for (auto it : VALID_ACCS) {
          listaccs << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listaccs.str());
  }
}
const VectorString & session::get_accs() const { return session::accs_; }
//
void session::set_aer_sim_type(const std::string &sim_type) {
  validate_aer_sim_type(sim_type);
  aer_sim_types_.clear();
  aer_sim_types_.emplace_back(std::vector<std::string>{sim_type});
}
void session::set_aer_sim_types(const VectorString &sim_types) {
  for (const auto &item : sim_types) {
    for (const auto &im : item) {
      validate_aer_sim_type(im);
    }
  }
  aer_sim_types_ = sim_types;
}
void session::validate_aer_sim_type(const std::string &sim_type) {
  if (VALID_AER_SIM_TYPES.find(sim_type) == VALID_AER_SIM_TYPES.end()) {
    std::stringstream aer_sim_type_error_msg;
    aer_sim_type_error_msg << "QB SDK: valid settings for aer_sim_type: " << std::endl;
    for (auto it : VALID_AER_SIM_TYPES) {
      aer_sim_type_error_msg << "* \"" << it << "\"" << std::endl;
    }
    throw std::range_error(aer_sim_type_error_msg.str());
  }
}
const VectorString &session::get_aer_sim_types() const { return aer_sim_types_; }

//

void session::set_random(const size_t &in_random) {
  session::randoms_.clear();
  session::randoms_.push_back({in_random});
}
void session::set_randoms(const VectorN &in_random) { session::randoms_ = in_random; }
const VectorN & session::get_randoms() const { return session::randoms_; }
//
void session::set_xasm(const bool &in_xasm) {
  session::xasms_.clear();
  session::xasms_.push_back({in_xasm});
}
void session::set_xasms(const VectorBool &in_xasm) { session::xasms_ = in_xasm; }
const VectorBool & session::get_xasms() const { return session::xasms_; }
//
void session::set_quil1(const bool &in_quil1) {
  session::quil1s_.clear();
  session::quil1s_.push_back({in_quil1});
}
void session::set_quil1s(const VectorBool &in_quil1) { session::quil1s_ = in_quil1; }
const VectorBool & session::get_quil1s() const { return session::quil1s_; }
//
void session::set_noplacement(const bool &in_noplacement) {
  session::noplacements_.clear();
  session::noplacements_.push_back({in_noplacement});
}
void session::set_noplacements(const VectorBool &in_noplacement) {
  session::noplacements_ = in_noplacement;
}
const VectorBool & session::get_noplacements() const { return session::noplacements_; }
//
void session::set_placement(const std::string &in_placement) {
  placements_.clear();
  placements_.push_back({in_placement});
}
void session::set_placements(const VectorString &in_placements) {
  placements_ = in_placements;
}
const VectorString &session::get_placements() const { return placements_; }
//
void session::set_nooptimise(const bool &in_nooptimise) {
  session::nooptimises_.clear();
  session::nooptimises_.push_back({in_nooptimise});
}
void session::set_nooptimises(const VectorBool &in_nooptimise) {
  session::nooptimises_ = in_nooptimise;
}
const VectorBool & session::get_nooptimises() const { return session::nooptimises_; }

void session::set_circuit_opt(const Passes &in_passes) {
  circuit_opts_ = {{in_passes}};
}

void session::set_circuit_opts(const Table2d<Passes> &in_passes) {
  circuit_opts_ = in_passes;
}

const Table2d<Passes> &session::get_circuit_opts() const {
  return circuit_opts_;
}

//
void session::set_nosim(const bool &in_nosim) {
  session::nosims_.clear();
  session::nosims_.push_back({in_nosim});
}
void session::set_nosims(const VectorBool &in_nosim) { session::nosims_ = in_nosim; }
const VectorBool & session::get_nosims() const { return session::nosims_; }
//
void session::set_noise(const bool &in_noise) {
  session::noises_.clear();
  session::noises_.push_back({in_noise});
}



void session::set_noises(const VectorBool &in_noise) { session::noises_ = in_noise; }
const VectorBool & session::get_noises() const { return session::noises_; }
//

void session::set_output_oqm_enabled(const bool &in_output_oqm_enabled) {
  session::output_oqm_enableds_.clear();
  session::output_oqm_enableds_.push_back({in_output_oqm_enabled});
}
void session::set_output_oqm_enableds(const VectorBool &in_output_oqm_enabled) {
  session::output_oqm_enableds_ = in_output_oqm_enabled;
}
const VectorBool & session::get_output_oqm_enableds() const {
  return session::output_oqm_enableds_;
}
//
void session::set_log_enabled(const bool &in_log_enabled) {
  session::log_enableds_.clear();
  session::log_enableds_.push_back({in_log_enabled});
}
void session::set_log_enableds(const VectorBool &in_log_enabled) {
  session::log_enableds_ = in_log_enabled;
}
const VectorBool & session::get_log_enableds() const { return session::log_enableds_; }
//
void session::set_notiming(const bool &in_notiming) {
  session::notimings_.clear();
  session::notimings_.push_back({in_notiming});
}
void session::set_notimings(const VectorBool &in_notiming) { session::notimings_ = in_notiming; }
const VectorBool & session::get_notimings() const { return session::notimings_; }
//
void session::set_qn(const size_t &in_qn) {
  session::qns_.clear();
  session::qns_.push_back({in_qn});
}
void session::set_qns(const VectorN &in_qn) { session::qns_ = in_qn; }
const VectorN & session::get_qns() const { return session::qns_; }
//
void session::set_rn(const size_t &in_rn) {
  session::rns_.clear();
  session::rns_.push_back({in_rn});
}
void session::set_rns(const VectorN &in_rn) { session::rns_ = in_rn; }
const VectorN & session::get_rns() const { return session::rns_; }
//
void session::set_sn(const size_t &in_sn) {
  session::sns_.clear();
  session::sns_.push_back({in_sn});
}
void session::set_sns(const VectorN &in_sn) { session::sns_ = in_sn; }
const VectorN & session::get_sns() const { return session::sns_; }
//
void session::set_beta(const ND &in_beta) {
  session::betas_.clear();
  session::betas_.push_back({in_beta});
}
void session::set_betas(const VectorMapND &in_beta) { session::betas_ = in_beta; }
const VectorMapND & session::get_betas() const { return session::betas_; }
//
void session::set_theta(const ND &in_theta) {
  session::thetas_.clear();
  session::thetas_.push_back({in_theta});
}
void session::set_thetas(const VectorMapND &in_theta) { session::thetas_ = in_theta; }
const VectorMapND & session::get_thetas() const { return session::thetas_; }
//

void session::set_initial_bond_dimension(const size_t &in_initial_bond_dimension) {
  session::initial_bond_dimensions_.clear();
  session::initial_bond_dimensions_.push_back({in_initial_bond_dimension});
}
void session::set_initial_bond_dimensions(const VectorN &in_initial_bond_dimension) { session::initial_bond_dimensions_ = in_initial_bond_dimension; }
const VectorN & session::get_initial_bond_dimensions() const { return session::initial_bond_dimensions_; }
//
void session::set_initial_kraus_dimension(const size_t &in_initial_kraus_dimension) {
  session::initial_kraus_dimensions_.clear();
  session::initial_kraus_dimensions_.push_back({in_initial_kraus_dimension});
}
void session::set_initial_kraus_dimensions(const VectorN &in_initial_kraus_dimension) { session::initial_kraus_dimensions_ = in_initial_kraus_dimension; }
const VectorN & session::get_initial_kraus_dimensions() const { return session::initial_kraus_dimensions_; }
//
void session::set_max_bond_dimension(const size_t &in_max_bond_dimension) {
  session::max_bond_dimensions_.clear();
  session::max_bond_dimensions_.push_back({in_max_bond_dimension});
}
void session::set_max_bond_dimensions(const VectorN &in_max_bond_dimension) { session::max_bond_dimensions_ = in_max_bond_dimension; }
const VectorN & session::get_max_bond_dimensions() const { return session::max_bond_dimensions_; }
//
void session::set_max_kraus_dimension(const size_t &in_max_kraus_dimension) {
  session::max_kraus_dimensions_.clear();
  session::max_kraus_dimensions_.push_back({in_max_kraus_dimension});
}
void session::set_max_kraus_dimensions(const VectorN &in_max_kraus_dimension) { session::max_kraus_dimensions_ = in_max_kraus_dimension; }
const VectorN & session::get_max_kraus_dimensions() const { return session::max_kraus_dimensions_; }
//
void session::set_svd_cutoff(const ND &in_svd_cutoff) {
  session::svd_cutoffs_.clear();
  session::svd_cutoffs_.push_back({in_svd_cutoff});
}
void session::set_svd_cutoffs(const VectorMapND &in_svd_cutoff) { session::svd_cutoffs_ = in_svd_cutoff; }
const VectorMapND & session::get_svd_cutoffs() const { return session::svd_cutoffs_; }
//
void session::set_rel_svd_cutoff(const ND &in_rel_svd_cutoff) {
  session::rel_svd_cutoffs_.clear();
  session::rel_svd_cutoffs_.push_back({in_rel_svd_cutoff});
}
void session::set_rel_svd_cutoffs(const VectorMapND &in_rel_svd_cutoff) { session::rel_svd_cutoffs_ = in_rel_svd_cutoff; }
const VectorMapND & session::get_rel_svd_cutoffs() const { return session::rel_svd_cutoffs_; }
//
void session::set_noise_model(const NoiseModel &noise_model) {
  noise_models_.clear();
  noise_models_.emplace_back(std::vector<NoiseModel>{noise_model});
}
void session::set_noise_models(const std::vector<std::vector<NoiseModel>> &noise_models) {
  noise_models_ = noise_models;
}
const std::vector<std::vector<NoiseModel>> &session::get_noise_models() const { return noise_models_; }
//
void session::set_output_amplitude(const std::map<
    std::string, std::complex<double>> &in_output_amplitude) {
  session::output_amplitudes_.clear();
  session::output_amplitudes_.push_back({in_output_amplitude});
}
void session::set_output_amplitudes(const std::vector<std::vector<std::map<
    std::string, std::complex<double>>>> &in_output_amplitude) {
  session::output_amplitudes_ = in_output_amplitude;
}
const std::vector<std::vector<std::map<std::string, std::complex<double>>>> &
    session::get_output_amplitudes() const { return session::output_amplitudes_; }
//
void session::set_debug(const bool & debug) {
  session::debug_ = debug;
}
const bool & session::get_debug() const { return session::debug_; }
//
void session::setName(const VectorString &name_) { session::name_m = name_; }
void session::setName(const std::string &name_) { session::name_m.push_back({name_}); }
const VectorString & session::getName() const { return session::name_m; }

const VectorString & session::get_out_raws() const { return session::out_raws_ ; }
//

const std::vector<std::vector<std::map<std::string, int>>> & session::get_out_bitstrings() const {
  return session::out_bitstrings_ ;
}
//

const VectorMapND & session::get_out_divergences() const { return session::out_divergences_ ; }
//

const VectorString & session::get_out_transpiled_circuits() const { return session::out_transpiled_circuits_ ; }
//

const VectorString & session::get_out_qobjs() const { return session::out_qobjs_ ; }
//

const VectorString & session::get_out_qbjsons() const { return session::out_qbjsons_ ; }
//

const VectorMapNN & session::get_out_single_qubit_gate_qtys() const { return session::out_single_qubit_gate_qtys_ ; }
//

const VectorMapNN & session::get_out_double_qubit_gate_qtys() const { return session::out_double_qubit_gate_qtys_ ; }
//

const VectorMapND & session::get_out_total_init_maxgate_readout_times() const { return session::out_total_init_maxgate_readout_times_ ; }
//

const VectorMapND & session::get_out_z_op_expects() const { return session::out_z_op_expects_ ; }
//

const std::shared_ptr<std::vector<std::complex<double>>> & session::get_state_vec_raw() const {return session::state_vec_ ;}
//

void session::get_state_vec(const bool &in_get_state_vec) {session::in_get_state_vec_ = in_get_state_vec ;}
//
void session::set_noise_mitigation(const std::string &noise_mitigation) {
  session::validate_noise_mitigation(noise_mitigation);
  session::error_mitigations_.clear();
  session::error_mitigations_.push_back({noise_mitigation});
}
//
void session::set_seed(const size_t &in_seed) {
  session::seeds_.clear();
  session::seeds_.push_back({in_seed});
}
void session::set_seeds(const VectorN &in_seeds) { session::seeds_ = in_seeds; }
const VectorN & session::get_seeds() const { return session::seeds_; }
//
void session::set_noise_mitigations(const VectorString &noise_mitigations) {
  for (const auto& item : noise_mitigations) {
    for (const auto& im : item) {
      session::validate_noise_mitigation(im);
    }
  }
  session::error_mitigations_ = noise_mitigations;
}

void session::validate_noise_mitigation(const std::string &noise_mitigation) {
  if (VALID_ERROR_MITIGATIONS.find(noise_mitigation) == VALID_ERROR_MITIGATIONS.end()) {
      std::stringstream ss;
      ss << "QB SDK: valid settings for error mitigation: " << std::endl;
      for (auto it : VALID_ERROR_MITIGATIONS) {
          ss << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(ss.str());
  }
}
const VectorString & session::get_noise_mitigations() const { return session::error_mitigations_; }
//
const std::string session::get_summary() const {
  std::ostringstream out;
  out << "* sn:" << std::endl <<
  "    Number of shots" << std::endl <<
  "  = ";
  for (auto item : session::get_sns()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* rn:" << std::endl <<
  "    Number of experiments/repetitions" << std::endl <<
  "  = ";
  for (auto item : get_rns()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* qn:" << std::endl <<
  "    Number of qubits" << std::endl <<
  "  = ";
  for (auto item : get_qns()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* random:" << std::endl <<
  "    Depth of randomly generated quantum circuit" << std::endl <<
  "  = ";
  for (auto item : get_randoms()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* infile:" << std::endl <<
  "    Filename containing quantum circuit" << std::endl <<
  "  = ";
  for (auto item : get_infiles()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* include_qb:" << std::endl <<
  "    Filename containing custom Quantum Brilliance gate definitions" << std::endl <<
  "  = ";
  for (auto item : get_include_qbs()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* remote_backend_database_path:" << std::endl <<
  "    Filename for YAML file with configuration data for remote backends (including hardware)" << std::endl <<
  "  = ";
  out << get_remote_backend_database_path() << std::endl << std::endl;
  //

  out << "* instring:" << std::endl <<
  "    String containing quantum circuit" << std::endl <<
  "  = ";
  for (auto item : get_instrings()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* irtarget_m:" << std::endl <<
  "    XACC IR (binary) circuit (C++ only)" << std::endl << std::endl;
  //

  out << "* xasm:" << std::endl <<
  "    Interpret circuit in XASM format" << std::endl <<
  "  = ";
  for (auto item : get_xasms()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* quil1:" << std::endl <<
  "    Interpret circuit in Quil 1.0 format" << std::endl <<
  "  = ";
  for (auto item : get_quil1s()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* acc:" << std::endl <<
  "    Back-end simulator" << std::endl <<
  "  = ";
  for (auto item : get_accs()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* noplacement:" << std::endl <<
  "    Disable the circuit placement step" << std::endl <<
  "  = ";
  for (auto item : get_noplacements()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* nooptimise:" << std::endl <<
  "    Disable the circuit optimiser step" << std::endl <<
  "  = ";
  for (auto item : get_nooptimises()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* nosim:" << std::endl <<
  "    Disable the circuit simulation step" << std::endl <<
  "  = ";
  for (auto item : get_nosims()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* noise:" << std::endl <<
  "    Enable noise modelling" << std::endl <<
  "  = ";
  for (auto item : get_noises()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* notiming:" << std::endl <<
  "    Disable timing data collection" << std::endl <<
  "  = ";
  for (auto item : get_notimings()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* output_oqm_enabled:" << std::endl <<
  "    Enable output of transpiled circuit" << std::endl <<
  "  = ";
  for (auto item : get_output_oqm_enableds()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* log_enabled:" << std::endl <<
  "    Enable log file output" << std::endl <<
  "  = ";
  for (auto item : get_log_enableds()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* beta:" << std::endl <<
  "    Parameters for quantum circuit" << std::endl <<
  "  = ";
  for (auto item : get_betas()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* theta:" << std::endl <<
  "    Hyperparameters for algorithms" << std::endl <<
  "  = ";
  for (auto item : get_thetas()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* initial_bond_dimension:" << std::endl <<
  "    Tensor network initial bond dimension" << std::endl <<
  "  = ";
  for (auto item : get_initial_bond_dimensions()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* initial_kraus_dimension:" << std::endl <<
  "    Purification initial kraus dimension" << std::endl <<
  "  = ";
  for (auto item : get_initial_kraus_dimensions()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* max_bond_dimension:" << std::endl <<
  "    Tensor network maximum bond dimension" << std::endl <<
  "  = ";
  for (auto item : get_max_bond_dimensions()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* max_kraus_dimension:" << std::endl <<
  "    Purification maximum kraus dimension" << std::endl <<
  "  = ";
  for (auto item : get_max_kraus_dimensions()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* svd_cutoff:" << std::endl <<
  "    Tensor network SVD cutoff" << std::endl <<
  "  = ";
  for (auto item : get_svd_cutoffs()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* rel_svd_cutoff:" << std::endl <<
  "    Tensor network relative SVD cutoff" << std::endl <<
  "  = ";
  for (auto item : get_rel_svd_cutoffs()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_raw:" << std::endl <<
  "    JSON string of measured counts" << std::endl <<
  "  = ";
  for (auto item : get_out_raws()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* out_bitstring:" << std::endl <<
  "    Measured bitstrings" << std::endl <<
  "      [string] Keys: bitstrings" << std::endl <<
  "  = ";
  for (auto item : get_out_bitstrings()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_z_op_expect:" << std::endl <<
  "    Z-operator expectation from shot counts observed" << std::endl <<
  "      [integer] Keys:" << std::endl <<
  "        0: Z-operator expectation (from shots)" << std::endl <<
  "  = ";
  for (auto item : get_out_z_op_expects()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_divergence:" << std::endl <<
  "    Calculated divergence of measured counts from the theoretical distribution" << std::endl <<
  "      [integer] Keys:" << std::endl <<
  "        0: Jensen-Shannon" << std::endl <<
  "  = ";
  for (auto item : get_out_divergences()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_transpiled_circuit:" << std::endl <<
  "    OpenQASM string containing transpiled circuit" << std::endl <<
  "  = ";
  for (auto item : get_out_transpiled_circuits()) {
      out << std::endl << "  =" << std::endl;
      for (auto itel : item) {
        out << itel;
        out << std::endl << "  -" << std::endl;
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_qobj:" << std::endl <<
  "    Aer qobj JSON input" << std::endl <<
  "  = ";
  for (auto item : get_out_qobjs()) {
      out << std::endl << "  =" << std::endl;
      for (auto itel : item) {
        out << itel;
        out << std::endl << "  " << std::endl;
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_qbjson:" << std::endl <<
  "    QB hardware JSON POST payload" << std::endl <<
  "  = ";
  for (auto item : get_out_qbjsons()) {
      out << std::endl << "  =" << std::endl;
      for (auto itel : item) {
        out << itel;
        out << std::endl << "  " << std::endl;
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_single_qubit_gate_qty:" << std::endl <<
  "    Count of single-qubit gates applied to qubit[qubit-index]" << std::endl <<
  "      [integer] Keys: qubit-index" << std::endl <<
  "  = ";
  for (auto item : get_out_single_qubit_gate_qtys()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_double_qubit_gate_qty:" << std::endl <<
  "    Count of two-qubit gates applied to qubit[qubit-index]" << std::endl <<
  "      [integer] Keys: qubit-index" << std::endl <<
  "  = ";
  for (auto item : get_out_double_qubit_gate_qtys()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* out_total_init_maxgate_readout_time:" << std::endl <<
  "    Time taken for the required number of shots [sn]"<< std::endl <<
  "      [integer] Keys:" << std::endl <<
  "        0: Total time, (estimated) in ms" << std::endl <<
  "        1: Initialisation time component, (estimated) in ms" << std::endl <<
  "        2: Gate (max. depth) time component, (estimated) in ms" << std::endl <<
  "        3: Readout time component, (estimated) in ms" << std::endl <<
  "        4: Total time (from classical simulation), in ms" << std::endl <<
  "        5: PC transfer to controller time, in ms" << std::endl <<

  "  = ";
  for (auto item : get_out_total_init_maxgate_readout_times()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (auto it : itel) {
              out << " | " << it.first << ": " << it.second;
          }
          if (itel.size() > 0) {
              out << " | ";
          } else {
              out << " NA ";
          }
      }
  }
  out << std::endl << std::endl;
  //

  out << "* debug:" << std::endl <<
  "    Switch to debug mode" << std::endl <<
  "  = ";
  out << get_debug();
  out << std::endl << std::endl;
  //
  return out.str();
}
} // namespace qb
