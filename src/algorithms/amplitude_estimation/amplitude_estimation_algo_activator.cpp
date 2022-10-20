/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/algorithms/amplitude_estimation/canonical_amplitude_estimation.hpp"
#include "qb/core/algorithms/amplitude_estimation/ML_amplitude_estimation.hpp"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"

using namespace cppmicroservices;

class US_ABI_LOCAL qbOSQAEAlgoActivator : public BundleActivator {
public:
  qbOSQAEAlgoActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::Algorithm>(
        std::make_shared<qbOS::CanonicalAmplitudeEstimation>());
    context.RegisterService<xacc::Algorithm>(
        std::make_shared<qbOS::MLAmplitudeEstimation>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(qbOSQAEAlgoActivator)
