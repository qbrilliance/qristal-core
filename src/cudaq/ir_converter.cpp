// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/cudaq/ir_converter.hpp>
#include <cudaq/utils/cudaq_utils.h>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <cassert>

namespace qristal {
cudaq_ir_converter::cudaq_ir_converter(
    std::shared_ptr<xacc::CompositeInstruction> xacc_ir)
    : m_cudaq_builder(cudaq::make_kernel<std::vector<double>>()),
      m_cudaq_qreg(m_cudaq_builder.qalloc(xacc_ir->nPhysicalBits())) {
  // Retrieve the "exprtk" parsing plugin from the registry
  m_parsing_util = xacc::getService<xacc::ExpressionParsingUtil>("exprtk");
  // Get the list of variables (indexed vector variable names have been assigned during the construction of the XACC IR),
  // i.e., theta[0], theta[1], etc.
  m_var_names = xacc_ir->getVariables();
  xacc::InstructionIterator it(xacc_ir);
  // Visit all IR nodes
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled() && !nextInst->isComposite()) {
      nextInst->accept(this);
    }
  }
}

cudaq::kernel_builder<std::vector<double>> &cudaq_ir_converter::get_cudaq_builder() { return m_cudaq_builder; }

// Visitor implementations
// Add the corresponding CUDAQ gate to the CUDAQ kernel_builder.

// ================ Non-parameterized gates ===============
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
  m_cudaq_builder.s<cudaq::adj>(m_cudaq_qreg[sdg.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::T &t) {
  m_cudaq_builder.t(m_cudaq_qreg[t.bits()[0]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Tdg &tdg) {
  m_cudaq_builder.t<cudaq::adj>(m_cudaq_qreg[tdg.bits()[0]]);
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

void cudaq_ir_converter::visit(xacc::quantum::Swap &swap) {
  m_cudaq_builder.swap(m_cudaq_qreg[swap.bits()[0]],
                       m_cudaq_qreg[swap.bits()[1]]);
}

void cudaq_ir_converter::visit(xacc::quantum::iSwap &iswap) {
  // iSwap: swap two qubit states and phase the |01> and :|10> amplitudes by i.
  // Reference Implementation
  //
  //      ┌───┐┌───┐     ┌───┐
  // q_0: ┤ S ├┤ H ├──■──┤ X ├─────
  //      ├───┤└───┘┌─┴─┐└─┬─┘┌───┐
  // q_1: ┤ S ├─────┤ X ├──■──┤ H ├
  //      └───┘     └───┘     └───┘
  auto q_0 = m_cudaq_qreg[iswap.bits()[0]];
  auto q_1 = m_cudaq_qreg[iswap.bits()[1]];
  m_cudaq_builder.s(q_0);
  m_cudaq_builder.s(q_1);
  m_cudaq_builder.h(q_0);
  m_cudaq_builder.x<cudaq::ctrl>(q_0, q_1);
  m_cudaq_builder.x<cudaq::ctrl>(q_1, q_0);
  m_cudaq_builder.h(q_1);
}

void cudaq_ir_converter::visit(xacc::quantum::CY &cy) {
  m_cudaq_builder.y<cudaq::ctrl>(m_cudaq_qreg[cy.bits()[0]],
                                 m_cudaq_qreg[cy.bits()[1]]);
}

void cudaq_ir_converter::visit(xacc::quantum::Reset &reset) {
  auto qubit = m_cudaq_qreg[reset.bits()[0]];
  m_cudaq_builder.reset(qubit);
}

cudaq::QuakeValue cudaq_ir_converter::instruction_variable_to_quake(
    const xacc::InstructionParameter &xacc_var) {
  assert(xacc_var.isVariable());
  const std::string param_str = xacc_var.toString();
  const auto [mul_factor, variable_name] = get_mul_factor_expression(param_str);
  assert(std::find(m_var_names.begin(), m_var_names.end(), variable_name) !=
         m_var_names.end());

  // The generated CUDAQ kernel takes a single argument of type
  // std::vector<double> we use the index of the XACC variable in the variable
  // name list as the CUDAQ index value when generating the QUAKE IR. This
  // will ensure the same runtime value binding between
  // XACC::Circuit::operator() and CUDAQ kernel invocation.
  const auto var_index = std::distance(
      m_var_names.begin(),
      std::find(m_var_names.begin(), m_var_names.end(), variable_name));
  auto &cudaq_params = m_cudaq_builder.getArguments()[0];
  return mul_factor * cudaq_params[var_index];
}

// ================ Parameterized gates ===============
// Need to handle the case whereby the angle is an expression
// involving a kernel variable.
void cudaq_ir_converter::visit(xacc::quantum::Rx &rx) {
  const auto angle_param = rx.getParameter(0);
  // Check if the angle is a variable
  if (angle_param.isVariable()) {
    m_cudaq_builder.rx(instruction_variable_to_quake(angle_param),
                       m_cudaq_qreg[rx.bits()[0]]);
  } else {
    // Constant angle, just cast it to a double.
    m_cudaq_builder.rx(xacc::InstructionParameterToDouble(angle_param),
                       m_cudaq_qreg[rx.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::Ry &ry) {
  const auto angle_param = ry.getParameter(0);
  if (angle_param.isVariable()) {
    m_cudaq_builder.ry(instruction_variable_to_quake(angle_param),
                       m_cudaq_qreg[ry.bits()[0]]);
  } else {
    m_cudaq_builder.ry(xacc::InstructionParameterToDouble(angle_param),
                       m_cudaq_qreg[ry.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::Rz &rz) {
  const auto angle_param = rz.getParameter(0);
  if (angle_param.isVariable()) {
    m_cudaq_builder.rz(instruction_variable_to_quake(angle_param),
                       m_cudaq_qreg[rz.bits()[0]]);
  } else {
    m_cudaq_builder.rz(xacc::InstructionParameterToDouble(angle_param),
                       m_cudaq_qreg[rz.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::CPhase &cphase) {
  const auto angle_param = cphase.getParameter(0);
  // U1 is called R1 in CUDAQ gate vocabulary
  if (angle_param.isVariable()) {
    m_cudaq_builder.r1<cudaq::ctrl>(instruction_variable_to_quake(angle_param),
                                    m_cudaq_qreg[cphase.bits()[0]],
                                    m_cudaq_qreg[cphase.bits()[1]]);
  } else {
    m_cudaq_builder.r1<cudaq::ctrl>(
        xacc::InstructionParameterToDouble(angle_param),
        m_cudaq_qreg[cphase.bits()[0]], m_cudaq_qreg[cphase.bits()[1]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::CRZ &crz) {
  const auto angle_param = crz.getParameter(0);
  if (angle_param.isVariable()) {
    m_cudaq_builder.rz<cudaq::ctrl>(instruction_variable_to_quake(angle_param),
                                    m_cudaq_qreg[crz.bits()[0]],
                                    m_cudaq_qreg[crz.bits()[1]]);
  } else {
    m_cudaq_builder.rz<cudaq::ctrl>(
        xacc::InstructionParameterToDouble(angle_param),
        m_cudaq_qreg[crz.bits()[0]], m_cudaq_qreg[crz.bits()[1]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::U1 &u1) {
  const auto angle_param = u1.getParameter(0);
  // U1 is called R1 in CUDAQ gate vocabulary
  if (angle_param.isVariable()) {
    m_cudaq_builder.r1(instruction_variable_to_quake(angle_param),
                       m_cudaq_qreg[u1.bits()[0]]);
  } else {
    m_cudaq_builder.r1(xacc::InstructionParameterToDouble(angle_param),
                       m_cudaq_qreg[u1.bits()[0]]);
  }
}

void cudaq_ir_converter::visit(xacc::quantum::U &u3) {
  // U3(theta, phi, lambda) ==
  // --Ry(pi/2)--Rx(lambda)--Ry(theta)--Rx(phi)--Ry(-pi/2)--
  auto qbit = m_cudaq_qreg[u3.bits()[0]];
  const auto theta = u3.getParameter(0);
  const auto phi = u3.getParameter(1);
  const auto lambda = u3.getParameter(2);
  m_cudaq_builder.ry(M_PI_2, qbit);
  if (lambda.isVariable()) {
    m_cudaq_builder.rx(instruction_variable_to_quake(lambda), qbit);
  } else {
    m_cudaq_builder.rx(xacc::InstructionParameterToDouble(lambda), qbit);
  }
  if (theta.isVariable()) {
    m_cudaq_builder.ry(instruction_variable_to_quake(theta), qbit);
  } else {
    m_cudaq_builder.ry(xacc::InstructionParameterToDouble(theta), qbit);
  }
  if (phi.isVariable()) {
    m_cudaq_builder.rx(instruction_variable_to_quake(phi), qbit);
  } else {
    m_cudaq_builder.rx(xacc::InstructionParameterToDouble(phi), qbit);
  }
  m_cudaq_builder.ry(-M_PI_2, qbit);
}

std::pair<double, std::string>
cudaq_ir_converter::get_mul_factor_expression(const std::string &expr_str) {
  // Currently, we only support multiplications in rotation angle expressions.
  // Note: this is the use case for all XACC built-in parameterized circuits,
  // e.g., exp_i_theta, ASWAP, UCCSD, etc.
  // To handle generic cases, a classical expression parser and full traversal
  // of the parse tree will be needed.
  const auto mul_pos = expr_str.find("*");
  if (mul_pos != std::string::npos) {
    auto lhs = expr_str.substr(0, mul_pos);
    auto rhs = expr_str.substr(mul_pos + 1);
    cudaq::trim(lhs);
    cudaq::trim(rhs);
    double factor = 0.0;
    // Either LHS or RHS can be a constant
    if (m_parsing_util->isConstant(lhs, factor)) {
      // RHS is the variable name
      return std::make_pair(factor, rhs);
    } else {
      const bool isConst = m_parsing_util->isConstant(rhs, factor);
      assert(isConst);
      // LHS is the argument name
      return std::make_pair(factor, lhs);
    }
  }
  // This is a variable name, no multiplication, hence factor = 1.0.
  return std::make_pair(1.0, expr_str);
}
}
