// Copyright (c) Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_QUANTUMPROCESSMATRIX_
#define _QB_BENCHMARK_QUANTUMPROCESSMATRIX_

// Qristal
#include <qristal/core/benchmark/Serializer.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>

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
        class QuantumProcessMatrixPythonBase {
            public:
                virtual ~QuantumProcessMatrixPythonBase() = default;
                virtual std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const = 0;
        };

        /**
        * @brief Quantum process matrix metric evaluation class templated for arbitrary quantum process tomography workflows @tparam QPTWorkflow.
        *
        * @details This class may be used to evaluate the quantum process matrices for arbitrary templated quantum process
        * tomography workflows @tparam QPTWorkflow. Compatible workflows need to be able to generate and serialize (i) measured
        * bit string counts and (ii) qristal::session information.
        */
        template <QPTWorkflow QPTWORKFLOW>
        requires CanStoreMeasuredCounts<QPTWORKFLOW> && CanStoreSessionInfos<QPTWORKFLOW>
        class QuantumProcessMatrix : public virtual QuantumProcessMatrixPythonBase {
            public:
                /**
                * @brief Constructor for the quantum process matrix metric evaluation class.
                *
                * Arguments:
                * @param workflow the templated quantum process tomography workflow object of type @tparam QPTWorkflow to be evaluated.
                *
                * @return ---
                */
                QuantumProcessMatrix( QPTWORKFLOW & workflow ) : workflow_(workflow) {}

                /**
                * @brief Evaluate the quantum process matrix for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                * @param SPAM_confusion optional SPAM confusion matrix to use in automatic SPAM correction of measured bit string counts.
                *
                * @return std::map<std::time_t, std::vector<ComplexMatrix>> of calculated quantum process matrices mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the quantum process matrices for
                * the already generated results or generate new results to evaluate. Consecutively, the process matrices
                * for all workflow circuits are evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(
                    const bool force_new = false,
                    const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt
                ) const;

            private:

                QPTWORKFLOW& workflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::Session};
        };

        /**
        * @brief The type-erased QuantumProcessMatrix handle exposed in the python bindings.
        */
        class QuantumProcessMatrixPython {
            public:
                //Due to the doubly nested template, it is necessary to implement a new constructor which takes in the python exposed type QuantumProcessTomographyPython
                //This requires a runtime check (via dynamic_cast) for compatible workflows, to construct the right QuantumProcessMatrix objects.
                //To not pollute the standalone header, this was moved to a cpp file.
                QuantumProcessMatrixPython(QuantumProcessTomographyPython& qstpython);

                std::map< std::time_t, std::vector<ComplexMatrix> > evaluate(const bool force_new = false, const std::optional<Eigen::MatrixXd>& SPAM_confusion = std::nullopt) const {
                    return workflow_ptr_->evaluate(force_new, SPAM_confusion);
                }

            private:
                std::unique_ptr<QuantumProcessMatrixPythonBase> workflow_ptr_;
        };

        template <QPTWorkflow QPTWORKFLOW>
        requires CanStoreMeasuredCounts<QPTWORKFLOW> && CanStoreSessionInfos<QPTWORKFLOW>
        std::map<std::time_t, std::vector<ComplexMatrix>> QuantumProcessMatrix<QPTWORKFLOW>::evaluate(const bool force_new, const std::optional<Eigen::MatrixXd>& SPAM_confusion) const {
            std::map<std::time_t, std::vector<ComplexMatrix>> timestamp2processes;
            std::cout << "Evaluating quantum process matrices" << std::endl;

            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(workflow_.get_identifier(), tasks_, force_new);
            dlg.execute(workflow_);

            //(2) obtain session info and measured bitcounts
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts(SPAM_confusion);
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) assemble process matrix for each workflow circuit of each selected timestamp
            std::vector<ComplexMatrix> processes;
            for (const auto & [measured_bitcounts, timestamp] : ::ranges::views::zip(measured_bitcounts_collection, timestamps)) {
                timestamp2processes[timestamp] = workflow_.assemble_processes(workflow_.get_qst().assemble_densities(measured_bitcounts));
            }
            return timestamp2processes;
        }
    }
}

#endif
