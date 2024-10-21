// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#pragma once

#include "qristal/core/benchmark/Serializer.hpp" // contains "qristal/core/session.hpp" & typedefs
#include "qristal/core/benchmark/Task.hpp"

namespace qristal
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
                * @param qubits the indices of the qubits given as std::set<size_t> to be used in the SPAM experiment
                * @param session a reference to the qristal::session where the workflow is supposed to be executed.

                * @return ---
                */
                SPAMBenchmark( const std::set<size_t>& qubits, qristal::session& session ) :
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
                * (v) the relevant information contained in the passed qristal::session.
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
                * (v) the relevant information contained in the passed qristal::session.
                */
                std::time_t execute_all() {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::IdealCounts, Task::IdealDensity, Task::IdealProcess, Task::Session});
                    return t;
                }

                /**
                * @brief Return a constant reference to the qubit indices of the SPAM workflow
                */
                const std::set<size_t>& get_qubits() const {return qubits_;}
                /**
                * @brief Return a constant reference to the unique std::string identifier of the SPAM workflow
                */
                const std::string& get_identifier() const {return identifier_;}
                /**
                * @brief Return a constant reference to the assigned qristal::session
                */
                const qristal::session& get_session() const {return session_;}
                /**
                * @brief Return a reference to the assigned qristal::session
                */
                qristal::session& set_session() const {return session_;}
                /**
                * @brief Assemble all quantum circuits for the SPAM workflow.
                *
                * Arguments: ---
                *
                * @return std::vector<qristal::CircuitBuilder> a vector of quantum circuits in the form of qristal::CircuitBuilder objects
                *
                * @details This member function will construct all 2^n SPAM quantum circuits by iterating over all bitsets between 0 and 2^n-1, and adding NOT gates to all "1" bits mapped to the qubit indices (through the qubit vector).
                */
                std::vector<qristal::CircuitBuilder> get_circuits() const; //return full list of SPAM circuits (no measurements!)

                /**
                * @brief Helper function to compute the confusion matrix for the given SPAM workflow.
                *
                * Arguments:
                * @param counts : The measured counts for all SPAM circuits. 
                *
                * @return Eigen::MatrixXd : The assembled confusion matrix.
                */
                Eigen::MatrixXd calculate_confusion_matrix(const std::vector<std::map<std::vector<bool>, int>>& counts) const;

                /**
                * @brief Serialization method for measured bit string counts
                *
                * Arguments:
                * @param counts the measured bit string counts returned by qristal::session
                * @param time the time stamp of execution
                *
                * @return ---
                */
                void serialize_measured_counts(const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t time ) const {
                    save_data<BitCounts, std::vector<std::map<std::vector<bool>, int>>>(identifier_, "_measured_", counts, time);
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
                void serialize_ideal_counts(const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t time ) const {
                    save_data<BitCounts, std::vector<std::map<std::vector<bool>, int>>>(identifier_, "_ideal_", counts, time);
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
                * @brief Serialization method for the assigned qristal::session
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
                const std::set<std::size_t> qubits_;
                qristal::session& session_;
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
                * produce the corresponding bit string counts as a std::vector<std::string> object, and serialize them.
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


    }
}
