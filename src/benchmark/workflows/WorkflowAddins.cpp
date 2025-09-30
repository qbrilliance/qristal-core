#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>



namespace qristal {
  namespace benchmark {

    //IdealCounts
    template <typename T>
    using AddinIdealCounts = AddinFromIdealSimulation<T, Task::IdealCounts>;
    //IdealDensity
    template <typename T>
    using AddinIdealDensity = AddinFromIdealSimulation<T, Task::IdealDensity>;
    //IdealProcess
    template <typename T>
    using AddinIdealProcess = AddinFromIdealSimulation<T, Task::IdealProcess>;

    AddinFromIdealSimulationPython::AddinFromIdealSimulationPython(
        PreOrAppendWorkflowPython& workflow,
        const Task& task
    ) {
      switch (task) {
        case Task::IdealCounts: {
          cast_python_metric_pointer<AddinIdealCounts, 
                                     AddinFromIdealSimulationPythonBase, 
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
          break; 
        }
        case Task::IdealDensity: {
          cast_python_metric_pointer<AddinIdealDensity, 
                                     AddinFromIdealSimulationPythonBase, 
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
          break; 
        }
        case Task::IdealProcess: {
          cast_python_metric_pointer<AddinIdealProcess, 
                                     AddinFromIdealSimulationPythonBase, 
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
          break; 
        }
      }
    }

  }
}
