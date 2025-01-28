#include "qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp"
#include "qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp"

namespace qristal {
  namespace benchmark {

    QuantumProcessMatrixPython::QuantumProcessMatrixPython(QuantumProcessTomographyPython& qptpython) {
      cast_python_metric_pointer<QuantumProcessMatrix, 
                                 QuantumProcessMatrixPythonBase, 
                                 QuantumProcessTomographyPython, 
                                 QuantumProcessTomography<QuantumStateTomography<SPAMBenchmark>>, //compatible workflows to check
                                 QuantumProcessTomography<QuantumStateTomography<RotationSweep>>, 
                                 QuantumProcessTomography<QuantumStateTomography<PyGSTiBenchmark>>, 
                                 QuantumProcessTomography<QuantumStateTomography<SimpleCircuitExecution>>>(
                                    workflow_ptr_, qptpython
                                 );
    }

  }
}