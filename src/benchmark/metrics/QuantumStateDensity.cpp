#include <qristal/core/benchmark/metrics/QuantumStateDensity.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>

namespace qristal {
  namespace benchmark {

    QuantumStateDensityPython::QuantumStateDensityPython(QuantumStateTomographyPython& qstpython) {
      cast_python_metric_pointer<QuantumStateDensity, 
                                 QuantumStateDensityPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>, 
                                 QuantumStateTomography<PyGSTiBenchmark>, 
                                 QuantumStateTomography<SimpleCircuitExecution>>(
                                    workflow_ptr_, qstpython
                                 );
    }

  }
}