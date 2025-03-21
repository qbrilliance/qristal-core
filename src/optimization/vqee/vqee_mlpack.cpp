// Copyright Quantum Brilliance

#include "qristal/core/optimization/vqee/vqee.hpp"

namespace qristal::vqee {
  std::shared_ptr<xacc::Optimizer> AdamMLP::get() {
    std::cout << "ADAM algorithm provided by mlpack" << "\n";
    xacc::HeterogeneousMap xoptions;
    pass_yaml_to_xacc<int>(m_node_, integer_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::string>(m_node_, string_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<bool>(m_node_, boolean_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<double>(m_node_, double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::vector<double>>(m_node_, vector_double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);

    // These parameters are always required:

    xoptions.insert("initial-parameters", m_initial_parameters_);
    xoptions.insert("nlopt-optimizer", m_algorithm_);
    xoptions.insert("nlopt-maxeval", m_maxeval_);
    xoptions.insert("nlopt-ftol", m_ftol_);
    std::shared_ptr<xacc::Optimizer> ret_optimizer = xacc::getOptimizer("mlpack");
    ret_optimizer->setOptions(xoptions);
    return ret_optimizer;
  }

  std::shared_ptr<xacc::Optimizer> CmaesMLP::get() {
    std::cout << "CMA-ES algorithm provided by mlpack" << "\n";
    xacc::HeterogeneousMap xoptions;
    pass_yaml_to_xacc<int>(m_node_, integer_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::string>(m_node_, string_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<bool>(m_node_, boolean_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<double>(m_node_, double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::vector<double>>(m_node_, vector_double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);

    // These parameters are always required:

    xoptions.insert("initial-parameters", m_initial_parameters_);
    xoptions.insert("nlopt-optimizer", m_algorithm_);
    xoptions.insert("nlopt-maxeval", m_maxeval_);
    xoptions.insert("nlopt-ftol", m_ftol_);
    std::shared_ptr<xacc::Optimizer> ret_optimizer = xacc::getOptimizer("mlpack");
    ret_optimizer->setOptions(xoptions);
    return ret_optimizer;
  }

  std::shared_ptr<xacc::Optimizer> LbfgsMLP::get() {
    std::cout << "L-BFGS algorithm provided by mlpack" << "\n";
    xacc::HeterogeneousMap xoptions;
    pass_yaml_to_xacc<int>(m_node_, integer_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::string>(m_node_, string_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<bool>(m_node_, boolean_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<double>(m_node_, double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);
    pass_yaml_to_xacc<std::vector<double>>(m_node_, vector_double_valued_fields_, all_valid_fields_yaml_xacc_, xoptions, true);

    // These parameters are always required:

    xoptions.insert("initial-parameters", m_initial_parameters_);
    xoptions.insert("nlopt-optimizer", m_algorithm_);
    xoptions.insert("nlopt-maxeval", m_maxeval_);
    xoptions.insert("nlopt-ftol", m_ftol_);
    std::shared_ptr<xacc::Optimizer> ret_optimizer = xacc::getOptimizer("mlpack");
    ret_optimizer->setOptions(xoptions);
    return ret_optimizer;
  }
  // Add other algorithms from mlpack

}
