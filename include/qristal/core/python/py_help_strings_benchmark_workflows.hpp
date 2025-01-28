#pragma once

namespace qristal {
  namespace help {
    namespace benchmark {
      const char* execute_ = R"(
          execute: 

          Run benchmark workflow and serialize results for a given list of tasks.

          Arguments: 
            List[Task]: A list of tasks to be executed. The following tasks are available 
              * MeasureCounts : the measured bit string counts after circuit execution, 
              * IdealCounts : the ideal (noise-free) bit string counts,
              * IdealDensity : the ideal quantum state density for each circuit, 
              * IdealProcess : the ideal quantum process matrix for each circuit, 
              * Session : the relevant information contained in the passed qristal.core.session.
          
          Returns: 
            int : The time stamp of successful task execution.
      )";
      const char* execute_all_ = R"(
          execute_all: 
          
          Run benchmark workflow and serialize results for all available tasks. 

          Arguments: --- 

          Returns: 
            int : The time stamp of successful task execution.
      )";
      const char* identifier_ = R"(
          identifier: 

          Returns the unique (str) workflow identifier. 
      )";
      const char* circuits_ = R"(
          circuits: 

          Returns a list of qristal.core.Circuit objects constructed by the workflow. 
      )";
      const char* session_ = R"(
          session: 

          Returns a reference to the qristal.core.session connected to the workflow.
      )";
      const char* qubits_ = R"(
            qubits: 

            Return the set of qubit indices on which the workflow is executed. 
        )";

      namespace SPAMBenchmark_ {
        const char* calculate_confusion_matrix_ = R"(
            calculate_confusion_matrix: 

            Helper function to compute the confusion matrix for the given SPAM workflow.

            Arguments: 
                * List[MapVectorBoolInt] : The measured bit string counts for all SPAM circuits 

            Returns: 
                * numpy.ndarray : The assembled confusion matrix. 
        )";
      }

      namespace RotationSweep_ {
        const char* start_rad_ = R"(
            start_rad: 

            Return the initial rotation angle of the rotation sweep in radian.
        )";
        const char* end_rad_ = R"(
            end_rad: 

            Return the final rotation angle of the rotation sweep in radian.
        )";
        const char* step_ = R"(
            step: 

            Return the angle step in radian between the rotations in the sweep.
        )";
        const char* get_rotations_per_qubit_ = R"(
            get_rotations_per_qubit: 

            Get the rotation axes per qubit in the rotation sweep. 

            Arguments: --- 

            Returns: 
              * List[char] : The individual rotation axes given by 'I' (do nothing), 'X', 'Y', or 'Z' per qubit.
        )";

      }

      namespace PyGSTiBenchmark_ {
        const char* pyGSTi_circuit_strings_ = R"(
          pyGSTi_circuit_strings: 

          Return the list of stored circuit strings in PyGSTi format.
        )";
      }

      namespace QuantumStateTomography_ {
        const char* get_basis_ = R"(
          get_basis: 

          Return the 1-qubit measurement basis used in the quantum state tomography protocol.
        )";
        const char* set_maximum_likelihood_estimation_ = R"(
          set_maximum_likelihood_estimation: 

          Enable maximum likelihood estimation (MLE) in the quantum state tomography protocol.

          Arguments: 
            * n_MLE_iterations : The maximum number of iterations used in the iterative MLE procedure, defaults to 100.
            * MLE_conv_threshold : The convergence threshold, defaults to 1e-3.
            * mBasisSymbols_to_Projectors : A dict from the used basis symbols to their corresponding complex-valued 2x2 projector matrices for bit results 0 and 1. By default, the mapping from the standard Pauli basis to Bloch sphere unit states.

          Returns: ---
        )";
        const char* assemble_densities_ = R"(
          assemble_densities: 

          Calculate density matrices from measured bit string counts of the quantum state tomography workflow.

          Arguments: 
            * counts : The measured bit string counts as serialized by the execute function.

          Returns: 
            * List[numpy.ndarray] : The assembled complex-valued quantum state density matrices.
        )";
      }

      namespace QuantumProcessTomography_ {
        const char* assemble_processes_ = R"(
          assemble_processes: 

          Calculate process matrices from measured quantum state densities by the passed quantum state tomography protocol.

          Arguments: 
            * densities : The measured densities obtained via the wrapped quantum state tomography protocoll given as List[numpy.ndarray].
          
          Returns: 
            * List[numpy.ndarray] : The calculated process matrices.
        )";
      }
    }
  }
}