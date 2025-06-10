// Copyright Quantum Brilliance Pty Ltd

#pragma once
#include <unordered_set>
#include <vector>
#include <numeric>
#include <optional>
#include <Utils.hpp>
#include <itertools.hpp>

namespace xacc {

  /*Generates all possible single alpha-electron excitations.

  This method assumes block-ordered spin-orbitals.

  Args:
      num_alpha: the number of alpha electrons.
      num_spin_orbitals: the total number of spin-orbitals (alpha + alpha spin).
      generalized: boolean flag whether or not to use generalized excitations,
  which ignore the occupation of the spin orbitals. As such, the set of
  generalized excitations is only determined from the number of spin orbitals and
  independent from the number of alpha electrons.

  Returns:
      The list of excitations encoded as pairs. The first entry contains
      the occupied spin orbital index and the second entry the unoccupied one.
  */
  std::vector<std::pair<int, int>>
  get_alpha_excitations(int num_alpha, int num_spin_orbitals,
                        bool generalized = false) {
    std::vector<int> spin_idxs(num_spin_orbitals / 2);
    std::iota(spin_idxs.begin(), spin_idxs.end(), 0);
    if (generalized) {
      std::vector<std::pair<int, int>> result;
      for (const auto &x : iter::combinations(spin_idxs, 2)) {
        result.emplace_back(std::make_pair(x[0], x[1]));
      }
      return result;
    }
    std::vector<int> alpha_occ(num_alpha);
    std::iota(alpha_occ.begin(), alpha_occ.end(), 0);
    assert(num_spin_orbitals / 2 > num_alpha);
    std::vector<int> alpha_unocc(num_spin_orbitals / 2 - num_alpha);
    std::iota(alpha_unocc.begin(), alpha_unocc.end(), num_alpha);
    std::vector<std::pair<int, int>> result;
    for (const auto &[a, b] : iter::product(alpha_occ, alpha_unocc)) {
      result.emplace_back(std::make_pair(a, b));
    }
    return result;
  }

  /*
      Generates all possible single beta-electron excitations.

      This method assumes block-ordered spin-orbitals.

      Args:
          num_beta: the number of beta electrons.
          num_spin_orbitals: the total number of spin-orbitals (alpha + beta
          spin). generalized: boolean flag whether or not to use generalized
          excitations, which ignore the
              occupation of the spin orbitals. As such, the set of generalized
              excitations is only determined from the number of spin orbitals
              and independent from the number of beta electrons.

      Returns:
          The list of excitations encoded as tuples. Each tuple is a pair. The
          first entry contains the occupied spin orbital index and the second
          entry the unoccupied one.
  */
  std::vector<std::pair<int, int>>
  get_beta_excitations(int num_beta, int num_spin_orbitals,
                       bool generalized = false) {
    std::vector<int> spin_idxs(num_spin_orbitals - num_spin_orbitals / 2);
    std::iota(spin_idxs.begin(), spin_idxs.end(), num_spin_orbitals / 2);
    if (generalized) {
      std::vector<std::pair<int, int>> result;
      for (const auto &x : iter::combinations(spin_idxs, 2)) {
        result.emplace_back(std::make_pair(x[0], x[1]));
      }
      return result;
    }

    const auto beta_index_offset = num_spin_orbitals / 2;
    std::vector<int> beta_occ(num_beta);
    std::iota(beta_occ.begin(), beta_occ.end(), beta_index_offset);
    assert(num_spin_orbitals / 2 > num_beta);
    std::vector<int> beta_unocc;
    for (int spinIdx = beta_index_offset + num_beta; spinIdx < num_spin_orbitals;
         ++spinIdx) {
      beta_unocc.emplace_back(spinIdx);
    }
    std::vector<std::pair<int, int>> result;
    for (const auto &[a, b] : iter::product(beta_occ, beta_unocc)) {
      result.emplace_back(std::make_pair(a, b));
    }
    return result;
  }

  // first vector contains the occupied spin orbital indices whereas the second
  // one contains the indices of the unoccupied spin orbitals.
  using ExcitationPairType = std::pair<std::vector<int>, std::vector<int>>;
  /*
      Generates all possible excitations with the given number of excitations for
     the specified number of particles distributed among the given number of spin
     orbitals.

      The method must be called for each type of excitation (singles, doubles,
     etc.) that is to be considered in the Ansatz. Excitations will be produced
     based on an initial `Hartree-Fock` occupation by default unless `generalized`
     is set to `True`, in which case the excitations are only determined based on
     the number of spin orbitals and are independent from the number of particles.

      This method assumes block-ordered spin-orbitals.

      Args:
          num_excitations: number of excitations per operator (1 means single
     excitations, etc.). num_spin_orbitals: number of spin-orbitals.
          num_particles: number of alpha and beta particles.
          alpha_spin: boolean flag whether to include alpha-spin excitations.
          beta_spin: boolean flag whether to include beta-spin excitations.
      Returns:
          The list of excitations encoded as pair of arrays. The
          first array contains the occupied spin orbital indices whereas the
     second one contains the indices of the unoccupied spin orbitals.
  */
  std::vector<ExcitationPairType>
  generate_fermionic_excitations(int num_excitations, int num_spin_orbitals,
                                 const std::pair<int, int> &num_particles,
                                 bool alpha_spin = true, bool beta_spin = true,
                                 bool generalized = false,
                                 bool preserve_spin = true) {
    std::vector<std::pair<int, int>> alpha_excitations;
    std::vector<std::pair<int, int>> beta_excitations;
    if (preserve_spin) {
      if (alpha_spin) {
        alpha_excitations = get_alpha_excitations(num_particles.first,
                                                  num_spin_orbitals, generalized);
      }
      if (beta_spin) {
        beta_excitations = get_beta_excitations(num_particles.second,
                                                num_spin_orbitals, generalized);
      }
    } else {
      auto single_excitations =
          get_alpha_excitations(num_particles.first + num_particles.second,
                                num_spin_orbitals * 2, false);

      const auto interleaved2blocked = [](int index, int total) -> int {
        if ((index % 2) == 0) {
          return index / 2;
        }
        return (index - 1 + total) / 2;
      };

      for (const auto &[occ_interleaved, unocc_interleaved] :
           single_excitations) {
        const auto occ_blocked =
            interleaved2blocked(occ_interleaved, num_spin_orbitals);
        const auto unocc_blocked =
            interleaved2blocked(unocc_interleaved, num_spin_orbitals);

        if ((occ_interleaved % 2) == 0) {
          alpha_excitations.emplace_back(
              std::make_pair(occ_blocked, unocc_blocked));
        } else {
          beta_excitations.emplace_back(
              std::make_pair(occ_blocked, unocc_blocked));
        }
      }
      std::sort(alpha_excitations.begin(), alpha_excitations.end());
      std::sort(beta_excitations.begin(), beta_excitations.end());
    }

    if (alpha_excitations.empty() && beta_excitations.empty()) {
      // std::cout << "Empty excitations!\n";
      return {};
    }
    //   std::cout << "Alpha: ";
    //   for (const auto &[a, b] : alpha_excitations) {
    //     std::cout << "(" << a << ", " << b << ") ";
    //   }
    //   std::cout << "\n";
    std::vector<std::pair<int, int>> combined_exciations = alpha_excitations;
    combined_exciations.insert(combined_exciations.end(),
                               beta_excitations.begin(), beta_excitations.end());
    const auto pool = iter::combinations(combined_exciations, num_excitations);
    std::vector<ExcitationPairType> excitations;
    std::set<std::set<int>> visited_excitations;
    for (auto &exc : pool) {
      assert(exc.size() == num_excitations);
      std::set<int> exc_set;
      for (const auto &val : exc) {
        exc_set.emplace(val.first);
        exc_set.emplace(val.second);
      }
      if (exc_set.size() == (2 * num_excitations) &&
          !xacc::container::contains(visited_excitations, exc_set)) {
        visited_excitations.emplace(exc_set);
        std::vector<int> occ, unocc;
        for (const auto &val : exc) {
          // std::cout << val << "\n";
          occ.emplace_back(val.first);
          unocc.emplace_back(val.second);
        }
        excitations.emplace_back(std::make_pair(occ, unocc));
      }
    }
    return excitations;
  }

  std::string excitationsToString(const std::vector<ExcitationPairType> &excitations) {
    std::stringstream ss;
    int count = 0;
    ss << "[";
    for (const auto &[a, b] : excitations) {
      ss << "((";
      for (const auto &x : a) {
        ss << x << ", ";
      }
      ss.seekp(a.size() > 1 ? -2 : -1, ss.cur);
      ss << "), (";
      for (const auto &x : b) {
        ss << x << ", ";
      }
      ss.seekp(b.size() > 1 ? -2 : -1, ss.cur);
      if (count != excitations.size() - 1) {
        ss << ")), ";
      } else {
        ss << "))";
      }
      ++count;
    }
    ss << "]";
    return ss.str();
  }

}
