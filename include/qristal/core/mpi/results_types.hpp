#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace qristal::mpi {

using Count = int32_t;

using OutCounts = std::vector<Count>;

using Qubits = std::vector<bool>;

using ResultsMap = std::map<Qubits, Count>;

using Probability = double;

using OutProbabilities = std::vector<Probability>;

using OutProbabilityGradients = std::vector<std::vector<Probability>>;

} // namespace qristal
