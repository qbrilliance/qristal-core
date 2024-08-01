// Copyright Quantum Brilliance Pty Ltd

#pragma once
#include "qristal/core/typedefs.hpp"
#include <memory>
#include <unordered_map>
#include <map>

// Forward declaration
namespace xacc { class CompositeInstruction; }

namespace qristal {

  /*
  *  Class: Profiler
  *  Profiles the time (in ms) for 1 shot of a given CompositeInstruction.
  *  The caller is responsible for scaling to the number-of-shots by passing this to the constructor.
  */
  class Profiler {

    protected:
      /// IR representation of the circuit for profiling
      std::shared_ptr<xacc::CompositeInstruction> placed_circuit_;

    private:
      /// @brief Map from qubit index to single-qubit gates on that qubit wire.
      /// Note that the qubit index can be non-contiguous due to placement,
      /// e.g., a 2-qubit circuit may happen to use qubit indices 20 and 100
      /// due to placement.
      std::unordered_map<int, int> count_1q_gates_on_q_;
      /// Map from qubit index to two-qubit gates on that qubit wire.
      std::unordered_map<int, int> count_2q_gates_on_q_;
      /// Qubit that has the largest gate depth (total of 1 qubit- and 2 qubit-gates).
      int largestdepth_q_;
      /// Number of qubits in the circuit.
      size_t n_qubits_;

      // Timing data
      /// Single-qubit gate time (in milliseconds)
      double gate_1q_time_ms_;
      /// Two-qubit gate time (in milliseconds)
      double gate_2q_time_ms_;
      /// Circuit initialization time before gate execution (in milliseconds)
      double q_initialisation_time_ms_;
      /// Qubit readout/measurement time (in milliseconds)
      double q_readout_time_ms_;
      /// Classical communication overhead w.r.t. remote QPU execution (in milliseconds)
      double pc_send_to_control_time_ms_;

      /// Debugging flag
      bool debug_;

    public:
      /// @brief Constructor (from an OpenQASM string)
      /// @param target_circuit
      /// @param n_qubits
      /// @param gate_1q_time_ms
      /// @param gate_2q_time_ms
      /// @param q_initialisation_time_ms
      /// @param q_readout_time_ms
      /// @param pc_send_to_control_time_ms
      /// @param debug
      Profiler(std::string target_circuit, const int n_qubits,
               const double gate_1q_time_ms = 0.001,
               const double gate_2q_time_ms = 0.001,
               const double q_initialisation_time_ms = 30,
               const double q_readout_time_ms = 10,
               const double pc_send_to_control_time_ms = 10000,
               const bool debug = false);

      /// @brief Constructor (from a XACC IR)
      /// @param f
      /// @param n_qubits
      /// @param gate_1q_time_ms
      /// @param gate_2q_time_ms
      /// @param q_initialisation_time_ms
      /// @param q_readout_time_ms
      /// @param pc_send_to_control_time_ms
      /// @param debug
      Profiler(std::shared_ptr<xacc::CompositeInstruction> &f, const int n_qubits,
               const double gate_1q_time_ms = 0.001,
               const double gate_2q_time_ms = 0.001,
               const double q_initialisation_time_ms = 30,
               const double q_readout_time_ms = 10,
               const double pc_send_to_control_time_ms = 10000,
               const bool debug = false);

      /// @brief Get the total time (i.e., init + gate + readout) for all qubits
      /// @param simulation_total_time
      /// @param shots
      /// @return
      std::map<int,double> get_total_initialisation_maxgate_readout_time_ms(
          const double simulation_total_time = 0.0, const int shots = 1);

      /// @brief Get the count of one-qubit gates for all qubits
      /// @return
      std::map<int,int> get_count_1q_gates_on_q();

      /// @brief  Get the count of two-qubit gates for all qubits
      /// @return
      std::map<int,int> get_count_2q_gates_on_q();

      /// @brief Get the count of one-qubit gates on a qubit line
      /// @param iq
      /// @return
      int get_count_1q_gates_on_q(const int iq);

      /// @brief Get the count of two-qubit gates on a qubit line
      /// Note: a two-qubit gate will be accounted for on both of its qubit operands.
      /// @param iq
      /// @return
      int get_count_2q_gates_on_q(const int iq);

      /// @brief Get Id of the qubit having the most number of gates (largest depth)
      /// @return
      int get_largestdepth_q();

      // Index keys to retrieve profiling results (returned as a (int -> double) map)
      /// Index key for total time
      const int KEY_TOTAL_TIME = 0;
      /// Index key for initialisation time
      const int KEY_INITIALISATION_TIME = 1;
      /// Index key for total gate time on the qubit having the max gate depth
      const int KEY_MAX_DEPTH_GATE_TIME = 2;
      /// Index key for readout time
      const int KEY_READOUT_TIME = 3;
      /// Index key for circuit simulation time
      const int KEY_SIMULATION_TOTAL_TIME = 4;
      /// Index key for communication overhead time
      const int KEY_PC_SEND_TO_CONTROL_TIME = 5;

    private:
      /// @brief Run the profiler (automatically called during Profiler
      /// construction)
      void run();
  };
}
