
// Copyright Quantum Brilliance Pty Ltd

#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceProperties.h>

#include <qristal/core/tket/tket_circuit_opt.hpp>
#include <qristal/core/tket/tket_placement.hpp>

using namespace cppmicroservices;

class US_ABI_LOCAL TketPluginActivator : public BundleActivator {
public:
  TketPluginActivator() {}

  void Start(BundleContext context) {
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::TketPlacement>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::SequencePass>()); 
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_redundancy_removal_plugin>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_two_qubit_squash_plugin>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_full_peephole_plugin>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_simplify_initial_plugin>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_decompose_swap_plugin>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_commute_through_multis>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_optimise_post_routing>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_decompose_ZX>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_rebase_to_clifford>());
    context.RegisterService<xacc::IRTransformation>(
        std::make_shared<qristal::tket_optimise_cliffords>());
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(TketPluginActivator)
