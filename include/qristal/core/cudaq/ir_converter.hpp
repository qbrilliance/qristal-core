// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "cudaq/builder/kernel_builder.h"
#include <memory>

namespace qristal {

/// @brief Utility class to perform XACC IR -> CUDAQ Quake IR (MLIR-based) conversion.
/// This is performed by implementing an `AllGateVisitor` and constructing the corresponding Quake IR node
/// using CUDAQ's kernel_builder utility.
/// This can also handle (classical) argument conversion (parameterized quantum circuit).
/// Notes: 
/// (1) XACC IR (CompositeInstruction) only accepts a single kernel argument of type: double or std::vector<double>.
/// Quake IR (MLIR) can accepts a much wider range of types and not limit the number of arguments.
/// (2) The argument evaluation is XACC IR is very rudimentary. 
/// (based on the C++ Mathematical Expression Parsing And Evaluation Library, https://github.com/ArashPartow/exprtk)
/// Hence, we need to do a 'hacky' conversion into QUAKE IR only supporting scaling (by multiplication) an argument in a rotation angle. 
/// e.g., Ry(0.1234*theta, q[0]), where theta is a parameterized angle bound to argument 'theta'.

class cudaq_ir_converter : public xacc::quantum::AllGateVisitor {
private:
  /// Underlying CUDAQ `kernel_builder`.
  // We specifically use std::vector<double> as the common signature of the
  // generated CUDAQ kernel, which can cover all the XACC IR cases (no
  // arguments, single double or a vector of doubles)
  cudaq::kernel_builder<std::vector<double>> m_cudaq_builder;
  /// Qubit register in Quake IR
  //  It is represented as a generic 'Value' IR node.
  cudaq::QuakeValue m_cudaq_qreg;
  /// List of argument names in the XACC IR
  // Note: indexed vector elements have their own unique names.
  std::vector<std::string> m_var_names;
  /// XACC expression parsing utility to handle string-based rotation angles
  /// (parameterized).
  std::shared_ptr<xacc::ExpressionParsingUtil> m_parsing_util;

public:
  /// @brief Constructor
  ///
  /// @details Taking the root XACC IR node of a kernel and construct the
  /// corresponding CUDAQ <a href="https://nvidia.github.io/cuda-quantum/latest/api/languages/cpp_api.html#kernel-builder">kernel_builder</a>
  /// by visiting all the child nodes.
  /// @param xacc_ir Input XACC IR
  cudaq_ir_converter(std::shared_ptr<xacc::CompositeInstruction> xacc_ir);

  /// @brief Get a reference to the constructed CUDAQ <tt>kernel_builder</tt>,
  /// e.g., to feed it to CUDAQ runtime (execution/vqe, etc.)
  /// @return CUDAQ <tt>kernel_builder</tt>
  cudaq::kernel_builder<std::vector<double>> &get_cudaq_builder();

  // ================================
  // XACC Visitor implementations
  // ================================
  /// Hadamard gate
  void visit(xacc::quantum::Hadamard &h) override;
  /// Pauli X gate
  void visit(xacc::quantum::X &x) override;
  /// Pauli Y gate
  void visit(xacc::quantum::Y &y) override;
  /// Pauli Z gate
  void visit(xacc::quantum::Z &z) override;
  /// S gate
  void visit(xacc::quantum::S &s) override;
  /// Inverse S gate
  void visit(xacc::quantum::Sdg &sdg) override;
  /// T gate
  void visit(xacc::quantum::T &t) override;
  /// Inverse T gate
  void visit(xacc::quantum::Tdg &tdg) override;
  /// Controlled NOT gate
  void visit(xacc::quantum::CNOT &cnot) override;
  /// Controlled Z gate
  void visit(xacc::quantum::CZ &cz) override;
  /// Controlled Hadamard gate
  void visit(xacc::quantum::CH &ch) override;
  /// Rotation about x axis
  void visit(xacc::quantum::Rx &rx) override;
  /// Rotation about y axis
  void visit(xacc::quantum::Ry &ry) override;
  /// Rotation about z axis
  void visit(xacc::quantum::Rz &rz) override;
  /// Controlled phase (aka u1) gate
  void visit(xacc::quantum::CPhase &cphase) override;
  /// Swap gate
  void visit(xacc::quantum::Swap &swap) override;
  /// iSwap gate  
  void visit(xacc::quantum::iSwap &iswap) override;
  /// Controlled Y gate
  void visit(xacc::quantum::CY &cy) override;
  /// Controlled Rz gate
  void visit(xacc::quantum::CRZ &crz) override;
  /// U1 gate (equivalent to Rz up to a global phase) 
  void visit(xacc::quantum::U1 &u1) override;
  /// U3 gate
  void visit(xacc::quantum::U &u3) override;
  /// Reset gate
  void visit(xacc::quantum::Reset &reset) override;

private:
  /// @brief Helper to parse expression of type <constant> * <var_name>
  /// This can handle the no explicit constant multiplication in the expression (constant = 1.0)
  /// @param expr_str Input expression string
  /// @return <constant> and <var_name> pair
  std::pair<double, std::string>
  get_mul_factor_expression(const std::string &expr_str);
  /// Helper to convert an instruction variable to a QuakeValue, aka, a CUDAQ
  /// kernel variable.
  cudaq::QuakeValue instruction_variable_to_quake(
      const xacc::InstructionParameter &xacc_var);
};
}
