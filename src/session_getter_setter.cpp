// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qristal/core/session.hpp"

namespace qristal {
//
// Getter/Setter methods for qristal::session
//
void session::set_infile(const std::string &infile) {
  session::infiles_.clear();
  session::infiles_.push_back({infile});
}
void session::set_infiles(const Table2d<std::string> &infiles) { session::infiles_ = infiles; }
const Table2d<std::string> & session::get_infiles() const { return session::infiles_; }
//
void session::set_instring(const std::string &instring) {
  session::instrings_.clear();
  session::instrings_.push_back({instring});
}
void session::set_instrings(const Table2d<std::string> &instrings) { session::instrings_ = instrings; }
const Table2d<std::string> & session::get_instrings() const { return session::instrings_; }
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
void session::set_include_qbs(const Table2d<std::string> &include_qbs) { session::include_qbs_ = include_qbs; }
const Table2d<std::string> & session::get_include_qbs() const { return session::include_qbs_; }
//
void session::set_parameter_vector(const std::vector<double> &vals) {
  session::parameter_vectors_.clear();
  session::parameter_vectors_.push_back({vals});
}
void session::set_parameter_vectors(Table2d<std::vector<double>> vals) {
  session::parameter_vectors_ = vals;
}
const Table2d<std::vector<double>> & session::get_parameter_vectors() const {
  return session::parameter_vectors_;
}
//
void session::set_calc_jacobian(bool calc_jacobian) {
  session::calc_jacobians_.clear();
  session::calc_jacobians_.push_back({calc_jacobian});
  if (calc_jacobian) set_calc_out_counts(true);
}
void session::set_calc_jacobians(Table2d<bool> calc_jacobians) {
  session::calc_jacobians_ = calc_jacobians;
}
const Table2d<bool> & session::get_calc_jacobians() const { return session::calc_jacobians_; }
//
void session::set_calc_out_counts(bool calc_out_counts) {
  if (calc_jacobians_[0][0] and not calc_out_counts) throw std::logic_error("You cannot set calc_out_counts false whilst calc_jacobian is true.");
  session::calc_out_counts_.clear();
  session::calc_out_counts_.push_back({calc_out_counts});
}
void session::set_calc_out_countss(Table2d<bool> calc_out_counts) {
session::calc_out_counts_ = calc_out_counts;
}
const Table2d<bool> & session::get_calc_out_counts() const { return session::calc_out_counts_; }
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
void session::set_accs(const Table2d<std::string> &accs) {
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
      listaccs << "Qristal: valid settings for acc: " << std::endl;
      for (auto it : VALID_ACCS) {
          listaccs << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listaccs.str());
  }
}
const Table2d<std::string> & session::get_accs() const { return session::accs_; }
//
void session::set_aer_sim_type(const std::string &sim_type) {
  validate_aer_sim_type(sim_type);
  aer_sim_types_.clear();
  aer_sim_types_.emplace_back(std::vector<std::string>{sim_type});
}
void session::set_aer_sim_types(const Table2d<std::string> &sim_types) {
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
    aer_sim_type_error_msg << "Qristal: valid settings for aer_sim_type: " << std::endl;
    for (auto it : VALID_AER_SIM_TYPES) {
      aer_sim_type_error_msg << "* \"" << it << "\"" << std::endl;
    }
    throw std::range_error(aer_sim_type_error_msg.str());
  }
}
const Table2d<std::string> &session::get_aer_sim_types() const { return aer_sim_types_; }

//

void session::set_random(const size_t &in_random) {
  session::randoms_.clear();
  session::randoms_.push_back({in_random});
}
void session::set_randoms(const Table2d<size_t> &in_random) { session::randoms_ = in_random; }
const Table2d<size_t> & session::get_randoms() const { return session::randoms_; }
//
void session::set_xasm(const bool &in_xasm) {
  session::xasms_.clear();
  session::xasms_.push_back({in_xasm});
}
void session::set_xasms(const Table2d<bool> &in_xasm) { session::xasms_ = in_xasm; }
const Table2d<bool> & session::get_xasms() const { return session::xasms_; }
//
void session::set_quil1(const bool &in_quil1) {
  session::quil1s_.clear();
  session::quil1s_.push_back({in_quil1});
}
void session::set_quil1s(const Table2d<bool> &in_quil1) { session::quil1s_ = in_quil1; }
const Table2d<bool> & session::get_quil1s() const { return session::quil1s_; }
//
void session::set_noplacement(const bool &in_noplacement) {
  session::noplacements_.clear();
  session::noplacements_.push_back({in_noplacement});
}
void session::set_noplacements(const Table2d<bool> &in_noplacement) {
  session::noplacements_ = in_noplacement;
}
const Table2d<bool> & session::get_noplacements() const { return session::noplacements_; }
//
void session::set_placement(const std::string &in_placement) {
  placements_.clear();
  placements_.push_back({in_placement});
}
void session::set_placements(const Table2d<std::string> &in_placements) {
  placements_ = in_placements;
}
const Table2d<std::string> &session::get_placements() const { return placements_; }
//
void session::set_nooptimise(const bool &in_nooptimise) {
  session::nooptimises_.clear();
  session::nooptimises_.push_back({in_nooptimise});
}
void session::set_nooptimises(const Table2d<bool> &in_nooptimise) {
  session::nooptimises_ = in_nooptimise;
}
const Table2d<bool> & session::get_nooptimises() const { return session::nooptimises_; }

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
void session::set_nosims(const Table2d<bool> &in_nosim) { session::nosims_ = in_nosim; }
const Table2d<bool> & session::get_nosims() const { return session::nosims_; }
//
void session::set_noise(const bool &in_noise) {
  session::noises_.clear();
  session::noises_.push_back({in_noise});
}



void session::set_noises(const Table2d<bool> &in_noise) { session::noises_ = in_noise; }
const Table2d<bool> & session::get_noises() const { return session::noises_; }
//

void session::set_output_oqm_enabled(const bool &in_output_oqm_enabled) {
  session::output_oqm_enableds_.clear();
  session::output_oqm_enableds_.push_back({in_output_oqm_enabled});
}
void session::set_output_oqm_enableds(const Table2d<bool> &in_output_oqm_enabled) {
  session::output_oqm_enableds_ = in_output_oqm_enabled;
}
const Table2d<bool> & session::get_output_oqm_enableds() const {
  return session::output_oqm_enableds_;
}
//
void session::set_notiming(const bool &in_notiming) {
  session::notimings_.clear();
  session::notimings_.push_back({in_notiming});
}
void session::set_notimings(const Table2d<bool> &in_notiming) { session::notimings_ = in_notiming; }
const Table2d<bool> & session::get_notimings() const { return session::notimings_; }
//
void session::set_qn(const size_t &in_qn) {
  session::qns_.clear();
  session::qns_.push_back({in_qn});
}
void session::set_qns(const Table2d<size_t> &in_qn) { session::qns_ = in_qn; }
const Table2d<size_t> & session::get_qns() const { return session::qns_; }
//
void session::set_sn(const size_t &in_sn) {
  session::sns_.clear();
  session::sns_.push_back({in_sn});
}
void session::set_sns(const Table2d<size_t> &in_sn) { session::sns_ = in_sn; }
const Table2d<size_t> & session::get_sns() const { return session::sns_; }
//
void session::set_initial_bond_dimension(const size_t &in_initial_bond_dimension) {
  session::initial_bond_dimensions_.clear();
  session::initial_bond_dimensions_.push_back({in_initial_bond_dimension});
}
void session::set_initial_bond_dimensions(const Table2d<size_t> &in_initial_bond_dimension) { session::initial_bond_dimensions_ = in_initial_bond_dimension; }
const Table2d<size_t> & session::get_initial_bond_dimensions() const { return session::initial_bond_dimensions_; }
//
void session::set_initial_kraus_dimension(const size_t &in_initial_kraus_dimension) {
  session::initial_kraus_dimensions_.clear();
  session::initial_kraus_dimensions_.push_back({in_initial_kraus_dimension});
}
void session::set_initial_kraus_dimensions(const Table2d<size_t> &in_initial_kraus_dimension) { session::initial_kraus_dimensions_ = in_initial_kraus_dimension; }
const Table2d<size_t> & session::get_initial_kraus_dimensions() const { return session::initial_kraus_dimensions_; }
//
void session::set_max_bond_dimension(const size_t &in_max_bond_dimension) {
  session::max_bond_dimensions_.clear();
  session::max_bond_dimensions_.push_back({in_max_bond_dimension});
}
void session::set_max_bond_dimensions(const Table2d<size_t> &in_max_bond_dimension) { session::max_bond_dimensions_ = in_max_bond_dimension; }
const Table2d<size_t> & session::get_max_bond_dimensions() const { return session::max_bond_dimensions_; }
//
void session::set_max_kraus_dimension(const size_t &in_max_kraus_dimension) {
  session::max_kraus_dimensions_.clear();
  session::max_kraus_dimensions_.push_back({in_max_kraus_dimension});
}
void session::set_max_kraus_dimensions(const Table2d<size_t> &in_max_kraus_dimension) { session::max_kraus_dimensions_ = in_max_kraus_dimension; }
const Table2d<size_t> & session::get_max_kraus_dimensions() const { return session::max_kraus_dimensions_; }
//
void session::set_svd_cutoff(const std::map<int,double> &in_svd_cutoff) {
  session::svd_cutoffs_.clear();
  session::svd_cutoffs_.push_back({in_svd_cutoff});
}
void session::set_svd_cutoffs(const Table2d<std::map<int,double>> &in_svd_cutoff) { session::svd_cutoffs_ = in_svd_cutoff; }
const Table2d<std::map<int,double>> & session::get_svd_cutoffs() const { return session::svd_cutoffs_; }
//
void session::set_rel_svd_cutoff(const std::map<int,double> &in_rel_svd_cutoff) {
  session::rel_svd_cutoffs_.clear();
  session::rel_svd_cutoffs_.push_back({in_rel_svd_cutoff});
}
void session::set_rel_svd_cutoffs(const Table2d<std::map<int,double>> &in_rel_svd_cutoff) { session::rel_svd_cutoffs_ = in_rel_svd_cutoff; }
const Table2d<std::map<int,double>> & session::get_rel_svd_cutoffs() const { return session::rel_svd_cutoffs_; }
//
void session::set_measure_sample_sequential(const std::string &in_measure_sample_sequential) {
  validate_measure_sample_options(in_measure_sample_sequential);
  session::measure_sample_sequentials_.clear();
  session::measure_sample_sequentials_.push_back({in_measure_sample_sequential});
}
void session::set_measure_sample_sequentials(const Table2d<std::string> &in_measure_sample_sequential) {
  for (auto item : in_measure_sample_sequential) {
    for (auto im : item) {
      session::validate_measure_sample_options(im);
    }
  }
  session::measure_sample_sequentials_ = in_measure_sample_sequential;
}
void session::validate_measure_sample_options(const std::string &measure_sample_options) {
  if (VALID_MEASURE_SAMPLING_OPTIONS.find(measure_sample_options) == VALID_MEASURE_SAMPLING_OPTIONS.end()) {
    std::stringstream measure_sample_options_error_message;
    measure_sample_options_error_message << "Valid measure sampling options: " << std::endl;
    for (auto it : VALID_MEASURE_SAMPLING_OPTIONS) {
      measure_sample_options_error_message << "* \"" << it << "\"" << std::endl;
    }
    throw std::range_error(measure_sample_options_error_message.str());
  }
}
const Table2d<std::string> & session::get_measure_sample_sequentials() const { return session::measure_sample_sequentials_; }

//
void session::set_noise_model(NoiseModel &noise_model) {
  noise_models_.clear();
  noise_models_.emplace_back(std::vector<NoiseModel*>{&noise_model});
}
void session::set_noise_models(const std::vector<std::vector<NoiseModel*>> &noise_models) {
  noise_models_ = noise_models;
}
const std::vector<std::vector<NoiseModel*>> &session::get_noise_models() const { return noise_models_; }
//
void session::set_expected_amplitudes(const std::map<
    std::vector<bool>, std::complex<double>> &amp) {
  session::expected_amplitudes_.clear();
  session::expected_amplitudes_.push_back({amp});
}
void session::set_expected_amplitudess(const std::vector<std::vector<std::map<
    std::vector<bool>, std::complex<double>>>> &amp) {
  session::expected_amplitudes_ = amp;
}
const std::vector<std::vector<std::map<std::vector<bool>, std::complex<double>>>> &
    session::get_expected_amplitudes() const { return session::expected_amplitudes_; }
//
void session::set_debug(const bool & debug) {
  session::debug_ = debug;
}
const bool & session::get_debug() const { return session::debug_; }
//
void session::setName(const Table2d<std::string> &name_) { session::name_m = name_; }
void session::setName(const std::string &name_) { session::name_m.push_back({name_}); }
const Table2d<std::string> & session::getName() const { return session::name_m; }

const Table2d<std::map<std::vector<bool>,int>>& session::results() const { return session::results_ ; }
const Table2d<std::map<std::vector<bool>,int>> & session::results_native() const { return session::results_native_; }
//
const Table2d<std::vector<int>> & session::get_out_counts() const {
  //TODO after removing i,j functionality: add a check that calc_out_counts is true
  return session::out_counts_;
}
//
const Table2d<std::vector<double>> & session::get_out_probs() const {
  //TODO after removing i,j functionality: add a check that calc_jacobian is true
  return session::out_probs_;
}
//
const Table2d<Table2d<double>> & session::get_out_prob_jacobians() const {
  //TODO after removing i,j functionality: add a check that calc_jacobian is true
  return session::out_prob_gradients_;
}
//
const Table2d<std::map<int,double>> & session::get_out_divergences() const { return session::out_divergences_ ; }
//

const Table2d<std::string> & session::get_out_transpiled_circuits() const { return session::out_transpiled_circuits_ ; }
//

const Table2d<std::string> & session::get_out_qobjs() const { return session::out_qobjs_ ; }
//

const Table2d<std::string> & session::get_out_qbjsons() const { return session::out_qbjsons_ ; }
//

const Table2d<std::map<int,int>> & session::get_out_single_qubit_gate_qtys() const { return session::out_single_qubit_gate_qtys_ ; }
//

const Table2d<std::map<int,int>> & session::get_out_double_qubit_gate_qtys() const { return session::out_double_qubit_gate_qtys_ ; }
//

const Table2d<std::map<int,double>> & session::get_out_total_init_maxgate_readout_times() const { return session::out_total_init_maxgate_readout_times_ ; }
//

const Table2d<std::map<int,double>> & session::get_out_z_op_expects() const { return session::out_z_op_expects_ ; }

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
void session::set_seeds(const Table2d<size_t> &in_seeds) { session::seeds_ = in_seeds; }
const Table2d<size_t> & session::get_seeds() const { return session::seeds_; }
//
void session::set_noise_mitigations(const Table2d<std::string> &noise_mitigations) {
  for (const auto& item : noise_mitigations) {
    for (const auto& im : item) {
      session::validate_noise_mitigation(im);
    }
  }
  session::error_mitigations_ = noise_mitigations;
}

void session::set_SPAM_correction_matrix(const Eigen::MatrixXd& mat)
{
  size_t dim = std::pow(2, get_qns()[0][0]);
  assert(mat.rows() == dim && mat.cols() == dim && "Mismatching dimensions of SPAM correction matrix and numbers of qubits!");
  perform_SPAM_correction_ = true; 
  SPAM_correction_mat_ = mat;
}
const Eigen::MatrixXd& session::get_SPAM_correction_matrix() const {
  return SPAM_correction_mat_;
}
void session::set_SPAM_confusion_matrix(const Eigen::MatrixXd& mat)
{
  set_SPAM_correction_matrix(mat.inverse());
}

void session::validate_noise_mitigation(const std::string &noise_mitigation) {
  if (VALID_ERROR_MITIGATIONS.find(noise_mitigation) == VALID_ERROR_MITIGATIONS.end()) {
      std::stringstream ss;
      ss << "Qristal: valid settings for error mitigation: " << std::endl;
      for (auto it : VALID_ERROR_MITIGATIONS) {
          ss << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(ss.str());
  }
}
const Table2d<std::string> & session::get_noise_mitigations() const { return session::error_mitigations_; }
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

  out << "* measure_sample_sequential:" << std::endl <<
  "    QB tensor network measurement sampling method" << std::endl <<
  "  = ";
  for (auto item : get_measure_sample_sequentials()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* out_counts:" << std::endl <<
  "    Measured counts" << std::endl <<
  "      [int] Bitstring indices" << std::endl <<
  "  = ";
  for (auto item : get_out_counts()) {
      out << std::endl << " ";
      for (auto itel : item) {
          for (size_t i = 0; auto it : itel) {
              out << " | " << i << ": " << it;
              i++;
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
}
