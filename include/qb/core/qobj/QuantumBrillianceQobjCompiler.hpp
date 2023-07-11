/**
 * Copyright Quantum Brilliance
 */
#pragma one
#include "AllGateVisitor.hpp"
#include "Compiler.hpp"
namespace qb {

/**
 * @brief Util class to map IR to QObj Json conforming to
 * QuantumBrillianceRemoteAccelerator transpilation.
 *
 */
class QuantumBrillianceQobjCompiler : public xacc::Compiler,
                                      public xacc::quantum::AllGateVisitor {
public:
  /**
   * @brief Construct a new Quantum Brilliance Qobj Compiler object
   * 
   */
  QuantumBrillianceQobjCompiler() = default;
  
  /**
   * @brief Compile input string
   * 
   * @param src Source string
   * @param acc Backend accelerator
   * @return IR tree
   */
  std::shared_ptr<xacc::IR>
  compile(const std::string &src,
          std::shared_ptr<xacc::Accelerator> acc) override;
  std::shared_ptr<xacc::IR> compile(const std::string &src) override;
  
  /**
   * @brief Translate IR tree (CompositeInstruction) to QObj Json string
   * 
   * @param function CompositeInstruction
   * @return QObj Json string
   */
  const std::string
  translate(std::shared_ptr<xacc::CompositeInstruction> function) override;
  
  /**
   * @brief Translate IR tree (CompositeInstruction) to QObj Json string
   * 
   * @param function CompositeInstruction
   * @param options Extra config option
   * @return QObj Json string
   */
  const std::string
  translate(std::shared_ptr<xacc::CompositeInstruction> function,
            xacc::HeterogeneousMap &options) override;

  /**
   * @brief Name of the service (to retrieve from the service registry)
   *
   * @return Name
   */
  const std::string name() const override { return "qristal-qobj"; }

  /**
   * @brief Description of this service
   *
   * @return Description
   */
  const std::string description() const override {
    return "Quantum Brilliance QObj compiler";
  }

  /**
   * @brief Destroy the Quantum Brilliance Qobj Compiler object
   * 
   */
  ~QuantumBrillianceQobjCompiler() override {}
};
} // namespace qb
