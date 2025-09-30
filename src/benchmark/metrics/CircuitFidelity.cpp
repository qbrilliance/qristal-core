#include <qristal/core/benchmark/metrics/CircuitFidelity.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>

namespace qristal {
  namespace benchmark {

    CircuitFidelityPython::CircuitFidelityPython(AddinFromIdealSimulationPython& workflow) {
      cast_python_metric_pointer<CircuitFidelity, 
                                 CircuitFidelityPythonBase, 
                                 AddinFromIdealSimulationPython, 
                                 AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealCounts>, //compatible workflows to check 
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealCounts>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealCounts>
                                 >(
                                    workflow_ptr_, workflow
                                 );
    }

  }
}