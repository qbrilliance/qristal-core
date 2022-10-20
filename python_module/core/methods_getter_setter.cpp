// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/methods.hpp"

namespace qbOS {
//
// Getter/Setter methods for qbOS::Qbqe
//
void Qbqe::set_infile(const std::string &infile) {
  Qbqe::infiles_.clear();
  Qbqe::infiles_.push_back({infile});
}
void Qbqe::set_infiles(const VectorString &infiles) { Qbqe::infiles_ = infiles; }
const VectorString & Qbqe::get_infiles() const { return Qbqe::infiles_; }
//
void Qbqe::set_instring(const std::string &instring) {
  Qbqe::instrings_.clear();
  Qbqe::instrings_.push_back({instring});
}
void Qbqe::set_instrings(const VectorString &instrings) { Qbqe::instrings_ = instrings; }
const VectorString & Qbqe::get_instrings() const { return Qbqe::instrings_; }
//
void Qbqe::set_irtarget_m(const std::shared_ptr<xacc::CompositeInstruction> &irtarget_m) {
    Qbqe::irtarget_ms_.clear();
    Qbqe::irtarget_ms_.push_back({irtarget_m});
}
void Qbqe::set_irtarget_ms(const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &irtarget_ms) {
    Qbqe::irtarget_ms_ = irtarget_ms;
}
const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> & Qbqe::get_irtarget_ms() const {
    return Qbqe::irtarget_ms_;
}

//
void Qbqe::set_include_qb(const std::string &include_qb) {
  Qbqe::include_qbs_.clear();
  Qbqe::include_qbs_.push_back({include_qb});
}
void Qbqe::set_include_qbs(const VectorString &include_qbs) { Qbqe::include_qbs_ = include_qbs; }
const VectorString & Qbqe::get_include_qbs() const { return Qbqe::include_qbs_; }
//
void Qbqe::set_qpu_config(const std::string &qpu_config) {
  Qbqe::qpu_configs_.clear();
  Qbqe::qpu_configs_.push_back({qpu_config});
}
void Qbqe::set_qpu_configs(const VectorString &qpu_configs) { Qbqe::qpu_configs_ = qpu_configs; }
const VectorString & Qbqe::get_qpu_configs() const { return Qbqe::qpu_configs_; }
//
void Qbqe::set_acc(const std::string &acc) {
  Qbqe::validate_acc(acc);
  Qbqe::accs_.clear();
  Qbqe::accs_.push_back({acc});
}
void Qbqe::set_accs(const VectorString &accs) {
  for (auto item : accs) {
    for (auto im : item) {
      Qbqe::validate_acc(im);
    }
  }
  Qbqe::accs_ = accs;
}

void Qbqe::validate_acc(const std::string &acc) {
  if (VALID_ACCS.find(acc) == VALID_ACCS.end()) {
      std::stringstream listaccs;
      listaccs << "qbOS: valid settings for acc: " << std::endl;
      for (auto it : VALID_ACCS) {
          listaccs << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listaccs.str());
  }
}
const VectorString & Qbqe::get_accs() const { return Qbqe::accs_; }
//
void Qbqe::set_aws_device_name(const std::string &device_name) {
  Qbqe::validate_aws_device_name(device_name);
  Qbqe::aws_device_names_.clear();
  Qbqe::aws_device_names_.push_back({device_name});
}
//
void Qbqe::set_aws_device_names(const VectorString &device_names) {
  for (auto item : device_names) {
    for (auto im : item) {
      Qbqe::validate_aws_device_name(im);
    }
  }
  Qbqe::aws_device_names_ = device_names;
}
//
void Qbqe::validate_aws_device_name(const std::string &device_name) {
  if (VALID_AWS_DEVICES.find(device_name) == VALID_AWS_DEVICES.end()) {
      std::stringstream listawsds;
      listawsds << "qbOS: valid settings for aws_device: " << std::endl;
      for (auto it : VALID_AWS_DEVICES) {
          listawsds << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listawsds.str());
  }
}
//
const VectorString & Qbqe::get_aws_device_names() const { return Qbqe::aws_device_names_; }

//
void Qbqe::set_aws_s3(const std::string &bucket_name) {
  Qbqe::validate_aws_s3(bucket_name);
  Qbqe::aws_s3s_.clear();
  Qbqe::aws_s3s_.push_back({bucket_name});
}

//
void Qbqe::set_aws_s3s(const VectorString &bucket_names) {
  for (auto item : bucket_names) {
    for (auto im : item) {
      Qbqe::validate_aws_s3(im);
    }
  }
  Qbqe::aws_s3s_ = bucket_names;
}

//
void Qbqe::validate_aws_s3(const std::string &bucket_name) {
  bool prefix_is_valid = false;
  for (auto epf : VALID_AWS_S3_PREFIXS) {
      auto res = std::mismatch(epf.begin(), epf.end(), bucket_name.begin());
      if (res.first == epf.end()) {
          prefix_is_valid = true;
      }
  }
  if (!prefix_is_valid) {
      std::stringstream listawspf;
      listawspf << "qbOS: valid prefix strings for aws_s3: " << std::endl;
      for (auto it : VALID_AWS_S3_PREFIXS) {
          listawspf << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listawspf.str());
  }
}

const VectorString & Qbqe::get_aws_s3s() const { return Qbqe::aws_s3s_; }

//
void Qbqe::set_aws_s3_path(const std::string &path) {
  Qbqe::aws_s3_paths_.clear();
  Qbqe::aws_s3_paths_.push_back({path});
}

//
void Qbqe::set_aws_s3_paths(const VectorString &paths) {
  Qbqe::aws_s3_paths_ = paths;
}

const VectorString & Qbqe::get_aws_s3_paths() const { return Qbqe::aws_s3_paths_; }

//
void Qbqe::set_aws_format(const std::string &format) {
  Qbqe::validate_aws_format(format);
  Qbqe::aws_formats_.clear();
  Qbqe::aws_formats_.push_back({format});
}

//
void Qbqe::set_aws_formats(const VectorString &formats) {
  for (auto item : formats) {
    for (auto im : item) {
      Qbqe::validate_aws_format(im);
    }
  }
  Qbqe::aws_formats_ = formats;
}

void Qbqe::validate_aws_format(const std::string &format) {
  if (VALID_AWS_FORMATS.find(format) == VALID_AWS_FORMATS.end()) {
      std::stringstream listawsds;
      listawsds << "qbOS: valid settings for aws_format: " << std::endl;
      for (auto it : VALID_AWS_FORMATS) {
          listawsds << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(listawsds.str());
  }
}

const VectorString & Qbqe::get_aws_formats() const { return Qbqe::aws_formats_; }


void Qbqe::set_aws_verbatim(const bool &verbatim) {
  //Qbqe::validate_aws_verbatim(verbatim);
  Qbqe::aws_verbatims_.clear();
  Qbqe::aws_verbatims_.push_back({verbatim});
}

void Qbqe::set_aws_verbatims(const VectorBool &verbatims) { Qbqe::aws_verbatims_ = verbatims; }
const VectorBool & Qbqe::get_aws_verbatims() const { return Qbqe::aws_verbatims_; }

//

void Qbqe::set_aer_sim_type(const std::string &sim_type) {
  validate_aer_sim_type(sim_type);
  aer_sim_types_.clear();
  aer_sim_types_.emplace_back(std::vector<std::string>{sim_type});
}
void Qbqe::set_aer_sim_types(const VectorString &sim_types) {
  for (const auto &item : sim_types) {
    for (const auto &im : item) {
      validate_aer_sim_type(im);
    }
  }
  aer_sim_types_ = sim_types;
}
void Qbqe::validate_aer_sim_type(const std::string &sim_type) {
  if (VALID_AER_SIM_TYPES.find(sim_type) == VALID_AER_SIM_TYPES.end()) {
    std::stringstream aer_sim_type_error_msg;
    aer_sim_type_error_msg << "qbOS: valid settings for aer_sim_type: " << std::endl;
    for (auto it : VALID_AER_SIM_TYPES) {
      aer_sim_type_error_msg << "* \"" << it << "\"" << std::endl;
    }
    throw std::range_error(aer_sim_type_error_msg.str());
  }
}
const VectorString &Qbqe::get_aer_sim_types() const { return aer_sim_types_; }

//

void Qbqe::set_random(const size_t &in_random) {
  Qbqe::randoms_.clear();
  Qbqe::randoms_.push_back({in_random});
}
void Qbqe::set_randoms(const VectorN &in_random) { Qbqe::randoms_ = in_random; }
const VectorN & Qbqe::get_randoms() const { return Qbqe::randoms_; }
//
void Qbqe::set_xasm(const bool &in_xasm) {
  Qbqe::xasms_.clear();
  Qbqe::xasms_.push_back({in_xasm});
}
void Qbqe::set_xasms(const VectorBool &in_xasm) { Qbqe::xasms_ = in_xasm; }
const VectorBool & Qbqe::get_xasms() const { return Qbqe::xasms_; }
//
void Qbqe::set_quil1(const bool &in_quil1) {
  Qbqe::quil1s_.clear();
  Qbqe::quil1s_.push_back({in_quil1});
}
void Qbqe::set_quil1s(const VectorBool &in_quil1) { Qbqe::quil1s_ = in_quil1; }
const VectorBool & Qbqe::get_quil1s() const { return Qbqe::quil1s_; }
//
void Qbqe::set_noplacement(const bool &in_noplacement) {
  Qbqe::noplacements_.clear();
  Qbqe::noplacements_.push_back({in_noplacement});
}
void Qbqe::set_noplacements(const VectorBool &in_noplacement) {
  Qbqe::noplacements_ = in_noplacement;
}
const VectorBool & Qbqe::get_noplacements() const { return Qbqe::noplacements_; }
//
void Qbqe::set_placement(const std::string &in_placement) {
  placements_.clear();
  placements_.push_back({in_placement});
}
void Qbqe::set_placements(const VectorString &in_placements) {
  placements_ = in_placements;
}
const VectorString &Qbqe::get_placements() const { return placements_; }
//
void Qbqe::set_nooptimise(const bool &in_nooptimise) {
  Qbqe::nooptimises_.clear();
  Qbqe::nooptimises_.push_back({in_nooptimise});
}
void Qbqe::set_nooptimises(const VectorBool &in_nooptimise) {
  Qbqe::nooptimises_ = in_nooptimise;
}
const VectorBool & Qbqe::get_nooptimises() const { return Qbqe::nooptimises_; }
//
void Qbqe::set_nosim(const bool &in_nosim) {
  Qbqe::nosims_.clear();
  Qbqe::nosims_.push_back({in_nosim});
}
void Qbqe::set_nosims(const VectorBool &in_nosim) { Qbqe::nosims_ = in_nosim; }
const VectorBool & Qbqe::get_nosims() const { return Qbqe::nosims_; }
//
void Qbqe::set_noise(const bool &in_noise) {
  Qbqe::noises_.clear();
  Qbqe::noises_.push_back({in_noise});
}



void Qbqe::set_noises(const VectorBool &in_noise) { Qbqe::noises_ = in_noise; }
const VectorBool & Qbqe::get_noises() const { return Qbqe::noises_; }
//

void Qbqe::set_output_oqm_enabled(const bool &in_output_oqm_enabled) {
  Qbqe::output_oqm_enableds_.clear();
  Qbqe::output_oqm_enableds_.push_back({in_output_oqm_enabled});
}
void Qbqe::set_output_oqm_enableds(const VectorBool &in_output_oqm_enabled) {
  Qbqe::output_oqm_enableds_ = in_output_oqm_enabled;
}
const VectorBool & Qbqe::get_output_oqm_enableds() const {
  return Qbqe::output_oqm_enableds_;
}
//
void Qbqe::set_log_enabled(const bool &in_log_enabled) {
  Qbqe::log_enableds_.clear();
  Qbqe::log_enableds_.push_back({in_log_enabled});
}
void Qbqe::set_log_enableds(const VectorBool &in_log_enabled) {
  Qbqe::log_enableds_ = in_log_enabled;
}
const VectorBool & Qbqe::get_log_enableds() const { return Qbqe::log_enableds_; }
//
void Qbqe::set_notiming(const bool &in_notiming) {
  Qbqe::notimings_.clear();
  Qbqe::notimings_.push_back({in_notiming});
}
void Qbqe::set_notimings(const VectorBool &in_notiming) { Qbqe::notimings_ = in_notiming; }
const VectorBool & Qbqe::get_notimings() const { return Qbqe::notimings_; }
//
void Qbqe::set_qn(const size_t &in_qn) {
  Qbqe::qns_.clear();
  Qbqe::qns_.push_back({in_qn});
}
void Qbqe::set_qns(const VectorN &in_qn) { Qbqe::qns_ = in_qn; }
const VectorN & Qbqe::get_qns() const { return Qbqe::qns_; }
//
void Qbqe::set_rn(const size_t &in_rn) {
  Qbqe::rns_.clear();
  Qbqe::rns_.push_back({in_rn});
}
void Qbqe::set_rns(const VectorN &in_rn) { Qbqe::rns_ = in_rn; }
const VectorN & Qbqe::get_rns() const { return Qbqe::rns_; }
//
void Qbqe::set_sn(const size_t &in_sn) {
  Qbqe::sns_.clear();
  Qbqe::sns_.push_back({in_sn});
}
void Qbqe::set_sns(const VectorN &in_sn) { Qbqe::sns_ = in_sn; }
const VectorN & Qbqe::get_sns() const { return Qbqe::sns_; }
//
void Qbqe::set_beta(const ND &in_beta) {
  Qbqe::betas_.clear();
  Qbqe::betas_.push_back({in_beta});
}
void Qbqe::set_betas(const VectorMapND &in_beta) { Qbqe::betas_ = in_beta; }
const VectorMapND & Qbqe::get_betas() const { return Qbqe::betas_; }
//
void Qbqe::set_theta(const ND &in_theta) {
  Qbqe::thetas_.clear();
  Qbqe::thetas_.push_back({in_theta});
}
void Qbqe::set_thetas(const VectorMapND &in_theta) { Qbqe::thetas_ = in_theta; }
const VectorMapND & Qbqe::get_thetas() const { return Qbqe::thetas_; }
//
void Qbqe::set_max_bond_dimension(const size_t &in_max_bond_dimension) {
  Qbqe::max_bond_dimensions_.clear();
  Qbqe::max_bond_dimensions_.push_back({in_max_bond_dimension});
}
void Qbqe::set_max_bond_dimensions(const VectorN &in_max_bond_dimension) { Qbqe::max_bond_dimensions_ = in_max_bond_dimension; }
const VectorN & Qbqe::get_max_bond_dimensions() const { return Qbqe::max_bond_dimensions_; }
//
void Qbqe::set_svd_cutoff(const ND &in_svd_cutoff) {
  Qbqe::svd_cutoffs_.clear();
  Qbqe::svd_cutoffs_.push_back({in_svd_cutoff});
}
void Qbqe::set_svd_cutoffs(const VectorMapND &in_svd_cutoff) { Qbqe::svd_cutoffs_ = in_svd_cutoff; }
const VectorMapND & Qbqe::get_svd_cutoffs() const { return Qbqe::svd_cutoffs_; }
//
void Qbqe::set_noise_model(const std::string &noise_model) {
  validate_noise_model(noise_model);
  noise_models_.clear();
  noise_models_.emplace_back(std::vector<std::string>{noise_model});
}
void Qbqe::set_noise_models(const VectorString &noise_models) {
  for (const auto &item : noise_models) {
    for (const auto &im : item) {
      validate_noise_model(im);
    }
  }
  noise_models_ = noise_models;
}
void Qbqe::validate_noise_model(const std::string &noise_model) {
  if (VALID_NOISE_MODEL_NAMES.find(noise_model) == VALID_NOISE_MODEL_NAMES.end()) {
    std::stringstream noise_model_error_msg;
    noise_model_error_msg << "qbOS: valid settings for noise_model: " << std::endl;
    for (auto it : VALID_NOISE_MODEL_NAMES) {
      noise_model_error_msg << "* \"" << it << "\"" << std::endl;
    }
    throw std::range_error(noise_model_error_msg.str());
  }
}
const VectorString &Qbqe::get_noise_models() const { return noise_models_; }
//
void Qbqe::set_output_amplitude(const NC &in_output_amplitude) {
  Qbqe::output_amplitudes_.clear();
  Qbqe::output_amplitudes_.push_back({in_output_amplitude});
}
void Qbqe::set_output_amplitudes(const VectorMapNC &in_output_amplitude) {
  Qbqe::output_amplitudes_ = in_output_amplitude;
}
const VectorMapNC & Qbqe::get_output_amplitudes() const { return Qbqe::output_amplitudes_; }
//
void Qbqe::set_debug_qbqe(const bool & debug_qbqe) {
  Qbqe::debug_qbqe_ = debug_qbqe;
}
const bool & Qbqe::get_debug_qbqe() const { return Qbqe::debug_qbqe_; }
//
void Qbqe::setName(const VectorString &name_) { Qbqe::name_m = name_; }
void Qbqe::setName(const std::string &name_) { Qbqe::name_m.push_back({name_}); }
const VectorString & Qbqe::getName() const { return Qbqe::name_m; }

void Qbqe::set_out_raw(const std::string & out_raw) {
  Qbqe::out_raws_.clear();
  Qbqe::out_raws_.push_back({out_raw});
}
void Qbqe::set_out_raws(const VectorString & out_raws) { Qbqe::out_raws_ = out_raws; }
const VectorString & Qbqe::get_out_raws() const { return Qbqe::out_raws_ ; }
//

void Qbqe::set_out_count(const NN & out_count) {
  Qbqe::out_counts_.clear();
  Qbqe::out_counts_.push_back({out_count});
}
void Qbqe::set_out_counts(const VectorMapNN & out_counts) { Qbqe::out_counts_ = out_counts; }
const VectorMapNN & Qbqe::get_out_counts() const { return Qbqe::out_counts_ ; }
//

void Qbqe::set_out_divergence(const ND & out_divergence) {
  Qbqe::out_divergences_.clear();
  Qbqe::out_divergences_.push_back({out_divergence});
}
void Qbqe::set_out_divergences(const VectorMapND & out_divergences) { Qbqe::out_divergences_ = out_divergences; }
const VectorMapND & Qbqe::get_out_divergences() const { return Qbqe::out_divergences_ ; }
//

void Qbqe::set_out_transpiled_circuit(const std::string & out_transpiled_circuit) {
  Qbqe::out_transpiled_circuits_.clear();
  Qbqe::out_transpiled_circuits_.push_back({out_transpiled_circuit});
}
void Qbqe::set_out_transpiled_circuits(const VectorString & out_transpiled_circuits) { Qbqe::out_transpiled_circuits_ = out_transpiled_circuits; }
const VectorString & Qbqe::get_out_transpiled_circuits() const { return Qbqe::out_transpiled_circuits_ ; }
//

void Qbqe::set_out_qobj(const std::string & out_qobj) {
  Qbqe::out_qobjs_.clear();
  Qbqe::out_qobjs_.push_back({out_qobj});
}
void Qbqe::set_out_qobjs(const VectorString & out_qobjs) { Qbqe::out_qobjs_ = out_qobjs; }
const VectorString & Qbqe::get_out_qobjs() const { return Qbqe::out_qobjs_ ; }
//

void Qbqe::set_out_qbjson(const std::string & out_qbjson) {
  Qbqe::out_qbjsons_.clear();
  Qbqe::out_qbjsons_.push_back({out_qbjson});
}
void Qbqe::set_out_qbjsons(const VectorString & out_qbjsons) { Qbqe::out_qbjsons_ = out_qbjsons; }
const VectorString & Qbqe::get_out_qbjsons() const { return Qbqe::out_qbjsons_ ; }
//

void Qbqe::set_out_single_qubit_gate_qty(const NN & out_single_qubit_gate_qty) {
  Qbqe::out_single_qubit_gate_qtys_.clear();
  Qbqe::out_single_qubit_gate_qtys_.push_back({out_single_qubit_gate_qty});
}
void Qbqe::set_out_single_qubit_gate_qtys(const VectorMapNN & out_single_qubit_gate_qtys) { Qbqe::out_single_qubit_gate_qtys_ = out_single_qubit_gate_qtys; }
const VectorMapNN & Qbqe::get_out_single_qubit_gate_qtys() const { return Qbqe::out_single_qubit_gate_qtys_ ; }
//

void Qbqe::set_out_double_qubit_gate_qty(const NN & out_double_qubit_gate_qty) {
  Qbqe::out_double_qubit_gate_qtys_.clear();
  Qbqe::out_double_qubit_gate_qtys_.push_back({out_double_qubit_gate_qty});
}
void Qbqe::set_out_double_qubit_gate_qtys(const VectorMapNN & out_double_qubit_gate_qtys) { Qbqe::out_double_qubit_gate_qtys_ = out_double_qubit_gate_qtys; }
const VectorMapNN & Qbqe::get_out_double_qubit_gate_qtys() const { return Qbqe::out_double_qubit_gate_qtys_ ; }
//

void Qbqe::set_out_total_init_maxgate_readout_time(const ND & out_total_init_maxgate_readout_time) {
  Qbqe::out_total_init_maxgate_readout_times_.clear();
  Qbqe::out_total_init_maxgate_readout_times_.push_back({out_total_init_maxgate_readout_time});
}
void Qbqe::set_out_total_init_maxgate_readout_times(const VectorMapND & out_total_init_maxgate_readout_times) { Qbqe::out_total_init_maxgate_readout_times_ = out_total_init_maxgate_readout_times; }
const VectorMapND & Qbqe::get_out_total_init_maxgate_readout_times() const { return Qbqe::out_total_init_maxgate_readout_times_ ; }
//

void Qbqe::set_out_z_op_expect(const ND & out_z_op_expect) {
  Qbqe::out_z_op_expects_.clear();
  Qbqe::out_z_op_expects_.push_back({out_z_op_expect});
}
void Qbqe::set_out_z_op_expects(const VectorMapND & out_z_op_expects) { Qbqe::out_z_op_expects_ = out_z_op_expects; }
const VectorMapND & Qbqe::get_out_z_op_expects() const { return Qbqe::out_z_op_expects_ ; }
//
void Qbqe::set_noise_mitigation(const std::string &noise_mitigation) {
  Qbqe::validate_noise_mitigation(noise_mitigation);
  Qbqe::error_mitigations_.clear();
  Qbqe::error_mitigations_.push_back({noise_mitigation});
}
//
void Qbqe::set_seed(const size_t &in_seed) {
  Qbqe::seeds_.clear();
  Qbqe::seeds_.push_back({in_seed});
}
void Qbqe::set_seeds(const VectorN &in_seeds) { Qbqe::seeds_ = in_seeds; }
const VectorN & Qbqe::get_seeds() const { return Qbqe::seeds_; }
//
void Qbqe::set_noise_mitigations(const VectorString &noise_mitigations) {
  for (const auto& item : noise_mitigations) {
    for (const auto& im : item) {
      Qbqe::validate_noise_mitigation(im);
    }
  }
  Qbqe::error_mitigations_ = noise_mitigations;
}

void Qbqe::validate_noise_mitigation(const std::string &noise_mitigation) {
  if (VALID_ERROR_MITIGATIONS.find(noise_mitigation) == VALID_ERROR_MITIGATIONS.end()) {
      std::stringstream ss;
      ss << "qbOS: valid settings for error mitigation: " << std::endl;
      for (auto it : VALID_ERROR_MITIGATIONS) {
          ss << "* \"" << it << "\"" << std::endl;
      }
      throw std::range_error(ss.str());
  }
}
const VectorString & Qbqe::get_noise_mitigations() const { return Qbqe::error_mitigations_; }
//
const std::string Qbqe::get_summary() const {
  std::ostringstream out;
  out << "* sn:" << std::endl <<
  "    Number of shots" << std::endl <<
  "  = ";
  for (auto item : Qbqe::get_sns()) {
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

  out << "* qpu_config:" << std::endl <<
  "    Filename for JSON file with configuration data for Quantum Brilliance hardware" << std::endl <<
  "  = ";
  for (auto item : get_qpu_configs()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
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

  out << "* aws_device:" << std::endl <<
  "    AWS back-end simulator or QPU" << std::endl <<
  "  = ";
  for (auto item : get_aws_device_names()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* aws_format:" << std::endl <<
  "    AWS Braket language format" << std::endl <<
  "  = ";
  for (auto item : get_aws_formats()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* aws_s3:" << std::endl <<
  "    AWS S3 bucket for storing outputs" << std::endl <<
  "  = ";
  for (auto item : get_aws_s3s()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* aws_s3_path:" << std::endl <<
  "    Path inside [aws_s3] bucket for storing outputs" << std::endl <<
  "  = ";
  for (auto item : get_aws_s3_paths()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* aws_format:" << std::endl <<
  "    AWS Braket language format" << std::endl <<
  "  = ";
  for (auto item : get_aws_formats()) {
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
  "    Enable the QB noise model" << std::endl <<
  "  = ";
  for (auto item : get_noises()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* noise_model:" << std::endl <<
  "    QB noise model name" << std::endl <<
  "  = ";
  for (auto item : get_noise_models()) {
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

out << "* verbatim:" << std::endl <<
  "    Enable the verbatim model" << std::endl <<
  "  = ";
  for (auto item: get_aws_verbatims()) {
      for (auto itel : item) {
              out << " " << itel;
      }
      out << std::endl;
  }
  //

  out << std::endl << std::endl;

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

  out << "* max_bond_dimension:" << std::endl <<
  "    ExaTN-MPS maximum bond dimension" << std::endl <<
  "  = ";
  for (auto item : get_max_bond_dimensions()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  //

  out << "* svd_cutoff:" << std::endl <<
  "    ExaTN-MPS SVD cutoff" << std::endl <<
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

  out << "* out_count:" << std::endl <<
  "    Measured counts" << std::endl <<
  "      [integer] Keys: state labels (assuming BCD format)" << std::endl <<
  "  = ";
  for (auto item : get_out_counts()) {
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
  out << get_debug_qbqe();
  out << std::endl << std::endl;
  //
  return out.str();
}
} // namespace qbOS
