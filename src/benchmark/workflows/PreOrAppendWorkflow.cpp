#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>


namespace qristal {
  namespace benchmark {

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const std::vector<qristal::CircuitBuilder>& circuits, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuits, placement
                                 );
    }

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const qristal::CircuitBuilder& circuit, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuit, placement
                                 );
    }

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const std::vector<std::vector<qristal::Pauli>>& circuits, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuits, placement
                                 );
    }

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const std::vector<qristal::Pauli>& circuit, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuit, placement
                                 );
    }

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const std::vector<std::vector<qristal::BlochSphereUnitState>>& circuits, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuits, placement
                                 );
    }

    PreOrAppendWorkflowPython::PreOrAppendWorkflowPython(
        PreOrAppendWorkflowPython& workflow,
        const std::vector<qristal::BlochSphereUnitState>& circuit, 
        const Placement placement
    ) {
      cast_python_metric_pointer<PreOrAppendWorkflow, 
                                 PreOrAppendWorkflowPythonBase, 
                                 PreOrAppendWorkflowPython, 
                                 PreOrAppendWorkflow<SPAMBenchmark>, //compatible workflows to check
                                 PreOrAppendWorkflow<RotationSweep>, 
                                 PreOrAppendWorkflow<PyGSTiBenchmark>, 
                                 PreOrAppendWorkflow<SimpleCircuitExecution>
                                 >(
                                    workflow_ptr_, workflow, circuit, placement
                                 );
    }
  }
}
