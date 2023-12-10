// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include "qb/core/backends/sparse_simulator/SparseSimulator.h"
#include "qb/core/backends/sparse_simulator/types.h"

// XACC
#include "Accelerator.hpp"
#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "CountGatesOfTypeVisitor.hpp"
#include "GateModifier.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"

// STL
#include <cassert>

namespace xacc {
namespace quantum {
using namespace Microsoft::Quantum::SPARSESIMULATOR;

class SparseSimVisitor : public AllGateVisitor,
                         public InstructionVisitor<Circuit> {
public:
  SparseSimVisitor(size_t nbQubits) : m_sim(nbQubits) {}

  void visit(Hadamard &h) override { m_sim.H(h.bits()[0]); }

  void visit(CNOT &cnot) override {
    m_sim.MCX({static_cast<logical_qubit_id>(cnot.bits()[0])}, cnot.bits()[1]);
  }

  void visit(Rz &rz) override {
    const double angle = InstructionParameterToDouble(rz.getParameter(0));
    m_sim.R(Gates::Basis::PauliZ, angle, rz.bits()[0]);
  }

  void visit(Ry &ry) override {
    const double angle = InstructionParameterToDouble(ry.getParameter(0));
    m_sim.R(Gates::Basis::PauliY, angle, ry.bits()[0]);
  }

  void visit(Rx &rx) override {
    const double angle = InstructionParameterToDouble(rx.getParameter(0));
    m_sim.R(Gates::Basis::PauliX, angle, rx.bits()[0]);
  }

  void visit(X &x) override { m_sim.X(x.bits()[0]); }
  void visit(Y &y) override { m_sim.Y(y.bits()[0]); }
  void visit(Z &z) override { m_sim.Z(z.bits()[0]); }

  void visit(CY &cy) override {
    m_sim.MCY({static_cast<logical_qubit_id>(cy.bits()[0])}, cy.bits()[1]);
  }

  void visit(CZ &cz) override {
    m_sim.MCZ({static_cast<logical_qubit_id>(cz.bits()[0])}, cz.bits()[1]);
  }

  void visit(Swap &s) override { m_sim.SWAP(s.bits()[0], s.bits()[1]); }

  void visit(CRZ &crz) override {
    const double angle = InstructionParameterToDouble(crz.getParameter(0));
    m_sim.MCR({static_cast<logical_qubit_id>(crz.bits()[0])},
              Gates::Basis::PauliZ, angle, crz.bits()[1]);
  }

  void visit(CH &ch) override {
    m_sim.MCH({static_cast<logical_qubit_id>(ch.bits()[0])}, ch.bits()[1]);
  }

  void visit(S &s) override { m_sim.S(s.bits()[0]); }

  void visit(Sdg &sdg) override { m_sim.AdjS(sdg.bits()[0]); }

  void visit(T &t) override { m_sim.T(t.bits()[0]); }

  void visit(Tdg &tdg) override { m_sim.AdjT(tdg.bits()[0]); }

  void visit(CPhase &cphase) override {
    const double angle = InstructionParameterToDouble(cphase.getParameter(0));
    m_sim.MCR1({static_cast<logical_qubit_id>(cphase.bits()[0])}, angle,
                  cphase.bits()[1]);
  }

  void visit(Identity &i) override {}

  void visit(U &u) override {
    const auto theta = InstructionParameterToDouble(u.getParameter(0));
    const auto phi = InstructionParameterToDouble(u.getParameter(1));
    const auto lambda = InstructionParameterToDouble(u.getParameter(2));

    // rz(lmda)
    m_sim.R(Gates::Basis::PauliZ, lambda, u.bits()[0]);

    // ry(theta)

    m_sim.R(Gates::Basis::PauliY, theta, u.bits()[0]);
    // rz(phi)

    m_sim.R(Gates::Basis::PauliZ, phi, u.bits()[0]);
  }

  void visit(iSwap &in_iSwapGate) override {
    xacc::error("Sparse simulator doesn't support iSwap.");
  }

  void visit(fSim &in_fsimGate) override {
    xacc::error("Sparse simulator doesn't support fSim.");
  }

  void visit(IfStmt &ifStmt) override {
    xacc::error("Sparse simulator doesn't support IfStmt.");
  }
  void visit(Measure &measure) override {}
  void visit(Circuit &in_circuit) override {
    // std::cout << "HOWDY: Visit quantum circuit: " << in_circuit.name() <<
    // "\n";
    if (in_circuit.name() == "C-U" &&
        dynamic_cast<xacc::quantum::ControlModifier *>(&in_circuit)) {
      auto *asControlledBlock =
          dynamic_cast<xacc::quantum::ControlModifier *>(&in_circuit);
      assert(asControlledBlock);
      // Controlled circuit
      const auto controlQubits = asControlledBlock->getControlQubits();
      auto baseCircuit = asControlledBlock->getBaseInstruction();
      assert(baseCircuit->isComposite());
      auto asComp = xacc::ir::asComposite(baseCircuit);
      assert(!controlQubits.empty());
      const bool should_perform_mcu_sim = [&]() {
        if (asComp->getInstructions().size() == 1) {
          // Only support these instructions for the moment
          if (asComp->getInstruction(0)->name() == "X" ||
              asComp->getInstruction(0)->name() == "Y" ||
              asComp->getInstruction(0)->name() == "Z" ||
              asComp->getInstruction(0)->name() == "H" ||
              asComp->getInstruction(0)->name() == "Rx" ||
              asComp->getInstruction(0)->name() == "Ry" ||
              asComp->getInstruction(0)->name() == "Rz") {
            return true;
          }
        }

        return false;
      }();

      if (should_perform_mcu_sim) {
        std::vector<logical_qubit_id> ctrlIdx;
        const std::string regName = controlQubits[0].first;
        for (const auto &[reg, idx] : controlQubits) {
          if (reg != regName) {
            xacc::error("Multiple qubit registers are not supported!");
          }
          ctrlIdx.emplace_back(idx);
        }
        assert(ctrlIdx.size() == controlQubits.size());

        auto baseGate = asComp->getInstruction(0);
        const logical_qubit_id targetIdx = baseGate->bits()[0];
        if (xacc::container::contains(
                std::vector<std::string>{"X", "Y", "Z", "H"},
                baseGate->name())) {
          if (baseGate->name() == "X") {
            m_sim.MCX(ctrlIdx, targetIdx);
          } else if (baseGate->name() == "Y") {
            m_sim.MCY(ctrlIdx, targetIdx);

          } else if (baseGate->name() == "Z") {
            m_sim.MCZ(ctrlIdx, targetIdx);

          } else {
            assert(baseGate->name() == "H");
            m_sim.MCH(ctrlIdx, targetIdx);
          }

        } else {
          assert(xacc::container::contains(
              std::vector<std::string>{"Rx", "Ry", "Rz"}, baseGate->name()));
          const double angle =
              InstructionParameterToDouble(baseGate->getParameter(0));
          const Gates::Basis basis = [&baseGate]() {
            if (baseGate->name() == "Rx") {
              return Gates::Basis::PauliX;
            } else if (baseGate->name() == "Ry") {
              return Gates::Basis::PauliY;
            } else {
              assert(baseGate->name() == "Rz");
              return Gates::Basis::PauliZ;
            }
          }();
          m_sim.MCR(ctrlIdx, basis, angle, targetIdx);
        }

        // No need to handle this sub-circuit anymore.
        in_circuit.disable();
        m_controlledBlocks.emplace_back(in_circuit);
      }
    }
  }

  std::map<std::string, int> sample(const std::vector<size_t> &bits,
                                    size_t shots) {
    if (shots == 0) {
      return {};
    }
    std::map<std::string, int> resultMap;
    const auto rawSamples = m_sim.Sample(shots);
    const auto length = rawSamples.front().size();
    for (const auto &rawSample : rawSamples) {
      std::string result;
      for (const auto &bit : bits) {
        assert(bit < length);
        result.push_back(rawSample[length - bit - 1]);
      }
      resultMap[result]++;
    }
    return resultMap;
  }

  ~SparseSimVisitor() {
    for (auto &block : m_controlledBlocks) {
      // We temporarily disabled these blocks while handling the simulation,
      // now reset the status.
      block.get().enable();
    }
    m_controlledBlocks.clear();
  }

private:
  SparseSimulator m_sim;
  std::vector<std::reference_wrapper<xacc::quantum::Circuit>>
      m_controlledBlocks;
};
} // namespace quantum

class SparseSimAccelerator : public xacc::Accelerator,
                             public xacc::Cloneable<xacc::Accelerator> {
private:
  int m_shots = 1024;

public:
  virtual const std::string name() const override { return "sparse-sim"; }
  virtual const std::string description() const override {
    return "Simulation Accelerator based on sparse state-vector "
           "representation.";
  }
  virtual void initialize(const xacc::HeterogeneousMap &params = {}) override {
    m_shots = 1024;
    if (params.keyExists<int>("shots")) {
      m_shots = params.get<int>("shots");
    }
  }
  virtual void
  updateConfiguration(const xacc::HeterogeneousMap &params) override {
    if (params.keyExists<int>("shots")) {
      m_shots = params.get<int>("shots");
    }
  }

  virtual const std::vector<std::string> configurationKeys() override {
    return {};
  }

  virtual xacc::HeterogeneousMap getProperties() override { return {}; }

  virtual void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                       const std::shared_ptr<xacc::CompositeInstruction>
                           compositeInstruction) override {
    xacc::quantum::SparseSimVisitor visitor(buffer->size());
    std::vector<size_t> measureBitIdxs;
    // Walk the IR tree, and visit each node
    InstructionIterator it(compositeInstruction);
    while (it.hasNext()) {
      auto nextInst = it.next();
      if (nextInst->isEnabled()) {
        if (nextInst->name() != "Measure") {
          nextInst->accept(&visitor);
        } else {
          measureBitIdxs.emplace_back(nextInst->bits()[0]);
        }
      }
    }
    const auto measurements = visitor.sample(measureBitIdxs, m_shots);
    //   std::cout << "Measure: " << bitstring << "\n";
    buffer->setMeasurements(measurements);
  }
  virtual BitOrder getBitOrder() override { return BitOrder::LSB; }
  virtual void
  execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
              CompositeInstructions) override {
    for (auto &f : CompositeInstructions) {
      auto tmpBuffer =
          std::make_shared<xacc::AcceleratorBuffer>(f->name(), buffer->size());
      execute(tmpBuffer, f);
      buffer->appendChild(f->name(), tmpBuffer);
    }
  }

  virtual std::shared_ptr<xacc::Accelerator> clone() override {
    return std::make_shared<xacc::SparseSimAccelerator>();
  }
};
} // namespace xacc
REGISTER_ACCELERATOR(xacc::SparseSimAccelerator)
