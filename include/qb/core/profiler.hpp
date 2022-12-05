// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#pragma once
#include "qb/core/typedefs.hpp"

// Forward declares
namespace xacc {
class CompositeInstruction;
}

namespace qb {
/*
*  Class: Profiler
*  Profiles the time (in ms) for 1 shot of a given CompositeInstruction.
*  The caller is responsible for scaling to the number-of-shots by passing this to the constructor.
*/
class Profiler {
protected:
  std::shared_ptr<xacc::CompositeInstruction> placed_circuit_;

private:
  std::vector<int> count_1q_gates_on_q_;
  std::vector<int> count_2q_gates_on_q_;
  int largestdepth_q_;
  int n_qubits_;

  // Timing data
  double gate_1q_time_ms_;
  double gate_2q_time_ms_;
  double q_initialisation_time_ms_;
  double q_readout_time_ms_;
  double pc_send_to_control_time_ms_;

  // Debugging
  bool debug_;

public:
  Profiler(std::string target_circuit, const int &n_qubits,
           const double gate_1q_time_ms = 0.001,
           const double gate_2q_time_ms = 0.001,
           const double q_initialisation_time_ms = 30,
           const double q_readout_time_ms = 10,
           const double pc_send_to_control_time_ms = 10000,
           const bool debug = false);

  Profiler(std::shared_ptr<xacc::CompositeInstruction> &f, const int &n_qubits,
           const double gate_1q_time_ms = 0.001,
           const double gate_2q_time_ms = 0.001,
           const double q_initialisation_time_ms = 30,
           const double q_readout_time_ms = 10,
           const double pc_send_to_control_time_ms = 10000,
           const bool debug = false);

  ND get_total_initialisation_maxgate_readout_time_ms(
      const double simulation_total_time = 0.0, const int shots = 1);

  NN get_count_1q_gates_on_q();

  NN get_count_2q_gates_on_q();

  const int get_count_1q_gates_on_q(const int &iq) {
    return count_1q_gates_on_q_.at(iq);
  }

  const int get_count_2q_gates_on_q(const int &iq) {
    return count_2q_gates_on_q_.at(iq);
  }

  const int get_largestdepth_q() { return largestdepth_q_; }

  void run();

  const int KEY_TOTAL_TIME = 0;
  const int KEY_INITIALISATION_TIME = 1;
  const int KEY_MAX_DEPTH_GATE_TIME = 2;
  const int KEY_READOUT_TIME = 3;
  const int KEY_SIMULATION_TOTAL_TIME = 4;
  const int KEY_PC_SEND_TO_CONTROL_TIME = 5;
};
} // namespace qb
