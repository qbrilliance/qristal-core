// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#pragma once
#include <vector>

namespace qb {
  /// Convenient typedef for nested vectors.
  template <class T> using Table2d = std::vector<std::vector<T>>;
}
