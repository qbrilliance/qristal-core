// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "qristal/core/optimization/qaoa/qaoa_base.hpp"

namespace qristal {
namespace op {

class QaoaSimple : public QaoaBase
{
  protected:
    Table2d<std::map<int,double>> thetas_;  

  public:
    QaoaSimple() : 
      thetas_{{{{0,1.0}}}} { }

    // Constructors
    QaoaSimple(const bool debug) : QaoaBase(debug) { }
    
    // Summary printout
    const std::string get_summary() const override;

    //Setters and Getters
    void set_theta(const std::map<int,double> &in_theta);
    void set_thetas(const Table2d<std::map<int,double>> &in_thetas);
    const Table2d<std::map<int,double>> &get_thetas() const;
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
}
