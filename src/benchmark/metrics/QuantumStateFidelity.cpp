#include "qristal/core/benchmark/metrics/QuantumStateFidelity.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"

namespace qristal {
  namespace benchmark {

    QuantumStateFidelityPython::QuantumStateFidelityPython(QuantumStateTomographyPython& qstpython) {
      cast_python_metric_pointer<QuantumStateFidelity, 
                                 QuantumStateFidelityPythonBase, 
                                 QuantumStateTomographyPython, 
                                 QuantumStateTomography<SPAMBenchmark>, //compatible workflows to check
                                 QuantumStateTomography<RotationSweep>>(
                                    workflow_ptr_, qstpython
                                 );
    }

  }
}