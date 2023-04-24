// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/cudaq/ir_converter.hpp"
#include "cudaq/utils/cudaq_utils.h"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <cassert>

namespace qb {
cudaq_ir_converter::cudaq_ir_converter(
    std::shared_ptr<xacc::CompositeInstruction> xacc_ir)
    : m_cudaq_builder(cudaq::make_kernel<std::vector<double>>()),
      m_cudaq_qreg(m_cudaq_builder.qalloc(xacc_ir->nPhysicalBits())) {
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

cudaq::kernel_builder<std::vector<double>> &cudaq_ir_converter::get_cudaq_builder() { return m_cudaq_builder; }

// Visitor implementations
void cudaq_ir_converter::visit(xacc::quantum::Hadamard &h) {
  m_cudaq_builder.h(m_cudaq_qreg[h.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::X &x) {
  m_cudaq_builder.x(m_cudaq_qreg[x.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Y &y) {
  m_cudaq_builder.y(m_cudaq_qreg[y.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Z &z) {
  m_cudaq_builder.z(m_cudaq_qreg[z.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::S &s) {
  m_cudaq_builder.s(m_cudaq_qreg[s.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Sdg &sdg) {
  // Workaround missing Rval overload in cudaq::kernel_builder adjoint.
  // Hence, need to have a temp. variable.
  auto qubit = m_cudaq_qreg[sdg.bits()[0]];
  m_cudaq_builder.s<cudaq::adj>(qubit);
}

void cudaq_ir_converter::visit(xacc::quantum::T &t) {
  m_cudaq_builder.t(m_cudaq_qreg[t.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Tdg &tdg) {
  // Workaround missing Rval overload in cudaq::kernel_builder adjoint.
  // Hence, need to have a temp. variable.
  auto qubit = m_cudaq_qreg[tdg.bits()[0]];
  m_cudaq_builder.t<cudaq::adj>(qubit);
}

void cudaq_ir_converter::visit(xacc::quantum::CNOT &cnot) {
  m_cudaq_builder.x<cudaq::ctrl>(m_cudaq_qreg[cnot.bits()[0]],
                                m_cudaq_qreg[cnot.bits()[1]]);
}

void cudaq_ir_converter::visit(xacc::quantum::CZ &cz) {
  m_cudaq_builder.z<cudaq::ctrl>(m_cudaq_qreg[cz.bits()[0]],
                                m_cudaq_qreg[cz.bits()[1]]);
}

void cudaq_ir_converter::visit(xacc::quantum::CH &ch) {
  m_cudaq_builder.h<cudaq::ctrl>(m_cudaq_qreg[ch.bits()[0]],
                                m_cudaq_qreg[ch.bits()[1]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Rx &rx) {
  auto &cudaq_params = m_cudaq_builder.getArguments()[0];
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

    m_cudaq_builder.rx(mul_factor * cudaq_params[var_index],
                       m_cudaq_qreg[rx.bits()[0]]);
  } else {
    m_cudaq_builder.rx(xacc::InstructionParameterToDouble(rx.getParameter(0)),
                       m_cudaq_qreg[rx.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::Ry &ry) {
  auto &cudaq_params = m_cudaq_builder.getArguments()[0];
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

    m_cudaq_builder.ry(mul_factor * cudaq_params[var_index],
                       m_cudaq_qreg[ry.bits()[0]]);
  } else {
    m_cudaq_builder.ry(xacc::InstructionParameterToDouble(ry.getParameter(0)),
                       m_cudaq_qreg[ry.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::Rz &rz) {
  auto &cudaq_params = m_cudaq_builder.getArguments()[0];
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
    m_cudaq_builder.rz(mul_factor * cudaq_params[var_index],
                       m_cudaq_qreg[rz.bits()[0]]);
  } else {
    m_cudaq_builder.rz(xacc::InstructionParameterToDouble(rz.getParameter(0)),
                       m_cudaq_qreg[rz.bits()[0]]);
  }
}

std::pair<double, std::string>
cudaq_ir_converter::get_mul_factor_expression(const std::string &expr_str) {
  const auto mul_pos = expr_str.find("*");
  if (mul_pos != std::string::npos) {
    auto lhs = expr_str.substr(0, mul_pos);
    auto rhs = expr_str.substr(mul_pos + 1);
    cudaq::trim(lhs);
    cudaq::trim(rhs);
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
