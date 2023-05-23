// Copyright Quantum Brilliance Pty Ltd

#include "qb/core/profiler.hpp"
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include "CompositeInstruction.hpp"

namespace qb {
//
// Methods for qb::Profiler
//
/*  Class: Profiler
    Profiles the time (in ms) for 1 shot of a given CompositeInstruction.  The caller is responsible for scaling to the number-of-shots by passing this to the constructor.  The number-of-repetitions should be handled by the caller appropriately.
    1.0 Accept a CompositeInstruction (via constructor), or OpenQASM circuit kernel (alternative constructor)
    2.0 With an InstructionIterator, iterate through the circuit and count single-qubit gates (u3 single-qubit 3-parameter gate)
    3.0 With an InstructionIterator, iterate through the circuit and count two-qubit gates
    4.0 With an InstructionIterator, iterate through the circuit and tally the number of gates (single-qubit + two-qubit)
        that apply to each qubit.  From there, find the qubit that sets the max-depth of the circuit.
*/
Profiler::Profiler(std::string target_circuit, const int n_qubits,
                   const double gate_1q_time_ms,
                   const double gate_2q_time_ms,
                   const double q_initialisation_time_ms,
                   const double q_readout_time_ms,
                   const double pc_send_to_control_time_ms,
                   const bool debug)
    : largestdepth_q_(-1), n_qubits_(n_qubits),
      gate_1q_time_ms_(gate_1q_time_ms), gate_2q_time_ms_(gate_2q_time_ms),
      q_initialisation_time_ms_(q_initialisation_time_ms),
      q_readout_time_ms_(q_readout_time_ms),
      pc_send_to_control_time_ms_(pc_send_to_control_time_ms),
      debug_(debug) {
  auto openQasmCompiler = xacc::getCompiler("staq");
  if (!xacc::hasBuffer("q") || xacc::getBuffer("q")->size() != n_qubits) {
    // The profiler taking a QB-transpiled circuit which always use a general
    // buffer named "q", register it with the "Staq" compiler if not yet.
    // e.g., the raw (not-transpiled) circuit uses a different register name.
    auto buffer_q = xacc::qalloc(n_qubits);
    buffer_q->setName("q");
    xacc::storeBuffer(buffer_q);
  }

  std::shared_ptr<xacc::IR> irtarget =
      openQasmCompiler->compile(target_circuit);
  // We expect a single circuit in this OpenQASM input
  assert(irtarget->getComposites().size() == 1);
  placed_circuit_ = irtarget->getComposites().front();

  count_1q_gates_on_q_.resize(n_qubits);
  count_2q_gates_on_q_.resize(n_qubits);
  run();
}

Profiler::Profiler(std::shared_ptr<xacc::CompositeInstruction> &f,
                   const int n_qubits, const double gate_1q_time_ms,
                   const double gate_2q_time_ms,
                   const double q_initialisation_time_ms,
                   const double q_readout_time_ms,
                   const double pc_send_to_control_time_ms,
                   const bool debug)
    : placed_circuit_(f), largestdepth_q_(-1), n_qubits_(n_qubits),
      gate_1q_time_ms_(gate_1q_time_ms), gate_2q_time_ms_(gate_2q_time_ms),
      q_initialisation_time_ms_(q_initialisation_time_ms),
      q_readout_time_ms_(q_readout_time_ms),
      pc_send_to_control_time_ms_(pc_send_to_control_time_ms),
      debug_(debug) {
  count_1q_gates_on_q_.resize(n_qubits);
  count_2q_gates_on_q_.resize(n_qubits);
  run();
}

ND Profiler::get_total_initialisation_maxgate_readout_time_ms(
    const double simulation_total_time, const int shots) {
  ND ret_nd;

  // Initialisation time
  double t_init_ms = n_qubits_ * q_initialisation_time_ms_;
  ret_nd.insert(std::make_pair(KEY_INITIALISATION_TIME, shots * t_init_ms));
  //
  // Readout time
  double t_readout_ms = n_qubits_ * q_readout_time_ms_;
  ret_nd.insert(std::make_pair(KEY_READOUT_TIME, shots * t_readout_ms));
  //
  // Max-depth gate time
  double t_max_depth_gate_ms = 0;
  if (largestdepth_q_ < 0) {
    throw std::invalid_argument("session: no gates found in circuit");
  } else {
    t_max_depth_gate_ms =
        (count_1q_gates_on_q_.at(largestdepth_q_) * gate_1q_time_ms_) +
        (count_2q_gates_on_q_.at(largestdepth_q_) * gate_2q_time_ms_);
    ret_nd.insert(
        std::make_pair(KEY_MAX_DEPTH_GATE_TIME, shots * t_max_depth_gate_ms));
  }
  //
  // Total time
  ret_nd.insert(std::make_pair(
      KEY_TOTAL_TIME,
      shots * (t_init_ms + t_max_depth_gate_ms + t_readout_ms)));
  //
  // Simulation - Total time
  ret_nd.insert(
      std::make_pair(KEY_SIMULATION_TOTAL_TIME, simulation_total_time));
  //
  // Transfer PC -> Controller time
  ret_nd.insert(std::make_pair(
      KEY_PC_SEND_TO_CONTROL_TIME,
      pc_send_to_control_time_ms_));

  return ret_nd;
}

NN Profiler::get_count_1q_gates_on_q() {
  NN ret_nn;
  int key = 0;
  for (auto count_1q : count_1q_gates_on_q_) {
    ret_nn.insert(std::make_pair(key, count_1q));
    key++;
  }
  return ret_nn;
}

NN Profiler::get_count_2q_gates_on_q() {
  NN ret_nn;
  int key = 0;
  for (auto count_2q : count_2q_gates_on_q_) {
    ret_nn.insert(std::make_pair(key, count_2q));
    key++;
  }
  return ret_nn;
}

void Profiler::run() {
  for (size_t iq = 0; iq < n_qubits_; iq++) {
    count_1q_gates_on_q_.at(iq) = 0;
    count_2q_gates_on_q_.at(iq) = 0;
    xacc::InstructionIterator countq(placed_circuit_);
    while (countq.hasNext()) {
      auto nextI = countq.next();
      if (nextI->isEnabled()) {
        if (nextI->bits().size() == 1) {
          if (nextI->bits()[0] == iq)
            count_1q_gates_on_q_.at(iq)++;
        }
        if (nextI->bits().size() == 2) {
          if ((nextI->bits()[0] == iq) || (nextI->bits()[1] == iq))
            count_2q_gates_on_q_.at(iq)++;
        }
      }
    }
    if (debug_) {
      std::cout << "[debug]: q" << iq
                << ": # 1-qubit gates: " << get_count_1q_gates_on_q(iq)
                << std::endl;
      std::cout << "[debug]: q" << iq
                << ": # 2-qubit gates: " << get_count_2q_gates_on_q(iq)
                << std::endl;
    }
  }
  int previous_1q = 0;
  int previous_2q = 0;
  for (int iq = 0; iq < n_qubits_; iq++) {
    if ((count_1q_gates_on_q_.at(iq) + count_2q_gates_on_q_.at(iq)) >
        (previous_1q + previous_2q)) {
      previous_1q = count_1q_gates_on_q_.at(iq);
      previous_2q = count_2q_gates_on_q_.at(iq);
      largestdepth_q_ = iq;
    }
  }
  if (debug_) {
    std::cout << "[debug]: largest depth set by q" << get_largestdepth_q()
              << std::endl;
  }
}
} // namespace qb
