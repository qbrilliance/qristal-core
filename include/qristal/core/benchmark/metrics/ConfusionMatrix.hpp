// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// Qristal
#include <qristal/core/benchmark/Serializer.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>
#include <qristal/core/primitives.hpp>

// range v3
#include <range/v3/view/zip.hpp>

namespace qristal
{
    namespace benchmark
    {
        /**
        * @brief Concept for compatible SPAM workflows. Given a list of measurement counts
        * each compatible SPAM workflow needs to be able to construct a confusion matrix
        * through the member function `calculate_confusion_matrix`.
        */
        template <typename T>
        concept CanCalculateConfusionMatrix = requires(const T t, const std::vector<std::map<std::vector<bool>, int>> counts) {
            {t.calculate_confusion_matrix(counts)} -> std::same_as<Eigen::MatrixXd>;
        };

        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class ConfusionMatrixPythonBase {
            public:
                virtual ~ConfusionMatrixPythonBase() = default;
                virtual std::map< std::time_t, Eigen::MatrixXd > evaluate(const bool force_new = false) const = 0;
        };

        /**
        * @brief Confusion matrix metric evaluation class templated for arbitrary @tparam ExecutableWorkflow SPAM workflows.
        *
        * @details This class may be used to evaluate the confusion matrix for arbitrary templated @tparam
        * ExecutableWorkflow workflows. Compatible workflows need to be able to generate and serialize measured
        * bit string counts and qristal::session information and need to provide the member function
        * `calculate_confusion_matrix` to compute the confusion matrix if passed a list of measurement counts.
        */
        template <ExecutableWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW> && CanCalculateConfusionMatrix<WORKFLOW>
        class ConfusionMatrix : public virtual ConfusionMatrixPythonBase {
            public:
                /**
                * @brief Constructor for the confusion matrix metric evaluation class.
                *
                * Arguments:
                * @param workflow : A templated workflow object of type @tparam WORKFLOW to be evaluated.
                *
                * @return ---
                */
                ConfusionMatrix( WORKFLOW & workflow ) : workflow_(workflow) {}

                /**
                * @brief Evaluate the confusion matrix for the given SPAM workflow.
                *
                * Arguments:
                * @param force_new : Optional boolean flag forcing a new execution of the workflow. Defaults to false.
                *
                * @return std::map<std::time_t, Eigen::MatrixXd> : Calculated confusion matrices mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the confusion matrices for
                * the already generated results or generate new results to evaluate. Consecutively, the confusion matrices
                * for all selected SPAM workflows are evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, Eigen::MatrixXd > evaluate(const bool force_new = false) const;

            private:
                WORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::Session};
        };

        /**
        * @brief The type-erased ConfusionMatrix handle exposed in the python bindings.
        */
        class ConfusionMatrixPython {
            public:
                template <ExecutableWorkflow WORKFLOW>
                requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW> && CanCalculateConfusionMatrix<WORKFLOW>
                ConfusionMatrixPython(WORKFLOW& workflow) : workflow_ptr_(std::make_unique<ConfusionMatrix<WORKFLOW>>(workflow)) {}

                std::map< std::time_t, Eigen::MatrixXd> evaluate(const bool force_new = false) const {
                    return workflow_ptr_->evaluate(force_new);
                }

            private:
                std::unique_ptr<ConfusionMatrixPythonBase> workflow_ptr_;
        };

        template <ExecutableWorkflow WORKFLOW>
        requires CanStoreMeasuredCounts<WORKFLOW> && CanStoreSessionInfos<WORKFLOW> && CanCalculateConfusionMatrix<WORKFLOW>
        std::map< std::time_t, Eigen::MatrixXd > ConfusionMatrix<WORKFLOW>::evaluate(const bool force_new) const {
            std::map< std::time_t, Eigen::MatrixXd > timestamp2confusion;
            std::cout << "Evaluating confusion matrices" << std::endl;

            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new);
            dlg.execute(workflow_);

            //(2) obtain measured bitcounts and timestamps
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts();
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) evaluate confusion matrix for every timestamp
            for (const auto & [measured_bitcounts, timestamp] : ::ranges::views::zip(measured_bitcounts_collection, timestamps)) {
                timestamp2confusion.insert(std::make_pair(timestamp, workflow_.calculate_confusion_matrix(measured_bitcounts)));
            }
            return timestamp2confusion;
        }
    }
}
