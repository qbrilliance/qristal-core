// Copyright Quantum Brilliance Pty Ltd

#include "qb/core/tket/tket_ir_converter.hpp"
#include "AllGateVisitor.hpp"
#include "Circuit/Circuit.hpp"

namespace {
using namespace xacc::quantum;

/// Convert absolute angle values to tket PI normalized values
inline double abs_angle_to_tket(double abs_angle) { return abs_angle / M_PI; }

/// Convert tket PI normalized angle values to absolute values
inline double tket_angle_to_abs(double tket_angle) { return tket_angle * M_PI; }

class TketCircuitVisitor : public AllGateVisitor {
public:
  TketCircuitVisitor(size_t nbQubits) : m_circ(nbQubits, nbQubits) {
    for (size_t q = 0; q < nbQubits; ++q) {
      m_circ.qubit_create(tket::Qubit(q));
    }
  }

  void visit(Hadamard &h) override {
    m_circ.add_op<unsigned>(tket::OpType::H, {(unsigned)h.bits()[0]});
  }

  void visit(CNOT &cnot) override {
    m_circ.add_op<unsigned>(
        tket::OpType::CX, {(unsigned)cnot.bits()[0], (unsigned)cnot.bits()[1]});
  }

  void visit(Rz &rz) override {
    const double angle = InstructionParameterToDouble(rz.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Rz, abs_angle_to_tket(angle), {(unsigned)rz.bits()[0]});
  }

  void visit(U1 &u1) override {
    const double angle = InstructionParameterToDouble(u1.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::U1, abs_angle_to_tket(angle), {(unsigned)u1.bits()[0]});
  }

  void visit(Ry &ry) override {
    const double angle = InstructionParameterToDouble(ry.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Ry, abs_angle_to_tket(angle), {(unsigned)ry.bits()[0]});
  }

  void visit(Rx &rx) override {
    const double angle = InstructionParameterToDouble(rx.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Rx, abs_angle_to_tket(angle), {(unsigned)rx.bits()[0]});
  }

  void visit(X &x) override {
    m_circ.add_op<unsigned>(tket::OpType::X, {(unsigned)x.bits()[0]});
  }
  void visit(Y &y) override {
    m_circ.add_op<unsigned>(tket::OpType::Y, {(unsigned)y.bits()[0]});
  }
  void visit(Z &z) override {
    m_circ.add_op<unsigned>(tket::OpType::Z, {(unsigned)z.bits()[0]});
  }

  void visit(CY &cy) override {
    m_circ.add_op<unsigned>(tket::OpType::CY,
                            {(unsigned)cy.bits()[0], (unsigned)cy.bits()[1]});
  }

  void visit(CZ &cz) override {
    m_circ.add_op<unsigned>(tket::OpType::CZ,
                            {(unsigned)cz.bits()[0], (unsigned)cz.bits()[1]});
  }

  void visit(Swap &s) override {
    m_circ.add_op<unsigned>(tket::OpType::SWAP,
                            {(unsigned)s.bits()[0], (unsigned)s.bits()[1]});
  }

  void visit(CRZ &crz) override {
    const double angle = InstructionParameterToDouble(crz.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::CRz, abs_angle_to_tket(angle),
                            {(unsigned)crz.bits()[0], (unsigned)crz.bits()[1]});
  }

  void visit(CH &ch) override {
    m_circ.add_op<unsigned>(tket::OpType::CH,
                            {(unsigned)ch.bits()[0], (unsigned)ch.bits()[1]});
  }

  void visit(S &s) override {
    m_circ.add_op<unsigned>(tket::OpType::S, {(unsigned)s.bits()[0]});
  }

  void visit(Sdg &sdg) override {
    m_circ.add_op<unsigned>(tket::OpType::Sdg, {(unsigned)sdg.bits()[0]});
  }

  void visit(T &t) override {
    m_circ.add_op<unsigned>(tket::OpType::T, {(unsigned)t.bits()[0]});
  }

  void visit(Tdg &tdg) override {
    m_circ.add_op<unsigned>(tket::OpType::Tdg, {(unsigned)tdg.bits()[0]});
  }

  void visit(CPhase &cphase) override {
    const double angle = InstructionParameterToDouble(cphase.getParameter(0));
    m_circ.add_op<unsigned>(
        tket::OpType::CU1, abs_angle_to_tket(angle),
        {(unsigned)cphase.bits()[0], (unsigned)cphase.bits()[1]});
  }

  void visit(Identity &i) override {}

  void visit(U &u) override {
    const auto theta = InstructionParameterToDouble(u.getParameter(0));
    const auto phi = InstructionParameterToDouble(u.getParameter(1));
    const auto lambda = InstructionParameterToDouble(u.getParameter(2));
    m_circ.add_op<unsigned>(tket::OpType::U3,
                            {abs_angle_to_tket(theta), abs_angle_to_tket(phi),
                             abs_angle_to_tket(lambda)},
                            {(unsigned)u.bits()[0]});
  }

  void visit(iSwap &in_iSwapGate) override {
    throw std::runtime_error("Cannot convert iSwap gate to TKET.");
  }

  void visit(fSim &in_fsimGate) override {
    throw std::runtime_error("Cannot convert fSim gate to TKET.");
  }

  void visit(IfStmt &ifStmt) override {
    throw std::runtime_error(
        "Cannot convert classical IfStmt IR node to TKET.");
  }

  void visit(Measure &measure) override {
    m_circ.add_op<unsigned>(
        tket::OpType::Measure,
        {(unsigned)measure.bits()[0], (unsigned)measure.bits()[0]});
  }

  const tket::Circuit &getTketCircuit() const { return m_circ; }

  ~TketCircuitVisitor() {}

private:
  tket::Circuit m_circ;
};

/// Traits class to be specialised to different values of tket::OpType enum
template <tket::OpType OpT> struct xaccOp;

/// Typedef of XACC gate IR generator function,
/// taking a list of qubit operands and optional parameters.
typedef xacc::InstPtr (*xacc_ir_gen_fn)(
    const std::vector<std::size_t> &,
    const std::vector<xacc::InstructionParameter> &);

/// Map from TKET OpType enum to the corresponding XACC IR generator.
static thread_local std::unordered_map<tket::OpType, xacc_ir_gen_fn>
    XACC_IR_GEN_FNS;

/// Construct a shared pointer to a XACC operation from the correct TKET
/// operation
template <tket::OpType OpT>
xacc::InstPtr
get_xacc_operation(const std::vector<std::size_t> &qbits,
                   const std::vector<xacc::InstructionParameter> &params) {
  using xacc_op_type = xaccOp<OpT>::type;
  // Select the proper constructor to use:
  if constexpr (std::is_constructible<
                    xacc_op_type, std::vector<std::size_t>,
                    std::vector<xacc::InstructionParameter>>{}) {
    // Use the constructor which takes the full list of qubits and params
    // if available (most generic).
    return std::make_shared<xacc_op_type>(qbits, params);
  } else if constexpr (std::is_constructible<xacc_op_type, std::size_t,
                                             xacc::InstructionParameter &&>{}) {
    // Some single-qubit, single-param gates (e.g., Rx, Ry, or Rz)
    // have a simplified constructor signature, 
    // e.g., Rx(std::size_t, xacc::InstructionParameter &&)
    assert(qbits.size() == 1);
    assert(params.size() == 1);
    auto angle_param = params[0];
    return std::make_shared<xacc_op_type>(qbits[0], std::move(angle_param));
  } else {
    // Non-parametric gates
    assert(params.empty());
    return std::make_shared<xacc_op_type>(qbits);
  }
}

/// Util macro to link a tket::OpType enum to a xacc quantum gate op subclass.
/// It also adds the corresponding generator function to the XACC_IR_GEN_FNS
/// map.
#define LINK_OPS(TKET_OP, XACC_OP)                                             \
  template <> struct xaccOp<tket::OpType::TKET_OP> {                           \
    typedef xacc::quantum::XACC_OP type;                                       \
  };                                                                           \
  static thread_local bool TKET_OP##_init_map_entry = []() {                   \
    XACC_IR_GEN_FNS[tket::OpType::TKET_OP] =                                   \
        get_xacc_operation<tket::OpType::TKET_OP>;                             \
    return true;                                                               \
  }();

// Link all the TKET ops that XACC supports.
LINK_OPS(X, X);
LINK_OPS(Y, Y);
LINK_OPS(Z, Z);
LINK_OPS(H, Hadamard);
LINK_OPS(S, S);
LINK_OPS(Sdg, Sdg);
LINK_OPS(T, T);
LINK_OPS(Tdg, Tdg);
LINK_OPS(Rx, Rx);
LINK_OPS(Ry, Ry);
LINK_OPS(Rz, Rz);
LINK_OPS(U3, U);
LINK_OPS(U1, U1);
LINK_OPS(CX, CNOT);
LINK_OPS(CY, CY);
LINK_OPS(CZ, CZ);
LINK_OPS(CH, CH);
LINK_OPS(SWAP, Swap);
LINK_OPS(CRz, CRZ);
LINK_OPS(CU1, CPhase);
LINK_OPS(Measure, Measure);

/// Convert a TKET Command (aka quantum gates) into XACC IR (instruction
/// pointer) Returns null if the command is unsupported.
xacc::InstPtr tket_command_to_xacc_inst(const tket::Command &command) {
  tket::Op_ptr op_ptr = command.get_op_ptr();
  assert(op_ptr);
  const tket::OpType opType = op_ptr->get_type();
  // Collect qubit operands
  const std::vector<tket::Qubit> qubits = command.get_qubits();
  std::vector<std::size_t> xacc_qubits;
  std::transform(
      qubits.cbegin(), qubits.cend(), std::back_inserter(xacc_qubits),
      [](const tket::Qubit &tket_qubit) { return tket_qubit.index()[0]; });
  // Collect gate parameters
  const std::vector<tket::Expr> tket_inst_params = op_ptr->get_params();
  std::vector<xacc::InstructionParameter> xacc_inst_params;
  std::transform(
      tket_inst_params.cbegin(), tket_inst_params.cend(),
      std::back_inserter(xacc_inst_params), [](const tket::Expr &param_expr) {
        return xacc::InstructionParameter(tket_angle_to_abs(tket::eval_expr(param_expr).value()));
      });
  const auto iter = XACC_IR_GEN_FNS.find(opType);
  if (iter != XACC_IR_GEN_FNS.end()) {
    // Invoke the generator
    return iter->second(xacc_qubits, xacc_inst_params);
  }
  return nullptr;
}
} // namespace

using namespace qb;
qb::tket_ir_converter::tket_ir_ptr_t
qb::tket_ir_converter::to_tket(qb::tket_ir_converter::xacc_ir_ptr_t xacc_ir) {
  TketCircuitVisitor visitor(xacc_ir->nPhysicalBits());
  // Walk the IR tree, and visit each node
  xacc::InstructionIterator it(xacc_ir);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled()) {
      nextInst->accept(&visitor);
    }
  }
  return std::make_shared<qb::tket_ir_converter::tket_ir_ptr_t::element_type>(
      visitor.getTketCircuit());
}

qb::tket_ir_converter::xacc_ir_ptr_t
qb::tket_ir_converter::to_xacc(qb::tket_ir_converter::tket_ir_ptr_t tket_ir) {
  auto program = std::make_shared<xacc::quantum::Circuit>("__qb_circuit_from_tket");

  std::map<unsigned, xacc::InstPtr> measure_gates;
  for (const auto &command : *tket_ir) {
    auto xacc_gate = tket_command_to_xacc_inst(command);
    if (!xacc_gate) {
      throw std::runtime_error("Unknown TKET instruction encountered: " +
                               command.to_str());
    }
    if (xacc_gate->name() == "Measure") {
      assert(command.get_args().size() == 2);
      assert(command.get_args()[0].index() == command.get_qubits()[0].index());
      assert(command.get_args()[1].index().size() == 1);
      measure_gates.insert({command.get_args()[1].index()[0], xacc_gate});
    } else {
      const auto iter =
          std::find_if(measure_gates.begin(), measure_gates.end(),
                       [&](const std::pair<unsigned, xacc::InstPtr> &item) {
                         return xacc::container::contains(
                             xacc_gate->bits(), item.second->bits()[0]);
                       });
      if (iter != measure_gates.end()) {
        program->addInstruction(iter->second);
        measure_gates.erase(iter);
      }
      program->addInstruction(xacc_gate);
    }
  }
  // Add back ordered measure gates
  for (auto &[regId, meas] : measure_gates) {
    program->addInstruction(meas);
  }

  return program;
}
