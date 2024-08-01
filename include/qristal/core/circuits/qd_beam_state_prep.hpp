#pragma once 
#include "Circuit.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>
///////////////////////////////////
// Other include statements go here
///////////////////////////////////

namespace qristal {

///////////////////////////////////
// Describe the circuit including
// inputs and outputs
///////////////////////////////////

class BeamStatePrep : public xacc::quantum::Circuit {
public:
  BeamStatePrep() : xacc::quantum::Circuit("BeamStatePrep") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(BeamStatePrep);
};
}