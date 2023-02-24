// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#ifndef _QB_QAOA_
#define _QB_QAOA_
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <cassert>
#include <fstream>
#include <memory>
#include <numeric>

// from xacc lib
#include "PauliOperator.hpp"
#include "ObservableTransform.hpp"
#include "Utils.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"


// from qb lib
#include "qb/core/optimization/qaoa/qaoa_simple.hpp"
#include "qb/core/optimization/qaoa/qaoa_recursive.hpp"
#include "qb/core/optimization/qaoa/qaoa_warmStart.hpp"

#endif //_QB_QAOA_