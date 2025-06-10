// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/backends/qb_hardware/qb_qpu.hpp>
#include <xacc_plugin.hpp>

namespace qristal {
/**
 * @brief QuantumBrillianceGateSetTransformation transform the input IR using 
 * qb_qpu
 */
class QuantumBrillianceGateSetTransformation : public xacc::IRTransformation {
public:
  /**
   * @brief Construct a new Quantum Brilliance Gate Set Transformation object
   * 
   */
  QuantumBrillianceGateSetTransformation() {}
  /**
   * @brief Apply IR transformation
   * 
   * @param function Input IR (CompositeInstruction)
   * @param accelerator [Optional] Backend Accelerator
   * @param options [Optional] Config parameters
   */
  void apply(std::shared_ptr<xacc::CompositeInstruction> function,
             const std::shared_ptr<xacc::Accelerator> accelerator,
             const xacc::HeterogeneousMap &options = {}) override {
    auto visitor =
        std::make_shared<xacc::quantum::qb_visitor>(
            function->nPhysicalBits());
    xacc::InstructionIterator it(function);
    std::vector<xacc::InstPtr> measure_insts;
    while (it.hasNext()) {
      // Get the next node in the tree
      auto nextInst = it.next();
      if (nextInst->isEnabled()) {
        if (nextInst->name() == "Measure") {
          measure_insts.emplace_back(nextInst);
        } else {
          nextInst->accept(visitor);
        }
      }
    }
    auto transpiled_ir = visitor->getTranspiledIR();
    function->clear();
    function->addInstructions(transpiled_ir->getInstructions());
    function->addInstructions(measure_insts);
  }

  /**
   * @brief Type of this transformation service
   * 
   * @return xacc::IRTransformationType 
   */
  const xacc::IRTransformationType type() const override {
    return xacc::IRTransformationType::Placement;
  }

  /**
   * @brief Name of the service (to retrieve from the service registry)
   * 
   * @return Name
   */
  const std::string name() const override { return "qb-gateset-transpiler"; }
  
  /**
   * @brief Description of this service
   * 
   * @return Description
   */
  const std::string description() const override {
    return "Convert IR to Quantum Brilliance gateset.";
  }
};
}

REGISTER_IRTRANSFORMATION(qristal::QuantumBrillianceGateSetTransformation)
