// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/passes/swap_placement_pass.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <xacc.hpp>

namespace {
class DummyAccelerator : public xacc::Accelerator {
private:
  const std::vector<std::pair<int, int>> &m_connectivity;

public:
  DummyAccelerator(const std::vector<std::pair<int, int>> &connectivity)
      : m_connectivity(connectivity){};

  virtual std::vector<std::pair<int, int>> getConnectivity() override {
    return m_connectivity;
  }
  virtual const std::string name() const override { return ""; }
  virtual const std::string description() const override { return ""; }
  virtual void initialize(const xacc::HeterogeneousMap &params) override {}
  virtual void
  updateConfiguration(const xacc::HeterogeneousMap &config) override {}

  virtual const std::vector<std::string> configurationKeys() override {
    return {};
  }
  virtual void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                       const std::shared_ptr<xacc::CompositeInstruction>
                           CompositeInstruction) override {}
  virtual void
  execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
              CompositeInstructions) override {}
};
} // namespace

namespace qristal {
// Constructor
swap_placement_pass::swap_placement_pass(
    const std::vector<std::pair<int, int>> &connectivity)
    : m_connectivity(connectivity) {}
swap_placement_pass::swap_placement_pass(std::shared_ptr<xacc::Accelerator> qpu)
    : m_connectivity(qpu->getConnectivity()) {}
/// Returns the pass name.
std::string swap_placement_pass::get_name() const { return "qb-swap"; }

/// Returns the pass description
std::string swap_placement_pass::get_description() const {
  return "SWAP-gate based circuit placement pass.";
}

/// Runs the pass over the circuit IR node
void swap_placement_pass::apply(CircuitBuilder &circuit) {
  auto swap_shortest_path = xacc::getIRTransformation("swap-shortest-path");
  std::shared_ptr<xacc::Accelerator> connectivity_provider =
      std::make_shared<DummyAccelerator>(m_connectivity);
  swap_shortest_path->apply(circuit.get(), connectivity_provider,
                            {{"no-inline", true}});
}
}
