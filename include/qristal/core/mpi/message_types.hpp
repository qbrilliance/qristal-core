#include <cstdint>

namespace qristal::mpi {

enum class MessageTags : int32_t {
  DEFAULT = 0,
  RESULTS_MAP = 100,
  RESULTS_NATIVE_MAP = 101,
  COUNTS = 102,
  PROBABILITIES = 103,
  PROBABILITY_GRADIENTS = 104,
  SHOT_COUNT = 105
};
}
