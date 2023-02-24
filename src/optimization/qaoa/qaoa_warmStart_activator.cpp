/// Modifications Copyright (c) Quantum Brilliance.
///
/// This file is adapted from one that is part of XACC: 
/// https://github.com/eclipse/xacc/blob/master/quantum/plugins/algorithms/qaoa/qaoa.cpp

/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Thien Nguyen - initial API and implementation
 *******************************************************************************/
#include "qb/core/optimization/qaoa/qaoa_warmStart_algorithm.hpp"
#include "qb/core/optimization/qaoa/qaoa_warmStart_circuit.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"

#include <memory>
#include <set>

using namespace cppmicroservices;

namespace {

/**
 */
class US_ABI_LOCAL WSQAOAActivator : public BundleActivator {

public:
  WSQAOAActivator() {}

  /**
   */
  void Start(BundleContext context) {
    context.RegisterService<xacc::Algorithm>(std::make_shared<xacc::algorithm::WS_QAOA>());
    context.RegisterService<xacc::Instruction>(std::make_shared<xacc::circuits::WS_QAOA>());
  }

  /**
   */
  void Stop(BundleContext /*context*/) {}
};

} // namespace

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(WSQAOAActivator)
