// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/optimization/qaoa/qaoa_base.hpp"
//#include "qb/core/noise_model/noise_model.hpp"

namespace qb {
namespace op {

class QaoaSimple : public QaoaBase
{
  protected:
    VectorMapND thetas_;  

  public:
    QaoaSimple() : 
      thetas_{{{{0,1.0}}}} { }

    // Constructors
    QaoaSimple(const bool debug) : QaoaBase(debug) { }
    
    // Summary printout
    const std::string get_summary() const override;

    //Setters and Getters
    void set_theta(const ND &in_theta);
    void set_thetas(const VectorMapND &in_thetas);
    const VectorMapND &get_thetas() const;
    static const char *help_thetas_;
    
    int is_ii_consistent() override;
    int is_jj_consistent() override;
    
    // Methods
    void run(const size_t &ii, const size_t &jj) override;
};


class [[deprecated]] QaoaLegacy : public QaoaSimple
{
  public:
    QaoaLegacy() { warn_python_deprecated(); }

    // Constructors
    QaoaLegacy(const bool debug) : QaoaSimple(debug) { warn_python_deprecated(); }

	void warn_python_deprecated(){
	  std::cerr << "Warning: the qaoa class is deprecated in favor of QaoaSimple" << std::endl;
	}
};    

} // namespace op
} // namespace qb
