#include "Circuit.hpp"
#include "CommonGates.hpp"
#include "FermionOperator.hpp"
#include "ObservableTransform.hpp"
#include "OperatorPool.hpp"
#include "PauliOperator.hpp"
#include "Utils.hpp"
#include "qb/core/uccsd/fermionic_excitation_generator.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
#include <Eigen/Dense>
#include <memory>
#include <vector>
using namespace xacc::quantum;
using namespace xacc;

namespace qb {
namespace circuits {
class UCCSD : public xacc::quantum::Circuit {
public:
  UCCSD() : Circuit("UCCSD") {}
  DEFINE_CLONE(UCCSD);

  const std::vector<std::string> requiredKeys() override {
    return {"nq", "ne"};
  }

  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override {
    if (!runtimeOptions.keyExists<int>("nq")) {
      return false;
    }

    if (!runtimeOptions.keyExists<int>("ne")) {
      return false;
    }

    int nQubits = runtimeOptions.get<int>("nq");
    int nElectrons = runtimeOptions.get<int>("ne");

    // Compute the number of parameters
    const auto _nOccupied = (int)std::ceil(nElectrons / 2.0);
    const auto _nVirtual = nQubits / 2 - _nOccupied;
    const auto _nOrbitals = _nOccupied + _nVirtual;
    std::map<std::string, Term> terms;
    std::vector<xacc::InstructionParameter> variables;
    const auto num_spin_orbitals = nQubits;
    const auto num_alpha_spins = nElectrons / 2;
    const auto num_beta_spins = nElectrons - num_alpha_spins;
    const auto single_excitations = xacc::generate_fermionic_excitations(
        1, num_spin_orbitals, std::make_pair(num_alpha_spins, num_beta_spins));
    const auto double_excitations = xacc::generate_fermionic_excitations(
        2, num_spin_orbitals, std::make_pair(num_alpha_spins, num_beta_spins));
    auto excitation_list = single_excitations;
    excitation_list.insert(excitation_list.end(), double_excitations.begin(),
                           double_excitations.end());
    const auto nbVariables = excitation_list.size();
    std::vector<std::string> params;

    if (variables.empty()) {
      for (int i = 0; i < nbVariables; ++i) {
        params.push_back("theta" + std::to_string(i));
        variables.push_back(InstructionParameter("theta" + std::to_string(i)));
        addVariable("theta" + std::to_string(i));
      }
    } else {
      for (int i = 0; i < nbVariables; ++i) {
        params.push_back(variables[i].as<std::string>());
      }
    }

    std::vector<FermionOperator> evolvedOps;
    for (int i = 0; i < nbVariables; ++i) {
      const auto exc = excitation_list[i];
      std::vector<std::pair<int, bool>> op_list;
      for (const auto &occ : exc.first) {
        op_list.emplace_back(std::make_pair(occ, true));
      }
      for (const auto &unocc : exc.second) {
        op_list.emplace_back(std::make_pair(unocc, false));
      }
      FermionOperator op(op_list, 1.0);
      op = op - op.hermitianConjugate();
      op = std::complex(0.0, 1.0) * op;
      evolvedOps.emplace_back(std::move(op));
    }
    assert(evolvedOps.size() == params.size());
    // Initial state:
    auto gateRegistry = xacc::getIRProvider("quantum");
    for (int i = (nElectrons / 2) - 1; i >= 0; i--) {
      const std::size_t alpha = (std::size_t)i;
      addInstruction(gateRegistry->createInstruction(
          "X", std::vector<std::size_t>{alpha}));
      const std::size_t beta = (std::size_t)(i + _nOrbitals);
      addInstruction(
          gateRegistry->createInstruction("X", std::vector<std::size_t>{beta}));
    }

    for (int i = 0; i < evolvedOps.size(); ++i) {
      auto &fermionOp = evolvedOps[i];
      auto pauliOpPtr = std::dynamic_pointer_cast<PauliOperator>(
          xacc::getService<ObservableTransform>("jw")->transform(
              xacc::as_shared_ptr(&fermionOp)));
      //   std::cout << "Fermion: " << fermionOp.toString() << "\n";
      //   std::cout << "Pauli: " << pauliOpPtr->toString() << "\n";
      const auto terms = pauliOpPtr->getTerms();
      constexpr double pi = xacc::constants::pi;
      const auto paramLetter = params[i];
      std::vector<xacc::InstPtr> exp_insts;
      for (auto inst : terms) {
        auto spinInst = inst.second;
        if (spinInst.isIdentity()) {
          continue;
        }
        // Get the individual pauli terms
        auto termsMap = std::get<2>(spinInst);

        std::vector<std::pair<int, std::string>> terms;
        for (auto &kv : termsMap) {
          if (kv.second != "I" && !kv.second.empty()) {
            terms.push_back({kv.first, kv.second});
          }
        }
        // The largest qubit index is on the last term
        int largestQbitIdx = terms[terms.size() - 1].first;
        std::vector<std::size_t> qidxs;
        std::vector<xacc::InstPtr> basis_front, basis_back;
        for (auto &term : terms) {
          auto qid = term.first;
          auto pop = term.second;
          qidxs.push_back(qid);
          if (pop == "X") {
            basis_front.emplace_back(
                std::make_shared<xacc::quantum::Hadamard>(qid));
            basis_back.emplace_back(
                std::make_shared<xacc::quantum::Hadamard>(qid));
          } else if (pop == "Y") {
            basis_front.emplace_back(
                std::make_shared<xacc::quantum::Rx>(qid, 1.57079362679));
            basis_back.emplace_back(
                std::make_shared<xacc::quantum::Rx>(qid, -1.57079362679));
          }
        }
        Eigen::MatrixXi cnot_pairs(2, qidxs.size() - 1);
        for (int i = 0; i < qidxs.size() - 1; i++) {
          cnot_pairs(0, i) = qidxs[i];
        }
        for (int i = 0; i < qidxs.size() - 1; i++) {
          cnot_pairs(1, i) = qidxs[i + 1];
        }

        std::vector<xacc::InstPtr> cnot_front, cnot_back;
        for (int i = 0; i < qidxs.size() - 1; i++) {
          Eigen::VectorXi pairs = cnot_pairs.col(i);
          auto c = pairs(0);
          auto t = pairs(1);
          cnot_front.emplace_back(std::make_shared<xacc::quantum::CNOT>(c, t));
        }

        for (int i = qidxs.size() - 2; i >= 0; i--) {
          Eigen::VectorXi pairs = cnot_pairs.col(i);
          auto c = pairs(0);
          auto t = pairs(1);
          cnot_back.emplace_back(std::make_shared<xacc::quantum::CNOT>(c, t));
        }
        exp_insts.insert(exp_insts.end(),
                         std::make_move_iterator(basis_front.begin()),
                         std::make_move_iterator(basis_front.end()));
        exp_insts.insert(exp_insts.end(),
                         std::make_move_iterator(cnot_front.begin()),
                         std::make_move_iterator(cnot_front.end()));

        double coeff;
        if (std::real(spinInst.coeff()) != 0.0) {
          coeff = std::real(spinInst.coeff());
        } else {
          coeff = std::imag(spinInst.coeff());
        }

        std::string p = std::to_string(2.0 * coeff) + " * " + paramLetter;
        exp_insts.emplace_back(
            std::make_shared<xacc::quantum::Rz>(qidxs[qidxs.size() - 1], p));

        exp_insts.insert(exp_insts.end(),
                         std::make_move_iterator(cnot_back.begin()),
                         std::make_move_iterator(cnot_back.end()));
        exp_insts.insert(exp_insts.end(),
                         std::make_move_iterator(basis_back.begin()),
                         std::make_move_iterator(basis_back.end()));
      }

      addInstructions(std::move(exp_insts), false);
    }
    return true;
  }
};
} // namespace circuits
} // namespace qb
REGISTER_PLUGIN(qb::circuits::UCCSD, xacc::Instruction)