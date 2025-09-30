// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

// Qristal
#include <qristal/core/benchmark/Serializer.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>

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
        class BitstringCountsPythonBase {
            public:
                virtual ~BitstringCountsPythonBase() = default;
                virtual std::map< std::time_t, std::vector<std::map<std::vector<bool>, int>> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const = 0;
        };

        /**
        * @brief Bitstring counts evaluation class templated for arbitrary executable workflows.
        */
        template <ExecutableWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
        class BitstringCounts : public virtual BitstringCountsPythonBase {
            public:
                /**
                * @brief Constructor for the bitstring counts metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated executable workflow object of type @tparam WORKFLOW to be evaluated.
                *
                * @return ---
                */
                BitstringCounts( WORKFLOW & workflow ) : workflow_(workflow) {}

                /**
                * @brief Evaluate the bitstring counts for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                * @param SPAM_confusion optional SPAM confusion matrix to use in automatic SPAM correction of measured bit string counts.
                *
                * @return std::map<std::time_t, std::vector<std::map<std::vector<bool>, int>>> of measured bitstring counts mapped to the corresponding time stamp of the workflow execution.
                */
                std::map< std::time_t, std::vector<std::map<std::vector<bool>, int>> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const {
                    std::map<std::time_t, std::vector<std::map<std::vector<bool>, int>>> timestamp2counts;
                    std::cout << "Evaluating measured bitstring counts" << std::endl;

                    //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
                    DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new);
                    dlg.execute(workflow_);

                    //(2) obtain session info and measured bitcounts
                    std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts(SPAM_confusion);
                    std::vector<std::time_t> timestamps = dlg.get_timestamps();

                    //(3) tie measured bitstring counts for each workflow circuit to each selected timestamp
                    for (const auto & [measured_bitcounts, timestamp] : ::ranges::views::zip(measured_bitcounts_collection, timestamps)) {
                        timestamp2counts[timestamp] = measured_bitcounts;
                    }
                    return timestamp2counts;
                }

            private:
                WORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::Session};
        };

        /**
        * @brief The type-erased BitstringCounts handle exposed in the python bindings.
        */
        class QuantumStateTomographyPython;
        class QuantumProcessTomographyPython;
        class PreOrAppendWorkflowPython;
        class AddinFromIdealSimulationPython;
        class BitstringCountsPython {
            public:
                template <ExecutableWorkflow WORKFLOW>
                requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW>
                BitstringCountsPython(WORKFLOW& workflow) : workflow_ptr_(std::make_unique<BitstringCounts<WORKFLOW>>(workflow)) {}

                //Special overloads for PreOrAppend, QST and QPT workflows
                BitstringCountsPython(PreOrAppendWorkflowPython& workflow);
                BitstringCountsPython(QuantumStateTomographyPython& qstpython);
                BitstringCountsPython(QuantumProcessTomographyPython& qstpython);

                std::map< std::time_t, std::vector<std::map<std::vector<bool>, int>> > evaluate(const bool force_new = false, const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt) const;

            private:
                std::unique_ptr<BitstringCountsPythonBase> workflow_ptr_;
        };
    }
}
