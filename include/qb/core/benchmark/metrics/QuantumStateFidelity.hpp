// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_QUANTUMSTATEFIDELITY_
#define _QB_BENCHMARK_QUANTUMSTATEFIDELITY_

#include <unsupported/Eigen/MatrixFunctions>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qb/core/tools/zip_tool.hpp"
#include "qb/core/benchmark/Serializer.hpp"
#include "qb/core/benchmark/DataLoaderGenerator.hpp"
#include "qb/core/benchmark/workflows/QuantumStateTomography.hpp"

namespace qb
{
    namespace benchmark
    {
        /**
        * @brief Quantum state fidelity metric evaluation class templated for arbitrary quantum state tomography workflows @tparam QSTWorkflow.
        *
        * @details This class may be used to evaluate the quantum state fidelity metric for arbitrary templated quantum
        * state tomography workflows @tparam QSTWorkflow. Compatible workflows need to be able to generate and serialize (i) measured
        * bit string counts, (ii) qb::session information, and (iii) need to be wrapped around ExecutableWorkflows that can serialize and generate
        * ideal quantum state densities.
        */
        template <QSTWorkflow QSTWORKFLOW>
        requires CanStoreMeasuredCounts<QSTWORKFLOW> && CanStoreIdealDensities<typename QSTWORKFLOW::ExecutableWorkflowType> && CanStoreSessionInfos<QSTWORKFLOW>
        class QuantumStateFidelity {
            public:
                /**
                * @brief Constructor for the quantum state fidelity metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated workflow object of type @tparam QSTWORKFLOW to evaluated.
                *
                * @return ---
                */
                QuantumStateFidelity( QSTWORKFLOW & qstworkflow) : qstworkflow_(qstworkflow) {}
                /**
                * @brief Evaluate the quantum state fidelity for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                *
                * @return std::map<std::time_t, std::vector<double>> of calculated quantum state fidelities mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the quantum state fidelities for
                * the already generated results or generate new results to evaluate. Consecutively, the quantum state fidelities
                * for all workflow circuits are evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, std::vector<double> > evaluate(const bool force_new = false);

            private:
                QSTWORKFLOW& qstworkflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::IdealDensity, Task::Session};
        };

        /**
        * @brief Helper function to evalute the state fidelity of two given complex-valued density matrices.
        *
        * Arguments:
        * @param a left complex density matrix of type Eigen::Matrix
        * @param b right complex density matrix of type Eigen::Matrix.
        *
        * @return double The quantum state fidelity f(a,b).
        *
        * @details This function evaluates the quantum state fidelity f(a,b) = trace(sqrt(sqrt(a) * b * sqrt(a)))^2
        * for complex density matrices a and b..
        */
        double calculate_state_fidelity(const ComplexMatrix& a, const ComplexMatrix& b)
        {
            return std::pow(std::abs(((a.sqrt() * b * a.sqrt()).sqrt()).trace()), 2);
        }


        template <QSTWorkflow QSTWORKFLOW>
        requires CanStoreMeasuredCounts<QSTWORKFLOW> && CanStoreIdealDensities<typename QSTWORKFLOW::ExecutableWorkflowType> && CanStoreSessionInfos<QSTWORKFLOW>
        std::map<std::time_t, std::vector<double>> QuantumStateFidelity<QSTWORKFLOW>::evaluate(const bool force_new) {
            std::map<std::time_t, std::vector<double>> timestamp2fidelities;
            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(qstworkflow_.get_identifier(), tasks_, force_new);
            dlg.execute(qstworkflow_);

            //(2) obtain session info, ideal, and measured bitcounts
            std::vector<SessionInfo> session_infos = dlg.obtain_session_infos();
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts();
            std::vector<std::vector<ComplexMatrix>> ideal_densities_collection = dlg.obtain_ideal_densities();
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) evaluate state fidelity for each circuit in each timestamp
            for (const auto & [measured_bitcounts, ideal_densities, timestamp] : std::ranges::views::zip(measured_bitcounts_collection, ideal_densities_collection, timestamps)) {
                std::vector<double> fidelities;
                std::vector<ComplexMatrix> measured_densities = qstworkflow_.assemble_densities(measured_bitcounts);
                for (const auto & [measured_density, ideal_density] : std::ranges::views::zip(measured_densities, ideal_densities)) {
                    fidelities.push_back(calculate_state_fidelity(measured_density, ideal_density));
                }
                timestamp2fidelities.insert(std::make_pair(timestamp, fidelities));
            }
            return timestamp2fidelities;
        }
    }
}

#endif
