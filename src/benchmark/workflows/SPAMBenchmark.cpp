// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#include <cmath>
#include <boost/dynamic_bitset.hpp>
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/circuit_builder.hpp"

namespace qristal
{
    namespace benchmark
    {

        std::vector<qristal::CircuitBuilder> SPAMBenchmark::get_circuits() const
        {
            std::vector<qristal::CircuitBuilder> circuits;
            const size_t N = std::pow(2, qubits_.size());
            for ( std::size_t bitstring = 0; bitstring < N; ++bitstring )
            {
                boost::dynamic_bitset<> current(qubits_.size(), bitstring);
                qristal::CircuitBuilder cb;
                auto it = qubits_.begin();
                for ( std::size_t i = 0; i < current.size(); ++i ) {
                    if ( current[i] ) {
                        cb.X(*it);
                    }
                    ++it;
                }
                circuits.push_back(cb);
            }

            return circuits;
        }

        void executeWorkflowTask<SPAMBenchmark, Task::IdealCounts>::operator()(SPAMBenchmark & workflow, std::time_t timestamp) const {
            std::vector<std::map<std::vector<bool>, int>> ideal_results;
            size_t n_bitstrings = std::pow(2, workflow.get_qubits().size());
            for (size_t bitstring = 0; bitstring < n_bitstrings; ++bitstring )
            {
                boost::dynamic_bitset<> b(workflow.get_qubits().size(), bitstring);
                //convert boost::dynamic_bitset to std::vector<bool>
                std::vector<bool> bvec(b.size());
                for (size_t i = 0; i < b.size(); ++i) {
                    bvec[i] = b[i];
                }
                const std::map<std::vector<bool>, int> m = {{ bvec, workflow.get_session().get_sns()[0][0] }};
                ideal_results.push_back(m);
            }
            workflow.serialize_ideal_counts(ideal_results, timestamp);
        }

        void executeWorkflowTask<SPAMBenchmark, Task::IdealDensity>::operator()(SPAMBenchmark & workflow, std::time_t timestamp) const {
            std::vector<ComplexMatrix> ideal_densities;
            size_t n_bitstrings = std::pow(2, workflow.get_qubits().size());
            for (size_t bitstring = 0; bitstring < n_bitstrings; ++bitstring ) {
                ComplexMatrix ideal_density = ComplexMatrix::Zero(n_bitstrings, n_bitstrings);
                ideal_density(bitstring, bitstring) = std::complex<double>(1.0, 0.0);
                ideal_densities.push_back(ideal_density);
            }
            workflow.serialize_ideal_densities(ideal_densities, timestamp);
        }

        void executeWorkflowTask<SPAMBenchmark, Task::IdealProcess>::operator()(SPAMBenchmark & workflow, std::time_t timestamp) const {
            std::vector<ComplexMatrix> ideal_processes;
            size_t n_elems = std::pow(4, workflow.get_qubits().size());
            size_t n_bitstrings = std::pow(2, workflow.get_qubits().size());
            for (size_t bitstring = 0; bitstring < n_bitstrings; ++bitstring) {
                ComplexMatrix ideal_process = ComplexMatrix::Zero(n_elems, n_elems);
                //convert size_t to bitset to string
                boost::dynamic_bitset<> b(workflow.get_qubits().size(), bitstring);
                //iterate through all bits and calculate the index of operator string corresponding to the bit string
                //00 -> II -> 0*4 + 0 = 0
                //01 -> IX -> 0*4 + 1 = 1
                //10 -> XI -> 1*4 + 0 = 4
                //11 -> XX -> 1*4 + 1 = 5
                //e.g., 10010 would be XIIXI would be 4^4 + 4^1 = 260
                size_t index = 0;
                for (size_t i = 0; i < b.size(); i++) {
                    if (b[i]) index += std::pow(4, i);
                }
                ideal_process(index, index) = 1.0;
                ideal_processes.push_back(ideal_process);
            }
            workflow.serialize_ideal_processes(ideal_processes, timestamp);
        }

    }
}
