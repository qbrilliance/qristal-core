// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <qristal/core/benchmark/Task.hpp>

namespace qristal
{
    namespace benchmark
    {
        template <typename WORKFLOW>
        concept CircuitConstructingWorkflow = requires( WORKFLOW w ) {
            {w.get_circuits()} -> std::same_as<std::vector<qristal::CircuitBuilder>>;
        };

        /**
        * @brief Concept for the bare minimum executable workflow usable in qristal::benchmark
        *
        * @details Each metric usable in qb::benchmark will require an executable workflow. The latter should contain member functions to
        * -> execute a set of Task's given as std::vector<Task>
        */
        template <typename WORKFLOW>
        concept ExecutableWorkflow = requires( WORKFLOW w, std::vector<Task> tasks ) {
            {w.execute(tasks)} -> std::same_as<std::time_t>;
        };

        /**
        * @brief Concept for the bare minimum quantum state tomography workflow usable in qristal::benchmark
        *
        * @details Each quantum state tomography protocol usable within qristal::benchmark must be able to
        * assemble quantum state densities (as std::vector<ComplexMatrix>) given a collection of measured bit strings.
        */
        template <typename QST>
        concept QSTWorkflow = requires( QST qst, const std::vector<std::map<std::vector<bool>, int>>& bitstrings ) {
            {qst.assemble_densities(bitstrings)} -> std::same_as<std::vector<ComplexMatrix>>;
        };

        /**
        * @brief Concept for the bare minimum quantum process tomography workflow usable in qristal::benchmark
        *
        * @details Each quantum process tomography protocol usable within qristal::benchmark must be able to assemble quantum
        * process matrices (as std::vector<ComplexMatrix>) given a collection of quantum state densities.
        */
        template <typename QPT>
        concept QPTWorkflow = requires( QPT qpt, const std::vector<ComplexMatrix>& densities ) {
            {qpt.assemble_processes(densities)} -> std::same_as<std::vector<ComplexMatrix>>;
        };

        /**
        * @brief Concept for the bare minimum pyGSTi workflow usable in qristal::benchmark
        *
        * @details Each pyGSTi workflow usable within qristal::benchmark must be able to expose the internal
        * one-line quantum circuit string representation used by pyGSTi.
        */
        template <typename PyGSTiWrapper>
        concept PyGSTiWorkflow = requires(PyGSTiWrapper pygsti) {
            {pygsti.get_pyGSTi_circuit_strings()} -> std::same_as<const std::vector<std::string>&>;
        };

        /**
        * @brief Workflow concept specializing benchmarks that can store circuit information
        *
        * @details Any compatible workflow needs to store circuit information (gate composition, circuit depth, width, etc.) through a call to serialize_circuit_information()
        */
        template <typename T>
        concept CanStoreCircuitInformation = requires(const T t) {
            t.serialize_circuit_information();
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store runtime information
        *
        * @details Any compatible workflow needs to store runtime information (compilation, transpilation, placement, execution, etc. wall time) through a call to serialize_runtime_information()
        */
        template <typename T>
        concept CanStoreRuntimeInformation = requires(const T t) {
            t.serialize_runtime_information();
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store measured bit string counts
        *
        * @details Any compatible workflow needs to store measured bit string counts (the natively measured bit string results obtained directly from qristal::session) through a call to serialize_measured_counts()
        */
        template <typename T>
        concept CanStoreMeasuredCounts = requires(const T t, const std::vector<std::map<std::vector<bool>, int>>& counts, const std::time_t& time) {
            t.serialize_measured_counts(counts, time);
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store ideal bit string counts
        *
        * @details Any compatible workflow needs to store ideal bit string counts (ideal counts obtained from either ideal state vector simulators or from analytical expressions) through a call to serialize_ideal_counts()
        */
        template <typename T>
        concept CanStoreIdealCounts = requires(const T t, const std::vector<std::map<std::vector<bool>, int>>&& counts, const std::time_t& time) {
            t.serialize_ideal_counts(counts, time);
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store ideal quantum state densities
        *
        * @details Any compatible workflow needs to ideal quantum state densities (ideal density matrices obtained from either ideal state vector simulators or from analytical expressions) through a call to serialize_ideal_densities()
        */
        template <typename T>
        concept CanStoreIdealDensities = requires(const T t, const std::vector<ComplexMatrix>& densities, const std::time_t& time) {
            t.serialize_ideal_densities(densities, time);
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store ideal quantum process matrices
        *
        * @details Any compatible workflow needs to store ideal quantum process matrices (ideal process matrices obtained from either ideal state vector simulators or from analytical expressions) through a call to serialize_ideal_processes()
        */
        template <typename T>
        concept CanStoreIdealProcesses = requires(const T t, const std::vector<ComplexMatrix>& processes, const std::time_t& time) {
            t.serialize_ideal_processes(processes, time);
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store application information
        *
        * @details Any compatible workflow needs to store application information (very specialized information for specific applications as, e.g., number of iterations / circuit ansatz / etc.) through a call to serialize_app_information()
        */
        template <typename T>
        concept CanStoreAppInformation = requires(const T t) {
            t.serialize_app_information();
        };
        /**
        * @brief Workflow concept specializing benchmarks that can store session information
        *
        * @details Any compatible workflow needs to store session information (everything contained within qristal::session such as number of qubits, number of shots, noise model, backend, etc.) through a call to serialize_session_infos()
        */
        template <typename T>
        concept CanStoreSessionInfos = requires(const T t, const std::time_t& time) {
            t.serialize_session_infos(time);
        };

    }

}
