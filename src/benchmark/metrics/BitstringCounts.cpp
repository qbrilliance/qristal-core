#include <qristal/core/benchmark/metrics/BitstringCounts.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>

namespace qristal {
  namespace benchmark {

    BitstringCountsPython::BitstringCountsPython(QuantumStateTomographyPython& qstpython) {
      cast_python_metric_pointer<BitstringCounts, 
                                 BitstringCountsPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>, 
                                 QuantumStateTomography<PyGSTiBenchmark>, 
                                 QuantumStateTomography<SimpleCircuitExecution>>(
                                    workflow_ptr_, qstpython
                                 );
    }

    BitstringCountsPython::BitstringCountsPython(QuantumProcessTomographyPython& qptpython) {
      cast_python_metric_pointer<BitstringCounts, 
                                 BitstringCountsPythonBase, 
                                 QuantumProcessTomographyPython, 
                                 QuantumProcessTomography<QuantumStateTomography<SPAMBenchmark>>, //compatible workflows to check
                                 QuantumProcessTomography<QuantumStateTomography<RotationSweep>>, 
                                 QuantumProcessTomography<QuantumStateTomography<PyGSTiBenchmark>>, 
                                 QuantumProcessTomography<QuantumStateTomography<SimpleCircuitExecution>>>(
                                    workflow_ptr_, qptpython
                                 );
    }

    BitstringCountsPython::BitstringCountsPython(PreOrAppendWorkflowPython& workflow) {
      cast_python_metric_pointer<BitstringCounts, 
                                 BitstringCountsPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>,
                                 PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>,
                                 PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>,
                                 PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>,
                                 PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>
                                 >(
                                    workflow_ptr_, workflow
                                 );
    }

    std::map< std::time_t, std::vector<std::map<std::vector<bool>, int>> > BitstringCountsPython::evaluate(
        const bool force_new, 
        const std::optional<Eigen::MatrixXd>& SPAM_confusion
    ) const {
        return workflow_ptr_->evaluate(force_new, SPAM_confusion);
    }

  }
}