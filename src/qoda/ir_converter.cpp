// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/qoda/ir_converter.hpp"
#include "qoda/utils/qoda_utils.h"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <cassert>

namespace qb {
qoda_ir_converter::qoda_ir_converter(
    std::shared_ptr<xacc::CompositeInstruction> xacc_ir)
    : m_qoda_builder(qoda::make_kernel<std::vector<double>>()),
      m_qoda_qreg(m_qoda_builder.qalloc(xacc_ir->nPhysicalBits())) {
  m_parsing_util = xacc::getService<xacc::ExpressionParsingUtil>("exprtk");
  m_var_names = xacc_ir->getVariables();
  xacc::InstructionIterator it(xacc_ir);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled() && !nextInst->isComposite()) {
      nextInst->accept(this);
    }
  }
}

qoda::kernel_builder<std::vector<double>> &qoda_ir_converter::get_qoda_builder() { return m_qoda_builder; }

// Visitor implementations
void qoda_ir_converter::visit(xacc::quantum::Hadamard &h) {
  m_qoda_builder.h(m_qoda_qreg[h.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::X &x) {
  m_qoda_builder.x(m_qoda_qreg[x.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::Y &y) {
  m_qoda_builder.y(m_qoda_qreg[y.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::Z &z) {
  m_qoda_builder.z(m_qoda_qreg[z.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::S &s) {
  m_qoda_builder.s(m_qoda_qreg[s.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::Sdg &sdg) {
  // This will be fixed soon... (reported to QODA dev team)
  // m_qoda_builder.s<qoda::adj>(m_qoda_qreg[sdg.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::T &t) {
  m_qoda_builder.t(m_qoda_qreg[t.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::Tdg &tdg) {
  // This will be fixed soon... (reported to QODA dev team)
  // m_qoda_builder.t<qoda::adj>(m_qoda_qreg[tdg.bits()[0]]);
}

void qoda_ir_converter::visit(xacc::quantum::CNOT &cnot) {
  m_qoda_builder.x<qoda::ctrl>(m_qoda_qreg[cnot.bits()[0]],
                                m_qoda_qreg[cnot.bits()[1]]);
}

void qoda_ir_converter::visit(xacc::quantum::CZ &cz) {
  m_qoda_builder.z<qoda::ctrl>(m_qoda_qreg[cz.bits()[0]],
                                m_qoda_qreg[cz.bits()[1]]);
}

void qoda_ir_converter::visit(xacc::quantum::CH &ch) {
  m_qoda_builder.h<qoda::ctrl>(m_qoda_qreg[ch.bits()[0]],
                                m_qoda_qreg[ch.bits()[1]]);
}

void qoda_ir_converter::visit(xacc::quantum::Rx &rx) {
  auto &qoda_params = m_qoda_builder.getArguments()[0];
  const auto angle_param = rx.getParameter(0);
  if (angle_param.isVariable()) {
    const auto param_str = angle_param.toString();
    const auto [mul_factor, variable_name] =
        get_mul_factor_expression(param_str);
    assert(std::find(m_var_names.begin(), m_var_names.end(), variable_name) !=
           m_var_names.end());
    const auto var_index = std::distance(
        m_var_names.begin(),
        std::find(m_var_names.begin(), m_var_names.end(), variable_name));

    m_qoda_builder.rx(mul_factor * qoda_params[var_index],
                       m_qoda_qreg[rx.bits()[0]]);
  } else {
    m_qoda_builder.rx(xacc::InstructionParameterToDouble(rx.getParameter(0)),
                       m_qoda_qreg[rx.bits()[0]]);
  }
}

void qoda_ir_converter::visit(xacc::quantum::Ry &ry) {
  auto &qoda_params = m_qoda_builder.getArguments()[0];
  const auto angle_param = ry.getParameter(0);
  if (angle_param.isVariable()) {
    const auto param_str = angle_param.toString();
    const auto [mul_factor, variable_name] =
        get_mul_factor_expression(param_str);
    assert(std::find(m_var_names.begin(), m_var_names.end(), variable_name) !=
           m_var_names.end());

    const auto var_index = std::distance(
        m_var_names.begin(),
        std::find(m_var_names.begin(), m_var_names.end(), variable_name));

    m_qoda_builder.ry(mul_factor * qoda_params[var_index],
                       m_qoda_qreg[ry.bits()[0]]);
  } else {
    m_qoda_builder.ry(xacc::InstructionParameterToDouble(ry.getParameter(0)),
                       m_qoda_qreg[ry.bits()[0]]);
  }
}

void qoda_ir_converter::visit(xacc::quantum::Rz &rz) {
  auto &qoda_params = m_qoda_builder.getArguments()[0];
  const auto angle_param = rz.getParameter(0);
  if (angle_param.isVariable()) {
    const auto param_str = angle_param.toString();
    const auto [mul_factor, variable_name] =
        get_mul_factor_expression(param_str);
    assert(std::find(m_var_names.begin(), m_var_names.end(), variable_name) !=
           m_var_names.end());

    const auto var_index = std::distance(
        m_var_names.begin(),
        std::find(m_var_names.begin(), m_var_names.end(), variable_name));
    m_qoda_builder.rz(mul_factor * qoda_params[var_index],
                       m_qoda_qreg[rz.bits()[0]]);
  } else {
    m_qoda_builder.rz(xacc::InstructionParameterToDouble(rz.getParameter(0)),
                       m_qoda_qreg[rz.bits()[0]]);
  }
}

std::pair<double, std::string>
qoda_ir_converter::get_mul_factor_expression(const std::string &expr_str) {
  const auto mul_pos = expr_str.find("*");
  if (mul_pos != std::string::npos) {
    auto lhs = expr_str.substr(0, mul_pos);
    auto rhs = expr_str.substr(mul_pos + 1);
    qoda::trim(lhs);
    qoda::trim(rhs);
    double factor = 0.0;
    if (m_parsing_util->isConstant(lhs, factor)) {
      return std::make_pair(factor, rhs);
    } else {
      const bool isConst = m_parsing_util->isConstant(rhs, factor);
      assert(isConst);
      return std::make_pair(factor, lhs);
    }
  }

  return std::make_pair(1.0, expr_str);
}
} // namespace qb
