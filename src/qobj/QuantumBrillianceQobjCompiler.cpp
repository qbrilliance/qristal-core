/**
 * Copyright Quantum Brilliance
 */
#include "qb/core/qobj/QuantumBrillianceQobjCompiler.hpp"
#include <nlohmann/json.hpp>
#include "xacc.hpp"
#include "xacc_plugin.hpp"

namespace qb {
struct QobjOp {
  std::string name;
  std::vector<size_t> qubits;
  std::vector<size_t> memory;
  std::vector<double> params;
};

void to_json(nlohmann::json &ret, const QobjOp &op) {
  ret["name"] = op.name;
  if (!op.qubits.empty()) {
    ret["qubits"] = op.qubits;
  }
  if (!op.memory.empty()) {
    ret["memory"] = op.memory;
  }
  if (!op.params.empty()) {
    ret["params"] = op.params;
  }
}

std::shared_ptr<xacc::IR> QuantumBrillianceQobjCompiler::compile(
    const std::string &src, std::shared_ptr<xacc::Accelerator> acc) {
  return xacc::getCompiler("qobj")->compile(src, acc);
}
std::shared_ptr<xacc::IR>
QuantumBrillianceQobjCompiler::compile(const std::string &src) {
  return xacc::getCompiler("qobj")->compile(src);
}
const std::string QuantumBrillianceQobjCompiler::translate(
    std::shared_ptr<xacc::CompositeInstruction> function) {
  auto transpiler = xacc::getIRTransformation("qb-gateset-transpiler");
  auto transpiled_ir = xacc::ir::asComposite(function->clone());
  transpiler->apply(transpiled_ir, nullptr);
  std::vector<QobjOp> qobj_instructions;
  for (auto &xacc_inst : transpiled_ir->getInstructions()) {
    if (xacc_inst->name() == "Rx") {
      QobjOp inst;
      inst.qubits = xacc_inst->bits();
      inst.name = "rx";
      inst.params = {mpark::get<double>(xacc_inst->getParameter(0))};
      qobj_instructions.emplace_back(std::move(inst));
    } else if (xacc_inst->name() == "Ry") {
      QobjOp inst;
      inst.qubits = xacc_inst->bits();
      inst.name = "ry";
      inst.params = {mpark::get<double>(xacc_inst->getParameter(0))};
      qobj_instructions.emplace_back(std::move(inst));
    } else if (xacc_inst->name() == "CZ") {
      QobjOp inst;
      inst.qubits = xacc_inst->bits();
      inst.name = "cz";
      qobj_instructions.emplace_back(std::move(inst));
    } else if (xacc_inst->name() == "Measure") {
      QobjOp inst;
      inst.qubits = xacc_inst->bits();
      inst.name = "measure";
      inst.memory = xacc_inst->bits();
      qobj_instructions.emplace_back(std::move(inst));
    } else {
      throw std::runtime_error("Invalid basis instructions.");
    }
  }
  auto default_qobj =
      nlohmann::json::parse(xacc::getCompiler("qobj")->translate(function));
  default_qobj["qObject"]["experiments"][0]["instructions"] = qobj_instructions;
  return default_qobj.dump();
}

const std::string QuantumBrillianceQobjCompiler::translate(
    std::shared_ptr<xacc::CompositeInstruction> function,
    xacc::HeterogeneousMap &options) {
  return translate(function);
}
} // namespace qb

REGISTER_COMPILER(qb::QuantumBrillianceQobjCompiler)