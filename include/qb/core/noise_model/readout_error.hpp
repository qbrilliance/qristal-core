// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

namespace qb
{
  
  /// Probabilities of reading out a value for a qubit that does not reflect its true state.
  struct ReadoutError
  {
      /**
      * @brief Classical probability of detecting 0 whereas the true state was |1>
      */
      double p_01;
      /**
      * @brief Classical probability of detecting 1 whereas the true state was |0>
      */
      double p_10;
  };

}
