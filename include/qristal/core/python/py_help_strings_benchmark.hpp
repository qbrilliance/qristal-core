#pragma once

namespace qristal {
  namespace help {
    namespace benchmark {

      namespace Task_ {
        const char* get_identifier_ = R"(
          get_identifier: 
          
          Converts a qristal.core.benchmark.Task to its identifier string.
        )";
      }

      namespace Pauli_ {
        const char* get_matrix_ = R"(
          get_matrix: 

          Returns the matrix representation of the given Pauli symbol as a numpy.ndarray.
        )";
        const char* symbol_ = R"(
          symbol: 

          Returns the stored qristal.core.benchmark.PauliSymbol of the given Pauli.
        )";
        const char* append_circuit_ = R"(
          append_circuit: 

          Append a given qristal.core.Circuit by post rotations to measure in the given Pauli basis. 

          Arguments: 
            * circuit : The qristal.core.Circuit to be appended. 
            * qubit : The integer qubit index, where the post rotation is applied.

          Return: 
            * qristal.core.Circuit : The appended circuit. 
        )";
      }

      namespace BlochSphereUnitState_ {
        const char* get_matrix_ = R"(
          get_matrix: 

          Returns the matrix representation of the given BlochSphereUnitState symbol as a numpy.ndarray.
        )";
        const char* symbol_ = R"(
          symbol: 

          Returns the stored qristal.core.benchmark.BlochSphereUnitStateSymbol of the given Pauli.
        )";
        const char* append_circuit_ = R"(
          append_circuit: 

          Append a given qristal.core.Circuit by an initialization gate to the given BlochSphereUnitState. 

          Arguments: 
            * circuit : The qristal.core.Circuit to be appended. 
            * qubit : The integer qubit index, where the state initalization is applied.

          Return: 
            * qristal.core.Circuit : The appended circuit. 
        )";
      }
    }
  }
}