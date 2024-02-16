// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_SPAMBENCHMARK_
#define _QB_BENCHMARK_SPAMBENCHMARK_

#include "qb/core/benchmark/Serializer.hpp" // contains "qb/core/session.hpp" & typedefs
#include "qb/core/benchmark/Task.hpp"

namespace qb
{

    class CircuitBuilder;

    namespace benchmark
    {
        /**
        * @brief Standard state preparation and measurement (SPAM) workflow for benchmarking
        * 
        * @details This workflow class may be used to execute standard SPAM experiments. It may be used in metric evaluations that require measured/ideal 
        * bit string counts, ideal quantum state densities, and process matrices. Beware that the number of SPAM circuits for n qubits scales exponentially with 2^n! 
        */
        class SPAMBenchmark 
        { 
            public:
                /**
                * @brief Constructor for standard SPAM workflows
                * 
                * Arguments: 
                * @param qubits the indices of the qubits given as std::vector<size_t> to be used in the SPAM experiment
                * @param session a reference to the qb::session where the workflow is supposed to be executed.

                * @return ---
                */
                SPAMBenchmark( const std::vector<size_t>& qubits, qb::session& session ) : 
                    qubits_(qubits), session_(session) {}

                /**
                * @brief Run workflow and store results for specific tasks
                * 
                * Arguments: 
                * @param tasks a selection of Tasks to be executed using the initialized SPAM workflow.
                * 
                * @return std::time_t the time stamp of the successful execution
                * 
                * @details This member function is used to execute specific tasks the SPAM workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution, 
                * (ii) the ideal (noise-free) bit string counts, 
                * (iii) the ideal quantum state densities for each SPAM circuit, 
                * (iv) the ideal quantum process matrices for each SPAM circuit, and 
                * (v) the relevant information contained in the passed qb::session. 
                * Beware that an actual circuit execution is only triggered for task (i). 
                */
                std::time_t execute(const std::vector<Task>& tasks) {
                    return executeWorkflowTasks<SPAMBenchmark>(*this, tasks);
                }
                /**
                * @brief Run workflow and store results for all possible tasks
                * 
                * Arguments: ---
                * 
                * @return std::time_t the time stamp of the successful execution
                * 
                * @details This member function is used to execute all available tasks the SPAM workflow is capable of. These include storing 
                * (i) the measured bit string counts after circuit execution, 
                * (ii) the ideal (noise-free) bit string counts, 
                * (iii) the ideal quantum state densities for each SPAM circuit, 
                * (iv) the ideal quantum process matrices for each SPAM circuit, and 
                * (v) the relevant information contained in the passed qb::session.
                */
                std::time_t execute_all() {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::IdealCounts, Task::IdealDensity, Task::Session});
                    return t;
                }
                
                /**
                * @brief Return a constant reference to the qubit indices of the SPAM workflow
                */
                const std::vector<size_t>& get_qubits() const {return qubits_;}
                /**
                * @brief Return a constant reference to the unique std::string identifier of the SPAM workflow
                */
                const std::string& get_identifier() const {return identifier_;}
                /**
                * @brief Return a constant reference to the assigned qb::session
                */
                const qb::session& get_session() const {return session_;}
                /**
                * @brief Return a reference to the assigned qb::session
                */
                qb::session& set_session() const {return session_;}
                /**
                * @brief Assemble all quantum circuits for the SPAM workflow.
                * 
                * Arguments: ---
                * 
                * @return std::vector<qb::CircuitBuilder> a vector of quantum circuits in the form of qb::CircuitBuilder objects
                * 
                * @details This member function will construct all 2^n SPAM quantum circuits by iterating over all bitsets between 0 and 2^n-1, and adding NOT gates to all "1" bits mapped to the qubit indices (through the qubit vector). 
                */
                std::vector<qb::CircuitBuilder> get_circuits() const; //return full list of SPAM circuits (no measurements!)

                /**
                * @brief Serialization method for measured bit string counts
                * 
                * Arguments: 
                * @param counts the measured bit string counts returned by qb::session 
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_measured_counts(const qb::String& counts, const std::time_t time ) const { 
                    save_data<BitCounts, qb::String>(identifier_, "_measured_", counts, time); 
                }
                /**
                * @brief Serialization method for ideal bit string counts
                * 
                * Arguments: 
                * @param counts the ideal bit string counts
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_ideal_counts(const qb::String& counts, const std::time_t time ) const { 
                    save_data<BitCounts, qb::String>(identifier_, "_ideal_", counts, time); 
                }
                /**
                * @brief Serialization method for ideal quantum state densities
                * 
                * Arguments: 
                * @param densities the ideal quantum state densities
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_ideal_densities( const std::vector<ComplexMatrix>& densities, const std::time_t time ) const { 
                    save_data<ComplexMatrices, std::vector<ComplexMatrix>>(identifier_, "_densities_", densities, time); 
                }
                /**
                * @brief Serialization method for ideal quantum process matrices
                * 
                * Arguments: 
                * @param processes the ideal quantum process matrices
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_ideal_processes( const std::vector<ComplexMatrix>& processes, const std::time_t time ) const {
                    save_data<ComplexMatrices, std::vector<ComplexMatrix>>(identifier_, "_processes_", processes, time);
                }
                /**
                * @brief Serialization method for the assigned qb::session
                * 
                * Arguments: 
                * @param time the time stamp of execution
                * 
                * @return ---
                */
                void serialize_session_infos( const std::time_t time ) const { 
                    save_data<SessionInfo, SessionInfo>(identifier_, "_session_" , session_, time); 
                }

            private:
                const std::vector<std::size_t> qubits_;
                qb::session& session_;

                const std::string identifier_ = "SPAM";
        };
        
        /**
        * @brief Fully specialized execute functor for the Task::IdealCounts task of the SPAMBenchmark workflow.
        */
        template <>
        class executeWorkflowTask<SPAMBenchmark, Task::IdealCounts> {
            public: 
                /**
                * @brief Specialized member function generating and serializing the ideal bit string counts of the SPAMBenchmark workflow.
                * 
                * Arguments: 
                * @param workflow A SPAMBenchmark reference 
                * @param timestamp The time stamp of execution. 
                * 
                * @return ---
                * 
                * @details The ideal counts of each SPAM circuit include only one specific bit string. This specialization, will generate each bit string,
                * produce the corresponding bit string counts as a qb::String objects, and serialize them. 
                */
                void operator()(SPAMBenchmark& workflow, std::time_t timestamp) const;
        };
        /**
        * @brief Fully specialized execute functor for the Task::IdealDensity task of the SPAMBenchmark workflow.
        */
        template <>
        class executeWorkflowTask<SPAMBenchmark, Task::IdealDensity> {
            public: 
                /**
                * @brief Specialized member function generating and serializing the ideal quantum state densities of the SPAMBenchmark workflow.
                * 
                * Arguments: 
                * @param workflow A SPAMBenchmark reference 
                * @param timestamp The time stamp of execution. 
                * 
                * @return ---
                * 
                * @details The ideal quantum state density of each SPAM circuit includes only one non-zero element. This specialization, will generate each bit string,
                * produce the corresponding quantum state densities as ComplexMatrix objects, and serialize them. 
                */
                void operator()(SPAMBenchmark& workflow, std::time_t timestamp) const;
        };
        /**
        * @brief Fully specialized execute functor for the Task::IdealProcess task of the SPAMBenchmark workflow.
        */
        template <>
        class executeWorkflowTask<SPAMBenchmark, Task::IdealProcess> {
            
            public: 
                /**
                * @brief Specialized member function generating and serializing the ideal quantum process matrices of the SPAMBenchmark workflow.
                * 
                * Arguments: 
                * @param workflow A SPAMBenchmark reference 
                * @param timestamp The time stamp of execution. 
                * 
                * @return ---
                * 
                * @details The ideal quantum process matrix of each SPAM circuit includes only one non-zero element. This specialization, will generate each bit string,
                * produce the corresponding quantum process matrices as ComplexMatrix objects, and serialize them. 
                */
                void operator()(SPAMBenchmark& workflow, std::time_t timestamp) const;
        };


    } // namespace qb::benchmark
} // namespace qb

#endif