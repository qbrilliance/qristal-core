
// Copyright (c) Quantum Brilliance Pty Ltd
#include <pybind11/pybind11.h>

// This file declares sub-component bindings (e.g., for a particular C++ class) of the overall module
// so that we can break the module binding into multiple cpp files.
namespace qb {
/// Bind circuit placement passes to Python API.
void bind_placement_passes(pybind11::module &m);
} // namespace qb