// Copyright (c) Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_QUANTUMSTATEDENSITY_
#define _QB_BENCHMARK_QUANTUMSTATEDENSITY_

// Qristal
#include <qristal/core/benchmark/Serializer.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>

// STL
#include <ranges>

// range v3
#include <range/v3/view/zip.hpp>

namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class QuantumStateDensityPythonBase {
            public:
                virtual ~QuantumStateDensityPythonBase() = default;
                virtual std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const = 0;
        };

        /**
        * @brief Quantum state density metric evaluation class templated for arbitrary quantum state tomography workflows @tparam QSTWorkflow.
        *
        * @details This class may be used to evaluate the quantum state densities for arbitrary templated quantum state
        * tomography workflows @tparam QSTWorkflow. Compatible workflows need to be able to generate and serialize (i) measured
        * bit string counts and (ii) qristal::session information.
        */
        template <QSTWorkflow QSTWORKFLOW>
        requires CanStoreMeasuredCounts<QSTWORKFLOW> && CanStoreSessionInfos<QSTWORKFLOW>
        class QuantumStateDensity : public virtual QuantumStateDensityPythonBase {
            public:
                /**
                * @brief Constructor for the quantum state density metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated quantum state tomography workflow object of type @tparam QSTWorkflow to be evaluated.
                *
                * @return ---
                */
                QuantumStateDensity( QSTWORKFLOW & workflow ) : workflow_(workflow) {}

                /**
                * @brief Evaluate the quantum state density for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                * @param SPAM_confusion optional SPAM confusion matrix to use in automatic SPAM correction of measured bit string counts.
                *
                * @return std::map<std::time_t, std::vector<ComplexMatrix>> of calculated quantum state densities mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the quantum state densities for
                * the already generated results or generate new results to evaluate. Consecutively, the state densities
                * for all workflow circuits are evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const;

            private:

                QSTWORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::Session};
        };

        /**
        * @brief The type-erased QuantumStateDensity handle exposed in the python bindings.
        */
        class QuantumStateDensityPython {
            public:
                //Due to the doubly nested template, it is necessary to implement a new constructor which takes in the python exposed type QuantumStateTomographyPython
                //This requires a runtime check (via dynamic_cast) for compatible workflows, to construct the right QuantumProcessTomography objects.
                //To not pollute the standalone header, this was moved to a cpp file.
                QuantumStateDensityPython(QuantumStateTomographyPython& qstpython);

                std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(const bool force_new = false, const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt) const {
                    return workflow_ptr_->evaluate(force_new, SPAM_confusion);
                }

            private:
                std::unique_ptr<QuantumStateDensityPythonBase> workflow_ptr_;
        };

        template <QSTWorkflow QSTWORKFLOW>
        requires CanStoreMeasuredCounts<QSTWORKFLOW> && CanStoreSessionInfos<QSTWORKFLOW>
        std::map<std::time_t, std::vector<ComplexMatrix>> QuantumStateDensity<QSTWORKFLOW>::evaluate(const bool force_new, const std::optional<Eigen::MatrixXd>& SPAM_confusion) const {
            std::map<std::time_t, std::vector<ComplexMatrix>> timestamp2densities;
            std::cout << "Evaluating quantum state densities" << std::endl;

            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new);
            dlg.execute(workflow_);

            //(2) obtain session info and measured bitcounts
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts(SPAM_confusion);
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) assemble density matrix for each workflow circuit of each selected timestamp
            std::vector<ComplexMatrix> densities;
            for (const auto & [measured_bitcounts, timestamp] : ::ranges::views::zip(measured_bitcounts_collection, timestamps)) {
                timestamp2densities[timestamp] = workflow_.assemble_densities(measured_bitcounts);
            }
            return timestamp2densities;
        }
    }
}

#endif
