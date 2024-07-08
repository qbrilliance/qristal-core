// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#include <cmath>
#include <boost/dynamic_bitset.hpp>
#include "qb/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qb/core/circuit_builder.hpp"

namespace qb
{
    namespace benchmark
    {

        std::vector<qb::CircuitBuilder> SPAMBenchmark::get_circuits() const
        {
            std::vector<qb::CircuitBuilder> circuits;
            const size_t N = std::pow(2, qubits_.size());
            for ( std::size_t bitstring = 0; bitstring < N; ++bitstring ) 
            {
                boost::dynamic_bitset<> current(qubits_.size(), bitstring);
                qb::CircuitBuilder cb;
                for ( std::size_t i = 0; i < current.size(); ++i ) {
                    if ( current[i] ) {
                        cb.X(qubits_[qubits_.size() - 1 - i]); //boost::dynamic_bitset is uses right to left ordering!
                    }
                }
                circuits.push_back(cb);
            }

            return circuits;
        }

        void executeWorkflowTask<SPAMBenchmark, Task::IdealCounts>::operator()(SPAMBenchmark & workflow, std::time_t timestamp) const {
            std::vector<std::string> ideal_results;
            size_t n_bitstrings = std::pow(2, workflow.get_qubits().size());
            for (size_t bitstring = 0; bitstring < n_bitstrings; ++bitstring )
            {
                boost::dynamic_bitset<> b(2, bitstring);
                std::string s; 
                boost::to_string(b, s);
                std::stringstream ss; 
                ss << "\"" << s << "\":  " << workflow.get_session().get_sns()[0][0];
                ideal_results.push_back(ss.str());
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
                std::string s; 
                boost::to_string(b, s);
                //iterate through all bits and calculate the index of operator string corresponding to the bit string 
                //00 -> II -> 0*4 + 0 = 0
                //01 -> IX -> 0*4 + 1 = 1
                //10 -> XI -> 1*4 + 0 = 4
                //11 -> XX -> 1*4 + 1 = 5
                //e.g., 10010 would be XIIXI would be 4^4 + 4^1 = 260
                size_t index = 0;
                for (size_t i = 0; i < s.size(); ++i) {
                    if (s[i] == '1') {
                        index += std::pow(4, i); //boost dynamic_bitset uses right to left ordering!
                    }
                }
                ideal_process(index, index) = 1.0;
                ideal_processes.push_back(ideal_process);
            }
            workflow.serialize_ideal_processes(ideal_processes, timestamp);
        }

    } // namespace qb::benchmark
} // namespace qb
