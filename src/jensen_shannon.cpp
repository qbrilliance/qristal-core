// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/jensen_shannon.hpp>

#include <numeric>

namespace qristal {

  double jensen_shannon(const std::map<std::vector<bool>, int>& counts,
                        const std::map<std::vector<bool>, std::complex<double>>& amplitudes) {
    double divergence = 0.0;
    double total_counts = std::accumulate(counts.begin(), counts.end(), 0,
                          [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
    double d_pm, d_qm;
    for (const auto& [bits, amp] : amplitudes) {
      // Calculate the mixture vector element m(i) = 0.5*(p(i) + q(i))
      double p_i = counts.find(bits) == counts.end() ? 0 : counts.at(bits) / total_counts;
      double q_i = std::norm(amp);
      double m_i = 0.5 * (p_i + q_i);
      // get the Kullback-Leibler divergence of probs p and q wrt m
      if (m_i > 0 && p_i > 0) d_pm += p_i * std::log(p_i/m_i);
      if (m_i > 0 && q_i > 0) d_qm += q_i * std::log(q_i/m_i);
    }

    return (0.5*d_pm) + (0.5*d_qm);
  }

}
