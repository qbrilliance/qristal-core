#include <qristal/core/benchmark/metrics/QuantumStateFidelity.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>

namespace qristal {
  namespace benchmark {

    QuantumStateFidelityPython::QuantumStateFidelityPython(QuantumStateTomographyPython& qstpython) {
      cast_python_metric_pointer<QuantumStateFidelity, 
                                 QuantumStateFidelityPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>,
                                 QuantumStateTomography<AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity>>
                                 >(
                                    workflow_ptr_, qstpython
                                 );
    }

  }
}