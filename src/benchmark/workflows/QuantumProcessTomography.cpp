#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>


namespace qristal {
  namespace benchmark {

    QuantumProcessTomographyPython::QuantumProcessTomographyPython(
        QuantumStateTomographyPython& qstpython,
        const std::vector<BlochSphereUnitState>& states
    ) {
      cast_python_metric_pointer<QuantumProcessTomography, 
                                 QuantumProcessTomographyPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>, 
                                 QuantumStateTomography<PyGSTiBenchmark>, 
                                 QuantumStateTomography<SimpleCircuitExecution>,
                                 QuantumStateTomography<AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess>>
                                 >(
                                    workflow_ptr_, qstpython, states
                                 );
    }

  }
}
