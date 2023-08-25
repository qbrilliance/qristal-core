// Copyright 2023 Quantum Brilliance

#include "qb/core/optimization/vqee/vqee.hpp"

namespace qb::vqee {
  std::shared_ptr<xacc::Optimizer> NLO::get() {
    show_info();
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
    std::shared_ptr<xacc::Optimizer> ret_optimizer = xacc::getOptimizer("nlopt");
    ret_optimizer->setOptions(xoptions);
    return ret_optimizer;
  }
  
  // Add other algorithms from nlopt
} //namespace qb::vqee