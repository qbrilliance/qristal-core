// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <qristal/core/benchmark/Serializer.hpp> // contains "qristal/core/session.hpp> & typedefs
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/benchmark/Task.hpp>


namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Rotation sweep workflow for standard Pauli rotation gates
        *
        * @details This workflow class may be used to execute rotation sweeps experiments for standard Pauli (X, Y, Z) rotations. It may be used in metric evaluations that require measured/ideal
        * bit string counts, ideal quantum state densities, and process matrices. Beware that multiple qubit rotation sweeps will be executed in parallel (and not consecutively)!
        */
        class RotationSweep
        {
            public:
                /**
                * @brief Constructor for rotation sweep workflows
                *
                * Arguments:
                * @param rotations_per_qubit an std::vector<char> for each qubit, where "X", "Y", "Z" (or "I") are used to indicate which rotation is applied
                * @param start_degree integer starting degree for the rotation sweep
                * @param end_degree integer final degree for the rotation sweep
                * @param n_points positive number of points (aka number of total circuits) included in the sweep between start_degree and end_degree
                * @param session a reference to the qristal::session where the workflow is supposed to be executed.

                * @return ---
                */
                RotationSweep( const std::vector<char>& rotations_per_qubit, //either X, Y, Z or I (do nothing) for each qubit
                               const int& start_degree,
                               const int& end_degree,
                               const size_t& n_points, //number of data points to collect for each sweep
                               qristal::session& session );

                /**
                * @brief Run workflow and store results for specific tasks
                *
                * Arguments:
                * @param tasks a selection of Tasks to be executed using the initialized rotation sweep workflow.
                *
                * @return std::time_t the time stamp of the successful execution
                *
                * @details This member function is used to execute specific tasks the rotation sweep workflow is capable of. These include storing
                * (i) the measured bit string counts after circuit execution,
                * (ii) the ideal (noise-free) bit string counts,
                * (iii) the ideal quantum state densities for each quantum circuit,
                * (iv) the ideal quantum process matrices for each quantum circuit, and
                * (v) the relevant information contained in the passed qristal::session.
                * Beware that an actual circuit execution is only triggered for task (i).
                */
                std::time_t execute(const std::vector<Task>& tasks) {
                    return executeWorkflowTasks<RotationSweep>(*this, tasks);
                }
                /**
                * @brief Run workflow and store results for all possible tasks
                *
                * Arguments: ---
                *
                * @return std::time_t the time stamp of the successful execution
                *
                * @details This member function is used to execute all available tasks the rotation sweep workflow is capable of. These include storing
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
                * @brief Return a constant reference to the assigned qristal::session
                */
                const qristal::session& get_session() const {return session_;}
                /**
                * @brief Return a reference to the assigned qristal::session
                */
                qristal::session& set_session() const {return session_;}

                /**
                * @brief Assemble all quantum circuits for the rotation sweep workflow.
                *
                * Arguments: ---
                *
                * @return std::vector<qristal::CircuitBuilder> a vector of quantum circuits in the form of qristal::CircuitBuilder objects
                *
                * @details This member function will iterate over all n_points_ steps from start_degree_ to end_degree_ and add rotation gates on all qubits according to rotations_per_qubit_ with the corresponding angles.
                */
                std::vector<qristal::CircuitBuilder> get_circuits() const; //return full list of RotationSweep circuits (no measurements!)
                /**
                * @brief Return a constant reference to the unique std::string identifier of the rotation sweep workflow
                */
                const std::string& get_identifier() const {return identifier_;}


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
                    save_data<BitCounts, std::vector<std::map<std::vector<bool>, int>>>(identifier_,  "_measured_", counts, time);
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
                void serialize_ideal_processes( const std::vector<ComplexMatrix>& superoperators, const std::time_t time ) const {
                    save_data<ComplexMatrices, std::vector<ComplexMatrix>>(identifier_, "_processes_", superoperators, time);
                }
                /**
                * @brief Serialization method for the assigned qristal::session
                *
                * Arguments:
                * @param time the time stamp of execution
                *
                * @return ---
                */
                void serialize_session_infos(const std::time_t time ) const {
                    save_data<SessionInfo, SessionInfo>(identifier_, "_session_" , session_, time);
                }

                /**
                * @brief Convert start_degree_ to radian and return.
                */
                double start_rad() const {
                    return static_cast<double>(start_degree_) / 180.0 * std::numbers::pi;
                }
                /**
                * @brief Convert end_degree_ to radian and return.
                */
                double end_rad() const {
                    return static_cast<double>(end_degree_) / 180.0 * std::numbers::pi;
                }
                /**
                * @brief Based on n_points_, calculate a single radian step of the rotation sweep and return.
                */
                double step() const {
                    return (end_rad() - start_rad()) / (n_points_ - 1);
                }
                /**
                * @brief Return a constant reference to the set rotations_per_qubit_ vector.
                */
                const std::vector<char>& get_rotations_per_qubit() const {return rotations_per_qubit_;}

            private:
                const std::vector<char> rotations_per_qubit_;
                const int start_degree_;
                const int end_degree_;
                const size_t n_points_;
                qristal::session& session_;

                const std::string identifier_ = "RotationSweep";
        };

        /**
        * @brief Fully specialized execute functor for the Task::IdealCounts task of the RotationSweep workflow.
        */
        template <>
        class executeWorkflowTask<RotationSweep, Task::IdealCounts> {
            public:
                /**
                * @brief Specialized member function generating and serializing the ideal bit string counts of the RotationSweep workflow.
                *
                * Arguments:
                * @param workflow A RotationSweep reference
                * @param timestamp The time stamp of execution.
                *
                * @return ---
                *
                * @details This member function will iterate over all rotation angles part of the sweep, calculate the exact bit string probabilies,
                * convert them to the expected ideal counts, and serialize them.
                */
                void operator()(RotationSweep& workflow, std::time_t timestamp) const;
        };
        /**
        * @brief Fully specialized execute functor for the Task::IdealDensity task of the RotationSweep workflow.
        */
        template <>
        class executeWorkflowTask<RotationSweep, Task::IdealDensity> {
            public:
                /**
                * @brief Specialized member function generating and serializing the ideal quantum state densities of the RotationSweep workflow.
                *
                * Arguments:
                * @param workflow A RotationSweep reference
                * @param timestamp The time stamp of execution.
                *
                * @return ---
                *
                * @details This member function will iterate over all rotation angles part of the sweep, calculate the exact quantum state densities,
                * and serialize them.
                */
                void operator()(RotationSweep& workflow, std::time_t timestamp) const;
        };
        /**
        * @brief Fully specialized execute functor for the Task::IdealProcess task of the SPAMBenchmark workflow.
        */
        template <>
        class executeWorkflowTask<RotationSweep, Task::IdealProcess> {

            public:
                /**
                * @brief Specialized member function generating and serializing the ideal quantum process matrices of the RotationSweep workflow.
                *
                * Arguments:
                * @param workflow A RotationSweep reference
                * @param timestamp The time stamp of execution.
                *
                * @return ---
                *
                * @details This member function will iterate over all rotation angles part of the sweep, calculate the exact quantum process matrices,
                * and serialize them.
                */
                void operator()(RotationSweep& workflow, std::time_t timestamp) const;
        };

    }
}
