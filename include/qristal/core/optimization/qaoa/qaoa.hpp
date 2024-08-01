// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

// STL
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <cassert>
#include <fstream>
#include <memory>
#include <numeric>

// XACC
#include "PauliOperator.hpp"
#include "ObservableTransform.hpp"
#include "Utils.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"


// Qristal
#include "qristal/core/optimization/qaoa/qaoa_simple.hpp"
#include "qristal/core/optimization/qaoa/qaoa_recursive.hpp"
#include "qristal/core/optimization/qaoa/qaoa_warmStart.hpp"
