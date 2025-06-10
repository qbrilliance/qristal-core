/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/algorithms/amplitude_estimation/canonical_amplitude_estimation.hpp>
#include <qristal/core/algorithms/amplitude_estimation/ML_amplitude_estimation.hpp>
#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceProperties.h>

using namespace cppmicroservices;

class US_ABI_LOCAL QAEAlgoActivator : public BundleActivator {
public:
  QAEAlgoActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::Algorithm>(
        std::make_shared<qristal::CanonicalAmplitudeEstimation>());
    context.RegisterService<xacc::Algorithm>(
        std::make_shared<qristal::MLAmplitudeEstimation>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(QAEAlgoActivator)
