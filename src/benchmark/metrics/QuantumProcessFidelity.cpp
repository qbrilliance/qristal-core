#include "qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"

namespace qristal {
  namespace benchmark {

    QuantumProcessFidelityPython::QuantumProcessFidelityPython(QuantumProcessTomographyPython& qptpython) {
      cast_python_metric_pointer<QuantumProcessFidelity, 
                                 QuantumProcessFidelityPythonBase, 
                                 QuantumProcessTomographyPython, 
                                 QuantumProcessTomography<QuantumStateTomography<SPAMBenchmark>>, //compatible workflows to check
                                 QuantumProcessTomography<QuantumStateTomography<RotationSweep>>>(
                                    workflow_ptr_, qptpython
                                 );
    }

  }
}