// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <vector>

namespace qristal {
  /// Convenient typedef for nested vectors.
  template <class T> using Table2d = std::vector<std::vector<T>>;
}
