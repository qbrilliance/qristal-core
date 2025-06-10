// Copyright Quantum Brilliance Pty Ltd

#include <qristal/core/profiler.hpp>
#include <qristal/core/backends/qb_hardware/qb_qpu.hpp>
#include <CompositeInstruction.hpp>

namespace qristal {
//
// Methods for qristal::Profiler
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
  run();
}

std::map<int,double> Profiler::get_total_initialisation_maxgate_readout_time_ms(
    const double simulation_total_time, const int shots) {
  std::map<int,double> ret_nd;

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
        (count_1q_gates_on_q_[largestdepth_q_] * gate_1q_time_ms_) +
        (count_2q_gates_on_q_[largestdepth_q_] * gate_2q_time_ms_);
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

std::map<int,int> Profiler::get_count_1q_gates_on_q() {
  std::map<int,int> ret_nn;
  for (const auto &[qId, count_1q] : count_1q_gates_on_q_) {
    ret_nn.insert(std::make_pair(qId, count_1q));
  }
  return ret_nn;
}

std::map<int,int> Profiler::get_count_2q_gates_on_q() {
  std::map<int,int> ret_nn;
  for (const auto &[qId, count_2q] : count_2q_gates_on_q_) {
    ret_nn.insert(std::make_pair(qId, count_2q));
  }
  return ret_nn;
}

int Profiler::get_count_1q_gates_on_q(const int iq) {
  assert(count_1q_gates_on_q_.find(iq) != count_1q_gates_on_q_.end());
  return count_1q_gates_on_q_[iq];
}

int Profiler::get_count_2q_gates_on_q(const int iq) {
  assert(count_2q_gates_on_q_.find(iq) != count_2q_gates_on_q_.end());
  return count_2q_gates_on_q_[iq];
}

int Profiler::get_largestdepth_q() { return largestdepth_q_; }

void Profiler::run() {
  // Walk the circuit and count gates on each qubit line
  std::set<int> qubitIds;
  xacc::InstructionIterator countq(placed_circuit_);
  while (countq.hasNext()) {
    auto nextI = countq.next();
    // Don't count Measure since we have a separate readout time for it.
    if (nextI->isEnabled() && nextI->name() != "Measure") {
      // Which map we need to update?
      auto &map_to_update = nextI->bits().size() == 1 ? count_1q_gates_on_q_
                                                      : count_2q_gates_on_q_;
      // Update gate count on qubit line(s)
      for (const auto &qId : nextI->bits()) {
        map_to_update[qId]++;
        qubitIds.emplace(qId);
      }
    }
  }

  if (debug_) {
    for (const auto &[qId, count] : count_1q_gates_on_q_) {
      std::cout << "[debug]: q" << qId << ": # 1-qubit gates: " << count
                << '\n';
    }
    for (const auto &[qId, count] : count_2q_gates_on_q_) {
      std::cout << "[debug]: q" << qId << ": # 2-qubit gates: " << count
                << '\n';
    }
  }

  // Find out which qubit line has the most gates (1 qubit- + 2 qubit- gates)
  // TODO: this works for our case since 1-q gate time == 2-q gate time.
  // Generally, we need to find the most-critical path by translating the gate count -> circuit runtime.
  // i.e., multiply the runtime/gate for each gate type (1q- or 2q- gates).
  int max_gate_count = 0;
  largestdepth_q_ = 0;
  for (const auto &qId : qubitIds) {
    const auto total_gates_on_q = count_1q_gates_on_q_[qId] + count_2q_gates_on_q_[qId];
    // This qubit line has more gates, update the current max target.
    if (total_gates_on_q > max_gate_count) {
      max_gate_count = total_gates_on_q;
      largestdepth_q_ = qId;
    }
  }

  if (debug_) {
    std::cout << "[debug]: largest depth set by q" << get_largestdepth_q()
              << std::endl;
  }
}
}
