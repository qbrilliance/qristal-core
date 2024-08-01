// Copyright Quantum Brilliance Pty Ltd

// STL
#include <algorithm>
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>

// XACC
#include "IRTransformation.hpp"
#include "NoiseModel.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"

// TKET
#include "Mapping/MappingManager.hpp"
#include "Mapping/RoutingMethod.hpp"
#include "Placement/Placement.hpp"
#include "Predicates/CompilerPass.hpp"
#include "Predicates/PassGenerators.hpp"
#include "Predicates/PassLibrary.hpp"

// Qristal
#include "qristal/core/passes/noise_aware_placement_config.hpp"
#include "qristal/core/tket/tket_ir_converter.hpp"
#include "qristal/core/tket/tket_placement.hpp"

namespace qristal {

/**
 * @brief Return the type of this IRTransformation plugin
 *
 * @return Type (placement) of this IRTransformation plugin
 */
const xacc::IRTransformationType TketPlacement::type() const {
  return xacc::IRTransformationType::Placement;
}

/**
 * @brief Return the plugin name (for retrieval from the plugin registry)
 *
 * @return Name of this plugin
 */
const std::string TketPlacement::name() const { return "noise-aware"; }

/**
 * @brief Return the plugin text description
 *
 * @return Description string
 */
const std::string TketPlacement::description() const {
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
std::shared_ptr<xacc::IRTransformation> TketPlacement::clone() {
  return std::make_shared<TketPlacement>();
}

/**
 * @brief Apply the IR transformation procedure
 *
 * @param program Input circuit IR to be transformed by this plugin
 * @param acc Ref. to the backend accelerator
 * @param options Configuration parameters
 */
void TketPlacement::apply(std::shared_ptr<xacc::CompositeInstruction> program,
                          const std::shared_ptr<xacc::Accelerator> acc,
                          const xacc::HeterogeneousMap &options) {
  auto tket_circ = *qristal::tket_ir_converter::to_tket(program);

  auto device_info = [&]() -> std::optional<qristal::noise_aware_placement_config> {
    if (options.keyExists<qristal::noise_aware_placement_config>(
            "noise_aware_placement_config")) {
      return options.get<qristal::noise_aware_placement_config>(
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
      return parseAwsDeviceConnectivity(options.getString("device_properties"));
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
    } else if (acc && acc->getProperties().stringExists("device_properties")) {
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
            {{tket::Node(qPair.first), tket::Node(qPair.second)}, gate_error});
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
      auto backendNoiseModel = providedNoiseModel
                                   ? xacc::as_shared_ptr(providedNoiseModel)
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
      // for (const auto &[q1, q2, fidelity] : twoQubitFidelity) {
      // bugfix for clang-16: capturing a structured binding is not yet supported in OpenMP
      for (const auto& tqf : twoQubitFidelity) {
        const auto& q1 = std::get<0>(tqf);
        const auto& q2 = std::get<1>(tqf);
        const auto& fidelity = std::get<2>(tqf);

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
  auto tket_circ_ptr = xacc::as_shared_ptr(&routed_circuit);
  program->clear();
  program->addInstructions(
      qristal::tket_ir_converter::to_xacc(tket_circ_ptr)->getInstructions());
}

/// Helper method to parse qubit connectivity from AWS device JSON
std::vector<std::pair<int, int>> TketPlacement::parseAwsDeviceConnectivity(
    const std::string &props_json_str) const {
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
TketPlacement::parseAwsDeviceCharacteristics(
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
}
