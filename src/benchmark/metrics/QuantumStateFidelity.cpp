#include <qristal/core/benchmark/metrics/QuantumStateFidelity.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>

namespace qristal {
  namespace benchmark {

    QuantumStateFidelityPython::QuantumStateFidelityPython(QuantumStateTomographyPython& qstpython) {
      cast_python_metric_pointer<QuantumStateFidelity, 
                                 QuantumStateFidelityPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>,
                                 QuantumStateTomography<AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealDensity>>,
                                 QuantumStateTomography<AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealDensity>>
                                 >(
                                    workflow_ptr_, qstpython
                                 );
    }

  }
}