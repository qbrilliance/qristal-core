#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>

namespace qristal {
  namespace benchmark {

    QuantumStateTomographyPython::QuantumStateTomographyPython(
        AddinFromIdealSimulationPython& workflow, 
        const std::set<size_t>& qubits,
        const bool perform_maximum_likelihood_estimation,
        const std::vector<Pauli>& basis,
        const Pauli& use_for_identity
    ) {
      cast_python_metric_pointer<QuantumStateTomography, 
                                 QuantumStateTomographyPythonBase, 
                                 AddinFromIdealSimulationPython, 
                                 AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity>, //compatible workflows to check 
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealDensity>, 
                                 AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealProcess>
                                 >(
                                    workflow_ptr_, workflow, qubits, perform_maximum_likelihood_estimation, basis, use_for_identity
                                 );
    }

    QuantumStateTomographyPython::QuantumStateTomographyPython(
        AddinFromIdealSimulationPython& workflow, 
        const bool perform_maximum_likelihood_estimation,
        const std::vector<Pauli>& basis,
        const Pauli& use_for_identity
    ) {
      cast_python_metric_pointer<QuantumStateTomography, 
                                 QuantumStateTomographyPythonBase, 
                                 AddinFromIdealSimulationPython, 
                                 AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity>, //compatible workflows to check 
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealDensity>,
                                 AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SimpleCircuitExecution>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<SPAMBenchmark>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<RotationSweep>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PyGSTiBenchmark>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SimpleCircuitExecution>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<SPAMBenchmark>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>, Task::IdealProcess>,
                                 AddinFromIdealSimulation<PreOrAppendWorkflow<PreOrAppendWorkflow<PyGSTiBenchmark>>, Task::IdealProcess>
                                 >(
                                    workflow_ptr_, workflow, perform_maximum_likelihood_estimation, basis, use_for_identity
                                 );
    }

  }
}