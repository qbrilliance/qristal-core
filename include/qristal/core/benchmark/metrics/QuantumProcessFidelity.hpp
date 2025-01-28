// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#ifndef _QB_BENCHMARK_QUANTUMPROCESSFIDELITY_
#define _QB_BENCHMARK_QUANTUMPROCESSFIDELITY_

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"
#include "qristal/core/benchmark/Serializer.hpp"
#include "qristal/core/benchmark/DataLoaderGenerator.hpp"
#include "qristal/core/benchmark/workflows/QuantumProcessTomography.hpp"

namespace qristal
{
    namespace benchmark
    {

        /**
        * @brief Pure virtual python bindings helper class not used in the C++ implementation.
        */
        class QuantumProcessFidelityPythonBase {
            public: 
                virtual ~QuantumProcessFidelityPythonBase() = default; 
                virtual std::map< std::time_t, std::vector<double> >  evaluate(const bool force_new = false) const = 0;
        };

        /**
        * @brief Quantum process fidelity metric evaluation class templated for arbitrary quantum process tomography workflows @tparam QPTWorkflow.
        *
        * @details This class may be used to evaluate the quantum process fidelity metric for arbitrary templated  quantum process tomogrpahy
        * @tparam QPTWORKFLOW workflows. Compatible workflows need to be able to generate and serialize (i) measured
        * bit string counts, (ii) qristal::session information, and (iii) need to be wrapped around ExecutableWorkflows that can serialize and generate
        * ideal quantum process matrices.
        */
        template <QPTWorkflow QPTWORKFLOW>
        requires CanStoreMeasuredCounts<QPTWORKFLOW> && CanStoreIdealProcesses<typename QPTWORKFLOW::QSTWorkflowType::ExecutableWorkflowType> && CanStoreSessionInfos<QPTWORKFLOW>
        class QuantumProcessFidelity : public virtual QuantumProcessFidelityPythonBase {
            public:
                /**
                * @brief Constructor for the quantum process fidelity metric evaluation class.
                *
                * Arguments:
                * @param qptworkflow the templated quantum process tomography workflow object of type @tparam QPTWORKFLOW to evaluated.
                *
                * @return ---
                */
                QuantumProcessFidelity( QPTWORKFLOW & qptworkflow ) : qptworkflow_(qptworkflow) {}
                /**
                * @brief Evaluate the quantum process fidelity for the given workflow.
                *
                * Arguments:
                * @param force_new optional boolean flag forcing a new execution of the workflow. Defaults to false.
                *
                * @return std::map<std::time_t, std::vector<double>> calculated quantum state fidelities mapped to the corresponding time stamp of the workflow execution.
                *
                * @details This member function initializes and utilizes a DataLoaderGenerator object to find already
                * serialized workflow execution results. The user may then choose to evaluate the quantum process fidelities for
                * the already generated results or generate new results to evaluate. Consecutively, the quantum process fidelities
                * for all workflow circuits are evaluated and returned in a std::map to the corresponding timestamp of
                * execution.
                */
                std::map< std::time_t, std::vector<double> > evaluate(const bool force_new = false) const;

            private:
                QPTWORKFLOW& qptworkflow_;
                const std::vector<Task> tasks_{Task::MeasureCounts, Task::IdealProcess, Task::Session};
        };

        /**
        * @brief The type-erased QuantumProcessMatrix handle exposed in the python bindings. 
        */
        class QuantumProcessFidelityPython {
            public:
                //Due to the doubly nested template, it is necessary to implement a new constructor which takes in the python exposed type QuantumProcessTomographyPython
                //This requires a runtime check (via dynamic_cast) for compatible workflows, to construct the right QuantumProcessFidelity objects. 
                //To not pollute the standalone header, this was moved to a cpp file.
                QuantumProcessFidelityPython(QuantumProcessTomographyPython& qstpython);

                std::map< std::time_t, std::vector<double> > evaluate(const bool force_new = false) const {
                    return workflow_ptr_->evaluate(force_new);
                }

            private:
                std::unique_ptr<QuantumProcessFidelityPythonBase> workflow_ptr_; 
        };

        /**
        * @brief Helper function to evalute the process fidelity of two given complex-valued quantum process matrices.
        *
        * Arguments:
        * @param a left complex process matrix of type Eigen::Matrix
        * @param b right complex process matrix of type Eigen::Matrix.
        *
        * @return double The quantum process fidelity f(a,b).
        *
        * @details This function evaluates the quantum process fidelity f(a,b) = |trace(b' * a)|
        * for complex process matrices a and b..
        */
        double calculate_process_fidelity(const ComplexMatrix& measured, const ComplexMatrix& ideal)
        {
            std::complex<double> HS = (ideal.adjoint() * measured).trace();
            return std::sqrt(HS.real() * HS.real() + HS.imag() * HS.imag());
        }


        template <QPTWorkflow QPTWORKFLOW>
        requires CanStoreMeasuredCounts<QPTWORKFLOW> && CanStoreIdealProcesses<typename QPTWORKFLOW::QSTWorkflowType::ExecutableWorkflowType> && CanStoreSessionInfos<QPTWORKFLOW>
        std::map<std::time_t, std::vector<double>> QuantumProcessFidelity<QPTWORKFLOW>::evaluate(const bool force_new) const {
            std::map<std::time_t, std::vector<double>> timestamp2fidelities;
            //(1) initialize DataLoaderGenerator to either read in already stored results or generate new ones
            DataLoaderGenerator dlg(qptworkflow_.get_identifier(), tasks_, force_new);
            dlg.execute(qptworkflow_);

            //(2) obtain session info, measured bitcounts, and ideal processes
            std::vector<SessionInfo> session_infos = dlg.obtain_session_infos();
            std::vector<std::vector<std::map<std::vector<bool>, int>>> measured_bitcounts_collection = dlg.obtain_measured_counts();
            std::vector<std::vector<ComplexMatrix>> ideal_processes_collection = dlg.obtain_ideal_processes();
            std::vector<std::time_t> timestamps = dlg.get_timestamps();

            //(3) evaluate state fidelity for each circuit in each timestamp
            for (const auto & [measured_bitcounts, ideal_processes, timestamp] : std::ranges::views::zip(measured_bitcounts_collection, ideal_processes_collection, timestamps)) {
                std::vector<double> fidelities;
                //obtain measured densities using quantum state tomography (from within QPT object)
                //and pass these on to the QPT parent to assemble superoperator representations of the quantum process
                std::vector<ComplexMatrix> measured_processes = qptworkflow_.assemble_processes(qptworkflow_.get_qst().assemble_densities(measured_bitcounts));
                //finally compute a process fidelity for each superoperator matrix
                for (const auto & [measured_process, ideal_process] : std::ranges::views::zip(measured_processes, ideal_processes)) {
                    fidelities.push_back(calculate_process_fidelity(measured_process, ideal_process));
                }
                timestamp2fidelities.insert(std::make_pair(timestamp, fidelities));
            }

            return timestamp2fidelities;
        }
    }
}

#endif
