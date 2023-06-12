// Copyright Quantum Brilliance Pty Ltd
#include "NoiseModel.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>
// XACC include
#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
// tket includes
#include "Circuit/Circuit.hpp"
#include "Circuit/Command.hpp"
#include "Mapping/MappingManager.hpp"
#include "Mapping/RoutingMethod.hpp"
#include "OpType/OpType.hpp"
#include "Ops/Op.hpp"
#include "Placement/Placement.hpp"
#include "Predicates/CompilerPass.hpp"
#include "Predicates/PassGenerators.hpp"
#include "Predicates/PassLibrary.hpp"

// QB
#include "qb/core/passes/noise_aware_placement_config.hpp"

namespace xacc {
namespace quantum {
class TketCircuitVisitor : public AllGateVisitor {
public:
  TketCircuitVisitor(size_t nbQubits) : m_circ(nbQubits, nbQubits) {}

  void visit(Hadamard &h) override {
    m_circ.add_op<unsigned>(tket::OpType::H, {(unsigned)h.bits()[0]});
  }

  void visit(CNOT &cnot) override {
    m_circ.add_op<unsigned>(
        tket::OpType::CX, {(unsigned)cnot.bits()[0], (unsigned)cnot.bits()[1]});
  }

  void visit(Rz &rz) override {
    const double angle = InstructionParameterToDouble(rz.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Rz, angle, {(unsigned)rz.bits()[0]});
  }

  void visit(U1 &u1) override {
    const double angle = InstructionParameterToDouble(u1.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::U1, angle, {(unsigned)u1.bits()[0]});
  }

  void visit(Ry &ry) override {
    const double angle = InstructionParameterToDouble(ry.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Ry, angle, {(unsigned)ry.bits()[0]});
  }

  void visit(Rx &rx) override {
    const double angle = InstructionParameterToDouble(rx.getParameter(0));
    m_circ.add_op<unsigned>(tket::OpType::Rx, angle, {(unsigned)rx.bits()[0]});
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
    m_circ.add_op<unsigned>(tket::OpType::CRz, angle,
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
        tket::OpType::CU1, angle,
        {(unsigned)cphase.bits()[0], (unsigned)cphase.bits()[1]});
  }

  void visit(Identity &i) override {}

  void visit(U &u) override {
    const auto theta = InstructionParameterToDouble(u.getParameter(0));
    const auto phi = InstructionParameterToDouble(u.getParameter(1));
    const auto lambda = InstructionParameterToDouble(u.getParameter(2));
    m_circ.add_op<unsigned>(tket::OpType::U3, {theta, phi, lambda},
                            {(unsigned)u.bits()[0]});
  }

  void visit(iSwap &in_iSwapGate) override {
    xacc::error("tket placement doesn't support iSwap.");
  }

  void visit(fSim &in_fsimGate) override {
    xacc::error("tket placement doesn't support fSim.");
  }

  void visit(IfStmt &ifStmt) override {
    xacc::error("tket placement doesn't support IfStmt.");
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

/**
 * @brief Noise-aware circuit placement based on the TKET library
 *
 * This is implemented as a xacc::IRTransformation plugin.
 */
class TketPlacement : public xacc::IRTransformation,
                      public xacc::Cloneable<xacc::IRTransformation> {
public:
  /**
   * @brief Construct a new TketPlacement object
   *
   */
  TketPlacement() {}

  /**
   * @brief Return the type of this IRTransformation plugin
   *
   * @return Type (placement) of this IRTransformation plugin
   */
  const IRTransformationType type() const override {
    return IRTransformationType::Placement;
  }

  /**
   * @brief Return the plugin name (for retrieval from the plugin registry)
   *
   * @return Name of this plugin
   */
  const std::string name() const override { return "noise-aware"; }

  /**
   * @brief Return the plugin text description
   *
   * @return Description string
   */
  const std::string description() const override {
    return "Noise-aware circuit placement based on the TKET library";
  }

  /**
   * @brief Create a new instance of this service.
   *
   * Note: by default, the service registry will return a ref (as a shared
   * pointer) to the same service instance unless clonable (xacc::Cloneable).
   * Clonable service can be used in a thread-safe manner.
   *
   * @return New instance of this service.
   */
  std::shared_ptr<xacc::IRTransformation> clone() override {
    return std::make_shared<xacc::quantum::TketPlacement>();
  }

  /**
   * @brief Apply the IR transformation procedure
   *
   * @param program Input circuit IR to be transformed by this plugin
   * @param acc Ref. to the backend accelerator
   * @param options Configuration parameters
   */
  void apply(std::shared_ptr<CompositeInstruction> program,
             const std::shared_ptr<Accelerator> acc,
             const HeterogeneousMap &options = {}) override {
    TketCircuitVisitor visitor(program->nPhysicalBits());
    // Walk the IR tree, and visit each node
    InstructionIterator it(program);
    while (it.hasNext()) {
      auto nextInst = it.next();
      if (nextInst->isEnabled()) {
        nextInst->accept(&visitor);
      }
    }
    auto tket_circ = visitor.getTketCircuit();

    auto device_info = [&]() -> std::optional<qb::noise_aware_placement_config> {
      if (options.keyExists<qb::noise_aware_placement_config>(
              "noise_aware_placement_config")) {
        return options.get<qb::noise_aware_placement_config>(
            "noise_aware_placement_config");
      } else {
        return std::nullopt;
      }
    }();
    const auto connectivity = [&]() -> std::vector<std::pair<int, int>> {
      if (device_info.has_value()) {
        std::vector<std::pair<int, int>> connectivity;
        for (const auto &[q1, q2] : device_info->qubit_connectivity) {
          connectivity.emplace_back(std::make_pair(q1, q2));
        }
        return connectivity;
      }
      if (options.stringExists("device_properties") &&
          !options.getString("device_properties").empty()) {
        return parseAwsDeviceConnectivity(
            options.getString("device_properties"));
      }
      return acc->getConnectivity();
    }();
    if (connectivity.empty()) {
      std::cout << "No connectivity. Skipped!\n";
      return;
    }
    std::vector<std::pair<unsigned, unsigned>> node_pairs;
    for (const auto &[from, to] : connectivity) {
      node_pairs.emplace_back(std::make_pair(from, to));
    }
    tket::ArchitecturePtr shared_arc =
        std::make_shared<tket::Architecture>(node_pairs);

    std::string backendName;
    if (options.stringExists("backend")) {
      backendName = options.getString("backend");
    }
    std::string backendJson;
    if (options.stringExists("backend-json")) {
      backendJson = options.getString("backend-json");
    } else if (acc && acc->getProperties().stringExists("total-json")) {
      // If this is a remote IBM Accelerator, grab the backend JSON
      // automatically.
      backendJson = acc->getProperties().getString("total-json");
    }

    auto [gate_errors, link_errors, measure_errors] = [&]() {
      if (options.stringExists("device_properties") &&
          !options.getString("device_properties").empty()) {
        return parseAwsDeviceCharacteristics(
            options.getString("device_properties"), connectivity);
      } else if (acc &&
                 acc->getProperties().stringExists("device_properties")) {
        return parseAwsDeviceCharacteristics(
            acc->getProperties().getString("device_properties"), connectivity);
      } else if (device_info.has_value()) {
        tket::avg_readout_errors_t readout_errors;
        tket::avg_node_errors_t single_qubit_gate_errors;
        tket::avg_link_errors_t two_qubit_gate_errors;
        for (const auto &[qId, gate_error] :
             device_info->avg_single_qubit_gate_errors) {
          single_qubit_gate_errors.insert({tket::Node(qId), gate_error});
        }
        for (const auto &[qId, ro_error] :
             device_info->avg_qubit_readout_errors) {
          readout_errors.insert({tket::Node(qId), ro_error});
        }
        for (const auto &[qPair, gate_error] :
             device_info->avg_two_qubit_gate_errors) {
          two_qubit_gate_errors.insert(
              {{tket::Node(qPair.first), tket::Node(qPair.second)},
               gate_error});
        }
        return std::make_tuple(single_qubit_gate_errors, two_qubit_gate_errors,
                               readout_errors);
      }

      tket::avg_readout_errors_t readout_errors;
      tket::avg_node_errors_t single_qubit_gate_errors;
      tket::avg_link_errors_t two_qubit_gate_errors;

      xacc::NoiseModel *providedNoiseModel = nullptr;
      if (options.pointerLikeExists<xacc::NoiseModel>("backend-noise-model")) {
        providedNoiseModel =
            options.getPointerLike<xacc::NoiseModel>("backend-noise-model");
      }
      const bool noiseAwareEnabled =
          !(backendName.empty() && backendJson.empty() && !providedNoiseModel);
      if (!noiseAwareEnabled) {
        // No noise-specific placement can be done.
        std::cout << "No noise information can be retrieved. Only perform "
                     "topology-based placement.\n";
      } else {
        auto backendNoiseModel =
            providedNoiseModel ? xacc::as_shared_ptr(providedNoiseModel)
                               : xacc::getService<xacc::NoiseModel>("IBM");
        if (!backendName.empty()) {
          backendNoiseModel->initialize({{"backend", backendName}});
        } else {
          backendNoiseModel->initialize({{"backend-json", backendJson}});
        }

        const size_t nbQubits = backendNoiseModel->nQubits();
        // Single-qubit gate errors
        const auto singleQubitFidelity =
            backendNoiseModel->averageSingleQubitGateFidelity();
        for (size_t i = 0; i < nbQubits; ++i) {
          single_qubit_gate_errors.insert(
              {tket::Node(i), 1.0 - singleQubitFidelity[i]});
        }

        // Two-qubit gate errors
        const auto twoQubitFidelity =
            backendNoiseModel->averageTwoQubitGateFidelity();
        std::vector<std::pair<size_t, size_t>> processedPairs;
        std::vector<std::tuple<size_t, size_t, double>> avgData;
        for (const auto &[q1, q2, fidelity] : twoQubitFidelity) {
          if (!xacc::container::contains(processedPairs,
                                         std::make_pair(q1, q2))) {
            assert(!xacc::container::contains(processedPairs,
                                              std::make_pair(q2, q1)));
            const double fid1 = fidelity;
            const auto iter =
                std::find_if(twoQubitFidelity.begin(), twoQubitFidelity.end(),
                             [&](const auto &fidTuple) {
                               return (std::get<0>(fidTuple) == q2) &&
                                      (std::get<1>(fidTuple) == q1);
                             });
            const double fid2 =
                iter != twoQubitFidelity.end() ? std::get<2>(*iter) : fid1;
            avgData.emplace_back(std::make_tuple(q1, q2, (fid1 + fid2) / 2.0));
            processedPairs.emplace_back(std::make_pair(q1, q2));
            processedPairs.emplace_back(std::make_pair(q2, q1));
          }
        }
        for (const auto &[q1, q2, avg_fidelity] : avgData) {
          two_qubit_gate_errors.insert(
              {{tket::Node(q1), tket::Node(q2)}, 1.0 - avg_fidelity});
          two_qubit_gate_errors.insert(
              {{tket::Node(q2), tket::Node(q1)}, 1.0 - avg_fidelity});
        }
        // Readout errors
        const auto roErrors = backendNoiseModel->readoutErrors();
        for (size_t i = 0; i < nbQubits; ++i) {
          const auto [meas0Prep1, meas1Prep0] = roErrors[i];
          const double avgRoFidelity =
              0.5 * ((1.0 - meas0Prep1) + (1.0 - meas1Prep0));
          readout_errors.insert({tket::Node(i), 1.0 - avgRoFidelity});
        }
      }
      return std::make_tuple(single_qubit_gate_errors, two_qubit_gate_errors,
                             readout_errors);
    }();
    if (!gate_errors.empty()) {
      // Noise aware placement
      tket::NoiseAwarePlacement placer(*shared_arc, gate_errors, link_errors,
                                       measure_errors);
      placer.place(tket_circ);
    }

    tket::MappingManager manager(shared_arc);
    manager.route_circuit(tket_circ,
                          {std::make_shared<tket::AASRouteRoutingMethod>(1),
                           std::make_shared<tket::LexiLabellingMethod>(),
                           std::make_shared<tket::LexiRouteRoutingMethod>()});
    auto dec = tket::gen_decompose_routing_gates_to_cxs_pass(*shared_arc);
    tket::CompilationUnit cu(tket_circ);
    dec->apply(cu);
    tket::Circuit routed_circuit(cu.get_circ_ref());

    std::map<unsigned, xacc::InstPtr> measure_gates;
    program->clear();
    for (const auto &command : routed_circuit) {
      auto xacc_gate = tKetCommandToXaccInst(command);
      assert(xacc_gate);
      if (xacc_gate->name() == "Measure") {
        assert(command.get_args().size() == 2);
        assert(command.get_args()[0].index() ==
               command.get_qubits()[0].index());
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
  }

private:
  /// Helper to convert TKET gate IR to XACC gate IR
  xacc::InstPtr tKetCommandToXaccInst(const tket::Command &command) {
    auto op_ptr = command.get_op_ptr();
    assert(op_ptr);
    const auto opType = op_ptr->get_type();
    const auto qubits = command.get_qubits();
    switch (opType) {
    case tket::OpType::X:
      return std::make_shared<xacc::quantum::X>(qubits[0].index()[0]);
    case tket::OpType::Y:
      return std::make_shared<xacc::quantum::Y>(qubits[0].index()[0]);
    case tket::OpType::Z:
      return std::make_shared<xacc::quantum::Z>(qubits[0].index()[0]);
    case tket::OpType::S:
      return std::make_shared<xacc::quantum::S>(qubits[0].index()[0]);
    case tket::OpType::Sdg:
      return std::make_shared<xacc::quantum::Sdg>(qubits[0].index()[0]);
    case tket::OpType::T:
      return std::make_shared<xacc::quantum::T>(qubits[0].index()[0]);
    case tket::OpType::Tdg:
      return std::make_shared<xacc::quantum::Tdg>(qubits[0].index()[0]);
    case tket::OpType::H:
      return std::make_shared<xacc::quantum::Hadamard>(qubits[0].index()[0]);
    // Rx, Ry, Rz
    case tket::OpType::Rx:
      return std::make_shared<xacc::quantum::Rx>(
          qubits[0].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value());
    case tket::OpType::Ry:
      return std::make_shared<xacc::quantum::Ry>(
          qubits[0].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value());
    case tket::OpType::Rz:
      return std::make_shared<xacc::quantum::Rz>(
          qubits[0].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value());
    // U gates
    case tket::OpType::U3:
      return std::make_shared<xacc::quantum::U>(
          qubits[0].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value(),
          tket::eval_expr(op_ptr->get_params()[1]).value(),
          tket::eval_expr(op_ptr->get_params()[2]).value());
    case tket::OpType::U1:
      return std::make_shared<xacc::quantum::U1>(
          qubits[0].index()[0],
          xacc::InstructionParameter(
              tket::eval_expr(op_ptr->get_params()[0]).value()));
    // Two-qubit gates
    case tket::OpType::CX:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CNOT>(qubits[0].index()[0],
                                                   qubits[1].index()[0]);
    case tket::OpType::CY:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CY>(qubits[0].index()[0],
                                                 qubits[1].index()[0]);
    case tket::OpType::CZ:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CZ>(qubits[0].index()[0],
                                                 qubits[1].index()[0]);
    case tket::OpType::CH:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CH>(qubits[0].index()[0],
                                                 qubits[1].index()[0]);
    case tket::OpType::SWAP:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::Swap>(qubits[0].index()[0],
                                                   qubits[1].index()[0]);
    case tket::OpType::CRz:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CRZ>(
          qubits[0].index()[0], qubits[1].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value());
    case tket::OpType::CU1:
      assert(qubits.size() == 2);
      return std::make_shared<xacc::quantum::CPhase>(
          qubits[0].index()[0], qubits[1].index()[0],
          tket::eval_expr(op_ptr->get_params()[0]).value());
    // Measure
    case tket::OpType::Measure:
      return std::make_shared<xacc::quantum::Measure>(qubits[0].index()[0]);

    default:
      std::cout << "Unknown: " << command << "\n";
      return nullptr;
    }
  }

  /// Helper method to parse qubit connectivity from AWS device JSON
  std::vector<std::pair<int, int>>
  parseAwsDeviceConnectivity(const std::string &props_json_str) const {
    auto props_json = nlohmann::json::parse(props_json_str);
    auto connectivityGraph =
        props_json["paradigm"]["connectivity"]["connectivityGraph"];
    std::set<std::pair<int, int>> connectivity;
    for (auto it = connectivityGraph.begin(); it != connectivityGraph.end();
         ++it) {
      const int fromQ = std::stoi(it.key());
      for (auto iit = it.value().begin(); iit != it.value().end(); ++iit) {
        const int toQ = std::stoi((*iit).get<std::string>());
        if (fromQ < toQ) {
          connectivity.insert({fromQ, toQ});
        } else {
          connectivity.insert({toQ, fromQ});
        }
      }
    }
    return std::vector<std::pair<int, int>>(connectivity.begin(),
                                            connectivity.end());
  }

  /// Helper to parse TKET noise characteristics (single/double qubit gate
  /// errors, readout errors) from the AWS device property JSON.
  std::tuple<tket::avg_node_errors_t, tket::avg_link_errors_t,
             tket::avg_readout_errors_t>
  parseAwsDeviceCharacteristics(
      const std::string &props_json_str,
      const std::vector<std::pair<int, int>> &connectivity) {
    tket::avg_node_errors_t single_qubit_gate_errors;
    tket::avg_readout_errors_t measure_errors;
    tket::avg_link_errors_t two_qubit_gate_errors;
    auto props_json = nlohmann::json::parse(props_json_str);
    auto oneQubitProperties = props_json["provider"]["specs"]["1Q"];
    for (auto it = oneQubitProperties.begin(); it != oneQubitProperties.end();
         ++it) {
      const int qubitId = std::stoi(it.key());
      const auto data_f1QRB = it.value()["f1QRB"].get<double>();
      assert(data_f1QRB <= 1.0);
      single_qubit_gate_errors.insert({tket::Node(qubitId), 1.0 - data_f1QRB});

      const auto data_fRO = it.value()["fRO"].get<double>();
      assert(data_fRO <= 1.0);
      measure_errors.insert({tket::Node(qubitId), 1.0 - data_fRO});
    }

    auto twoQubitProperties = props_json["provider"]["specs"]["2Q"];
    for (auto it = twoQubitProperties.begin(); it != twoQubitProperties.end();
         ++it) {
      const auto delimPos = it.key().find('-');
      assert(delimPos != std::string::npos);
      const std::string fromQ = it.key().substr(0, delimPos);
      const std::string toQ = it.key().substr(delimPos + 1);
      const int fromQubitId = std::stoi(fromQ);
      const int toQubitId = std::stoi(toQ);

      if (it.value().count("fCZ")) {
        const auto data_fCZ = it.value()["fCZ"].get<double>();
        assert(data_fCZ <= 1.0);
        two_qubit_gate_errors.insert(
            {{tket::Node(fromQubitId), tket::Node(toQubitId)}, 1.0 - data_fCZ});
      } else {
        two_qubit_gate_errors.insert(
            {{tket::Node(fromQubitId), tket::Node(toQubitId)}, 1.0});
      }
    }
    return std::make_tuple(single_qubit_gate_errors, two_qubit_gate_errors,
                           measure_errors);
  }
};
} // namespace quantum
} // namespace xacc
REGISTER_PLUGIN(xacc::quantum::TketPlacement, xacc::IRTransformation)
