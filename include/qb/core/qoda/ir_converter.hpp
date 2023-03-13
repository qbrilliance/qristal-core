// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once
#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "qoda/builder/kernel_builder.h"
#include <memory>

namespace qb {
class qoda_ir_converter : public xacc::quantum::AllGateVisitor {
private:
  qoda::kernel_builder<std::vector<double>> m_qoda_builder;
  qoda::QuakeValue m_qoda_qreg;
  std::vector<std::string> m_var_names;
  std::shared_ptr<xacc::ExpressionParsingUtil> m_parsing_util;

public:
  qoda_ir_converter(std::shared_ptr<xacc::CompositeInstruction> xacc_ir);

  qoda::kernel_builder<std::vector<double>> &get_qoda_builder();

  // Visitor implementations
  void visit(xacc::quantum::Hadamard &h) override;

  void visit(xacc::quantum::X &x) override;
  void visit(xacc::quantum::Y &y) override;

  void visit(xacc::quantum::Z &z) override;

  void visit(xacc::quantum::S &s) override;

  void visit(xacc::quantum::Sdg &sdg) override;

  void visit(xacc::quantum::T &t) override;

  void visit(xacc::quantum::Tdg &tdg) override;

  void visit(xacc::quantum::CNOT &cnot) override;

  void visit(xacc::quantum::CZ &cz) override;

  void visit(xacc::quantum::CH &ch) override;

  void visit(xacc::quantum::Rx &rx) override;

  void visit(xacc::quantum::Ry &ry) override;

  void visit(xacc::quantum::Rz &rz) override;

private:
  std::pair<double, std::string>
  get_mul_factor_expression(const std::string &expr_str);
};
} // namespace qb