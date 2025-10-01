// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>
#include <gtest/gtest.h>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <qristal/core/noise_model/noise_model.hpp>
#include <qristal/core/backends/hardware/qb/visitor_CZ.hpp>
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

  auto visitor = std::make_shared<xacc::quantum::visitor_CZ>(3);
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
  auto program = xasmCompiler->compile(
  R"(__qpu__ void test1(qbit q) {
      H(q[0]);
    })",
  nullptr)->getComposites()[0];

  auto compiler = get_qb_qobj_compiler();
  const auto qobj_str = compiler->translate(program);
  auto qobj = nlohmann::json::parse(qobj_str);
  auto insts_json = qobj["qObject"]["experiments"][0]["instructions"];
  // H -> |Ry(0.5*pi)|--|Rx(pi)|
  EXPECT_EQ(insts_json.size(), 2);
  EXPECT_EQ(insts_json[0]["name"], "ry");
  EXPECT_NEAR(insts_json[0]["params"][0].get<double>(), M_PI_2, 1e-3);
  EXPECT_EQ(insts_json[1]["name"], "rx");
  EXPECT_NEAR(std::abs(insts_json[1]["params"][0].get<double>()), M_PI, 1e-3);
}

TEST(transpilationTester, checkAerSim) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler->compile(
  R"(__qpu__ void test1(qbit q) {
      X(q[0]);
      Measure(q[0]);
    })",
  nullptr)->getComposites()[0];

  auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
  auto qobj = nlohmann::json::parse(accelerator->getNativeCode(program));
  // X -> Rx(pi)
  EXPECT_EQ(qobj["experiments"][0]["instructions"][0]["name"], "rx");
  EXPECT_NEAR(std::abs(qobj["experiments"][0]["instructions"][0]["params"][0].get<double>()), M_PI, 1e-3);
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

TEST(transpilationTester, checkSubsetMeasure) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  xacc::HeterogeneousMap config;
  const int shots = 1000;
  config.insert("shots", shots);
  auto program_measure01 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Rx(q[1], pi);
      Measure(q[0]);
      Measure(q[1]);
    })", nullptr)->getComposites()[0];

  auto program_measure0 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Rx(q[1], pi);
      Measure(q[0]);
    })", nullptr)->getComposites()[0];

  auto program_measure1 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Rx(q[1], pi);
      Measure(q[1]);
    })", nullptr)->getComposites()[0];

  { // Check with Xacc QObj generator (default): Measure both qubits
    auto accelerator = xacc::getAccelerator("aer");
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure01);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["10"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }

  { // Check with Xacc QObj generator (default): Measure qubit 0 only
    auto accelerator = xacc::getAccelerator("aer");
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure0);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["0"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["1"], 500, 70);
  }

  { // Check with Xacc QObj generator (default): Measure qubit 1 only
    auto accelerator = xacc::getAccelerator("aer");
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure1);
    buffer->print();
    EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1000);
  }

  { // Check with qristal QObj generator: Measure both qubits
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure01);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["10"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }

  { // Check with qristal QObj generator: Measure qubit 0 only
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure0);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["0"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["1"], 500, 70);
  }

  { // Check with qristal QObj generator: Measure qubit 1 only
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    accelerator->updateConfiguration(config);
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program_measure1);
    buffer->print();
    EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1000);
  }
}

TEST(transpilationTester, checkSubsetMeasure2) {
  // Check qristal QObj generator. Measure a subset of 2 qubits out of a circuit containing 3 qubits.
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program1_measure_all = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      Rx(q[2], pi);
      Measure(q[0]);
      Measure(q[1]);
      Measure(q[2]);
    })", nullptr)->getComposites()[0];

  auto program1_measure01 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      Rx(q[2], pi);
      Measure(q[0]);
      Measure(q[1]);
    })", nullptr)->getComposites()[0];

  auto program1_measure10 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      Rx(q[2], pi);
      Measure(q[1]);
      Measure(q[0]);
    })", nullptr)->getComposites()[0];

  auto program1_measure12 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      Rx(q[2], pi);
      Measure(q[1]);
      Measure(q[2]);
    })", nullptr)->getComposites()[0];

  auto program1_measure21 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      Rx(q[2], pi);
      Measure(q[2]);
      Measure(q[1]);
    })", nullptr)->getComposites()[0];

  auto program2_measure02 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Rx(q[2], pi);
      Measure(q[0]);
      Measure(q[2]);
    })", nullptr)->getComposites()[0];

  auto program2_measure20 = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Rx(q[2], pi);
      Measure(q[2]);
      Measure(q[0]);
    })", nullptr)->getComposites()[0];

  { // Program1: Measure all 3 qubits
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program1_measure_all);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["100"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["110"], 500, 70);
  }

  { // Program1: Measure qubits 0 and 1
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program1_measure01);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["00"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["10"], 500, 70);
  }

  { // Program1: Measure qubits 1 and 0
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program1_measure10);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["00"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["01"], 500, 70);
  }

  { // Program1: Measure qubits 1 and 2
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program1_measure12);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["10"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }

  { // Program1: Measure qubits 2 and 1
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program1_measure21);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["01"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }

  { // Program 2: Measure qubits 0 and 2
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program2_measure02);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["10"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }

  { // Program 2: Measure qubits 0 and 2
    auto accelerator = xacc::getAccelerator("aer", {{"qobj-compiler", get_qb_qobj_compiler()->name()}});
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program2_measure20);
    buffer->print();
    EXPECT_NEAR(buffer->getMeasurementCounts()["01"], 500, 70);
    EXPECT_NEAR(buffer->getMeasurementCounts()["11"], 500, 70);
  }
}

TEST(transpilationTester, checkQObjTranspileSubsetMeasure) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler->compile(R"(__qpu__ void test1(qbit q) {
      Measure(q[1]);
    })", nullptr)->getComposites()[0];

  { // Use default QObj
    auto compiler = xacc::getCompiler("qobj");
    const auto qobj_str = compiler->translate(program);
    // std::cout << "qobj_str:\n" << qobj_str << "\n";
    auto qobj = nlohmann::json::parse(qobj_str);
    auto insts_json = qobj["qObject"]["experiments"][0]["instructions"];
    // std::cout << "insts_json:\n" << insts_json << "\n";
    EXPECT_EQ(insts_json[0]["memory"][0], 0);
    EXPECT_EQ(insts_json[0]["qubits"][0], 1);
  }

  { // Use QB QObj
    auto compiler = xacc::getCompiler("qristal-qobj");
    const auto qobj_str = compiler->translate(program);
    // std::cout << "qobj_str:\n" << qobj_str << "\n";
    auto qobj = nlohmann::json::parse(qobj_str);
    auto insts_json = qobj["qObject"]["experiments"][0]["instructions"];
    // std::cout << "insts_json:\n" << insts_json << "\n";
    EXPECT_EQ(insts_json[0]["memory"][0], 0);
    EXPECT_EQ(insts_json[0]["qubits"][0], 1);
  }
}
