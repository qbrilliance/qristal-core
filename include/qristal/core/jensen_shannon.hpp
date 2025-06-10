// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include <map>
#include <complex>
#include <vector>

namespace qristal {

  /**
   * @brief Compute the Jensen-Shannon divergence
   *
   * Calculate the divergence between two discrete probability distributions supported in the same space.
   *
   * @param counts: the counts from a quantum simulation, indexed by bit vectors corresponding to measurement basis states.
   * @param amplitudes : the amplitudes for the theoretical distribution of states from which the counts have been sampled.
   *
   * @return Jensen-Shannon divergence of the distribution of predicted amplitudes in_q from the distribution of measured counts in_p,
   * 0.5*(D_KL(in_p||m) + D_KL(in_q||m))
   *
   * where:
   *
   *   m = 0.5*(in_p + in_q)
   *
   *   and
   *
   *   D_KL(X||Y) = Sum_z X(z) * (log(X(z))-log(Y(z)))
   *
   *   is the Kullback-Leibler divergence of probability distributions X and Y.  Here X and Y are column vectors, and
   *   the sum runs over all bitstrings z for which both X and Y are non-zero.
   *
   **/
  double jensen_shannon(const std::map<std::vector<bool>, int>& counts,
                        const std::map<std::vector<bool>, std::complex<double>>& amplitudes);

}
