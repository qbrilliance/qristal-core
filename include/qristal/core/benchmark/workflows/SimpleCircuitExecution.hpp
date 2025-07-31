// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <qristal/core/benchmark/Serializer.hpp> // contains <qristal/core/session.hpp> & typedefs
#include <qristal/core/benchmark/Task.hpp>

namespace qristal
{

    class CircuitBuilder;

    namespace benchmark
    {
        /**
        * @brief Simple wrapper to connect simple circuit executions with qristal::benchmark functionalities.
        *
        * @details This workflow class may be used to execute simple quantum circuit experiments. It may be used in metric evaluations that require measured
        * bit string counts only. Please provide custom executors for ideal densities/processes if you wish to calculate fidelities.
        */
        class SimpleCircuitExecution {
            public:
                /**
                * @brief Constructor for the simple circuit execution wrapper
                *
                * Arguments:
                * @param circuits the quantum circuits to be executed given as std::vector<qristal::CircuitBuilder>
                * @param session a reference to the qristal::session where the workflow is supposed to be executed.

                * @return ---
                */
                SimpleCircuitExecution(const std::vector<qristal::CircuitBuilder>& circuits, qristal::session& session) :
                    circuits_(circuits), session_(session) {}

                /**
                * @brief Constructor overload for single circuits
                *
                * Arguments:
                * @param circuit the quantum circuit to be executed given as std::vector<qristal::CircuitBuilder>
                * @param session a reference to the qristal::session where the workflow is supposed to be executed.

                * @return ---
                */
                SimpleCircuitExecution(const qristal::CircuitBuilder& circuit, qristal::session& session) :
                    circuits_(std::vector<qristal::CircuitBuilder>{circuit}), session_(session) {}

                /**
                * @brief Run workflow and store results for specific tasks
                *
                * Arguments:
                * @param tasks a selection of Tasks to be executed using the initialized SPAM workflow.
                *
                * @return std::time_t the time stamp of the successful execution
                *
                * @details This member function is used to execute specific tasks the SimpleCircuitExecution workflow is capable of. These include storing
                * (i) the measured bit string counts after circuit execution, and
                * (ii) the relevant information contained in the passed qristal::session.
                * For additional functionality, please provide specializations for executeWorkflowTask<SimpleCircuitExecution, Task>.
                */
                std::time_t execute(const std::vector<Task>& tasks) {
                    return executeWorkflowTasks<SimpleCircuitExecution>(*this, tasks);
                }

                /**
                * @brief Run workflow and store results for all possible tasks
                *
                * Arguments: ---
                *
                * @return std::time_t the time stamp of the successful execution
                *
                * @details This member function is used to execute all available tasks the SimpleCircuitExecution workflow is capable of. These include storing
                * (i) the measured bit string counts after circuit execution, and
                * (ii) the relevant information contained in the passed qristal::session.
                */
                std::time_t execute_all() {
                    std::time_t t = execute(std::vector<Task>{Task::MeasureCounts, Task::Session});
                    return t;
                }

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
                * @brief Return a copy of the wrapped circuits in this class.
                *
                * Arguments: ---
                *
                * @return std::vector<qristal::CircuitBuilder> a vector of quantum circuits in the form of qristal::CircuitBuilder objects
                */
                std::vector<qristal::CircuitBuilder> get_circuits() const {
                    return circuits_;
                }

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

                std::vector<qristal::CircuitBuilder> circuits_;
                qristal::session& session_;

                const std::string identifier_ = "SimpleCircuitExecution";
        };

    }
}
