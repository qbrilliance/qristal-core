// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#pragma once
#include <string>
#include <map>
#include <vector>

namespace qb {
/*
*  Class: Pretranspile
*  Adds custom QB gate definitions (e.g., multi-controlled NOT gates) into the OpenQASM includes
*  and substitudes parameterized angle variables with concrete values (floating point numbers)
*/    
class Pretranspile {
private:
  std::string description_;

protected:
  std::map<std::string, std::string> define_;  // Use this order to prevent warning: -Werror=reorder
  std::map<std::string, std::string> regex_;   // Use this order to prevent warning: -Werror=reorder
  

public:
  // Default contructor
  Pretranspile() : description_("No description"), define_{{}}, regex_{{}} {};

  // Constructor with text description
  Pretranspile(const std::string &description)
      : description_(description), define_{{}}, regex_{{}} {};

  // qb_cn_max_n : scan through a circuit [sorig] and detect an expression [inregex] which contains an integer.
  //               Return the highest integer value found.
  int qb_cn_max_n(const std::string & sorig, const std::string & inregex);

  // qb_cn_max_ns : same functionality as qb_cn_max_n but use a set of regular expressions and find the
  //                highest integer from the set.
  int qb_cn_max_ns(const std::string & sorig, const std::vector<std::string> & inregexs);

  // qb_control : expects OpenQASM [input_circuit]. Generates and inserts multi-control gates using recursion
  //              qb_c<nn>_x
  //              qb_c<nn>_x_dag
  //              qb_c<nn>_ry(theta)
  //              qb_c<nn>_ry_dag(theta)
  //
  //              qb_c<nn-1>_x
  //              qb_c<nn-1>_x_dag
  //              qb_c<nn-1>_ry(theta)
  //              qb_c<nn-1>_ry_dag(theta)
  //               .
  //               .
  //               .
  //              qb_c2_x
  //              qb_c2_x_dag
  //              qb_c2_ry(theta)
  //              qb_c2_ry_dag(theta)
  std::stringstream qb_control(const int &nn);

  // add_n_control_gates : takes OpenQASM [input_circuit] and finds gates matching [qbgates], then
  //                       calls qb_control() to insert the required gate definitions after the [anchor].
  //                       Note: when adding new qbgates, qb_control() also needs updating to handle these.
  void add_n_control_gates(std::string & input_circuit,
                           std::string anchor = "include \"qelib1.inc\";",
                           std::vector<std::string> qbgates = {
                               "qb_c(\\d*)_x ",      // QB multi-control Toffoli
                               "qb_c(\\d*)_x_dag ",  // QB multi-control Toffoli inverse
                               "qb_c(\\d*)_ry",      // QB multi-control Ry(theta), arbitrary theta
                               "qb_c(\\d*)_ry_dag"   // QB multi-control Ry(theta) inverse, arbitrary theta
                            });

  // define_gate : Populates std::map<std::string, std::string> define_
  void define_gate(const std::string &gate_name,
                   const std::string &gate_definition);

  // set_parameter : Populates std::map<std::string, std::string> regex_
  void set_parameter(const std::string &key, const std::string &value);

  // run : takes OpenQASM [input_circuit] and performs regular expression replacements
  //       according to define_gate() and set_parameter()
  void run(std::string &input_circuit,
           std::string anchor = "include \"qelib1.inc\";");
};
} // namespace qb
