/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"
#include "qb/core/algorithms/exponential_search/exponential_search.hpp"

using namespace cppmicroservices;

class US_ABI_LOCAL qbOSExpSearchAlgoActivator : public BundleActivator {
public:
  qbOSExpSearchAlgoActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::Algorithm>(
        std::make_shared<qbOS::ExponentialSearch>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(qbOSExpSearchAlgoActivator)
