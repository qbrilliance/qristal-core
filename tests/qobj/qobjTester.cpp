// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include <gtest/gtest.h>
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/backends/qb_hardware/qb_visitor.hpp"
#include <nlohmann/json.hpp>
namespace {
std::shared_ptr<xacc::Compiler> get_qb_qobj_compiler() {
  return xacc::getCompiler("qristal-qobj");
}
} // namespace

TEST(transpilationTester, checkQBAccTranspile) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      CNOT(q[1], q[2]);
      CNOT(q[0], q[1]);
      H(q[0]);
    })",
                               nullptr)
                     ->getComposites()[0];

  auto visitor = std::make_shared<xacc::quantum::qb_visitor>(3);
  xacc::InstructionIterator it(program);
  while (it.hasNext()) {
    // Get the next node in the tree
    auto nextInst = it.next();
    if (nextInst->isEnabled()) {
      nextInst->accept(visitor);
    }
  }
  auto ir = visitor->getTranspiledIR();
  std::cout << "NATIVE IR:\n" << ir->toString() << "\n";
  for (const auto &inst : ir->getInstructions()) {
    EXPECT_TRUE(inst->name() == "Rx" || inst->name() == "Ry" ||
                inst->name() == "CZ");
  }
}

TEST(transpilationTester, checkQObjTranspile) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
    })",
                               nullptr)
                     ->getComposites()[0];

  auto compiler = get_qb_qobj_compiler();
  const auto qobj_str = compiler->translate(program);
  auto qobj = nlohmann::json::parse(qobj_str);
  auto insts_json = qobj["qObject"]["experiments"][0]["instructions"];
  // H -> |Ry(0.5*pi)|--|Rx(pi)|
  EXPECT_EQ(insts_json.size(), 2);
  EXPECT_EQ(insts_json[0]["name"], "ry");
  EXPECT_NEAR(insts_json[0]["params"][0].get<double>(), M_PI_2, 1e-3);
  EXPECT_EQ(insts_json[1]["name"], "rx");
  EXPECT_NEAR(insts_json[1]["params"][0].get<double>(), M_PI, 1e-3);
}

TEST(transpilationTester, checkAerSim) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      X(q[0]);
      Measure(q[0]);
    })",
                               nullptr)
                     ->getComposites()[0];

  auto accelerator = xacc::getAccelerator(
      "aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
  auto qobj = nlohmann::json::parse(accelerator->getNativeCode(program));
  // X -> Rx(pi)
  EXPECT_EQ(qobj["experiments"][0]["instructions"][0]["name"], "rx");
  EXPECT_NEAR(
      qobj["experiments"][0]["instructions"][0]["params"][0].get<double>(),
      M_PI, 1e-3);
}

TEST(transpilationTester, checkAerNoiseSim1) {
  qristal::NoiseModel noise_model;
  const size_t nb_qubits = 2;

  // Create a test noise model with supper-strong depolarizing error on all
  // single qubit gates "u1", "u2", "u3", and no error on two qubit gates.
  for (const auto &gate_name : {"u1", "u2", "u3"}) {
    for (size_t qId = 0; qId < nb_qubits; ++qId) {
      noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, 0.25),
                                 gate_name, {qId});
    }
  }

  auto xasmCompiler = xacc::getCompiler("xasm");
  // A program with single CZ:
  // (1) If the default AER transpilation is used, it will have a high level of
  // noise due to CZ -> u2-CX-u2 decomposition. 
  // (2) If the custom QB QObj
  // compiler is used, CZ will be noise free (since we didn't specify any
  // two-qubit noises)
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      CZ(q[0], q[1]);
    })",
                               nullptr)
                     ->getComposites()[0];
  {
    std::cout << "Default QObj\n";
    auto accelerator =
        xacc::getAccelerator("aer", {{"noise-model", noise_model.to_json()},
                                     {"sim-type", "density_matrix"}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    auto dm = accelerator
                  ->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
                      xacc::ExecutionInfo::DmKey);
    for (const auto &row : *dm) {
      for (const auto &x : row) {
        std::cout << x << " ";
      }
      std::cout << "\n";
    }
    // Lots of noise effect (i.e., |00> probability is reduced)
    EXPECT_LT((*dm)[0][0].real(), 0.9);
  }

  {
    std::cout << "QB QObj\n";
    auto accelerator =
        xacc::getAccelerator("aer", {{"noise-model", noise_model.to_json()},
                                     {"sim-type", "density_matrix"},
                                     {"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    auto dm = accelerator
                  ->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
                      xacc::ExecutionInfo::DmKey);
    for (const auto &row : *dm) {
      for (const auto &x : row) {
        std::cout << x << " ";
      }
      std::cout << "\n";
    }
    // No noise (i.e., |00> probability is 1.0)
    EXPECT_NEAR((*dm)[0][0].real(), 1.0, 1e-3);
  }
}


TEST(transpilationTester, checkAerNoiseSim2) {
  qristal::NoiseModel noise_model;
  const size_t nb_qubits = 2;

  // Create a test noise model with supper-strong depolarizing error on all
  // single qubit gates "rx", "ry" and no error on two qubit gates.
  for (const auto &gate_name : {"rx", "ry"}) {
    for (size_t qId = 0; qId < nb_qubits; ++qId) {
      noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, 0.25),
                                 gate_name, {qId});
    }
  }

  auto xasmCompiler = xacc::getCompiler("xasm");
  // A program with single CNOT:
  // (1) If the default AER transpilation is used, it will have no noise since CNOT is a basis gate and no noise was assigned. 
  // (2) If the custom QB QObj compiler is used, CNOT will be noisy due to "rx", "ry" noise
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      CX(q[0], q[1]);
    })",
                               nullptr)
                     ->getComposites()[0];
  {
    std::cout << "Default QObj\n";
    auto accelerator =
        xacc::getAccelerator("aer", {{"noise-model", noise_model.to_json()},
                                     {"sim-type", "density_matrix"}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    auto dm = accelerator
                  ->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
                      xacc::ExecutionInfo::DmKey);
    for (const auto &row : *dm) {
      for (const auto &x : row) {
        std::cout << x << " ";
      }
      std::cout << "\n";
    }
    // No noise (i.e., |00> probability is 1.0)
    EXPECT_NEAR((*dm)[0][0].real(), 1.0, 1e-3);
  }

  {
    std::cout << "QB QObj\n";
    auto accelerator =
        xacc::getAccelerator("aer", {{"noise-model", noise_model.to_json()},
                                     {"sim-type", "density_matrix"},
                                     {"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    auto dm = accelerator
                  ->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
                      xacc::ExecutionInfo::DmKey);
    for (const auto &row : *dm) {
      for (const auto &x : row) {
        std::cout << x << " ";
      }
      std::cout << "\n";
    }
    // Lots of noise effect (i.e., |00> probability is reduced)
    EXPECT_LT((*dm)[0][0].real(), 0.9);
  }
}
