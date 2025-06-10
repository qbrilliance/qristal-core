// Copyright Quantum Brilliance
#include <qristal/core/qobj/QuantumBrillianceQobjCompiler.hpp>

#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceProperties.h>

#include <xacc.hpp>

namespace qristal {
/// Simple Pimpl wrapper for the default XACC's QObj generator,
/// allowing us to alias the service name for usability.
class XaccQObjPimpl : public xacc::Compiler {
  /// Underlying XACC Compiler plugin implementing IR -> IBM's QObj generation.
  const std::string XACC_QOBJ_PLUGIN_NAME = "qobj";

public:
  /// Constructor
  XaccQObjPimpl() = default;

  /**
   * @brief Compile input string
   *
   * @param src Source string
   * @param acc Backend accelerator
   * @return IR tree
   */
  std::shared_ptr<xacc::IR>
  compile(const std::string &src,
          std::shared_ptr<xacc::Accelerator> acc) override {
    // Forward the call to the underlying plugin.
    return xacc::getCompiler(XACC_QOBJ_PLUGIN_NAME)->compile(src, acc);
  }
  std::shared_ptr<xacc::IR> compile(const std::string &src) override {
    // Forward the call to the underlying plugin.
    return xacc::getCompiler(XACC_QOBJ_PLUGIN_NAME)->compile(src);
  }

  /**
   * @brief Translate IR tree (CompositeInstruction) to QObj Json string
   *
   * @param function CompositeInstruction
   * @return QObj Json string
   */
  const std::string
  translate(std::shared_ptr<xacc::CompositeInstruction> function) override {
    // Forward the call to the underlying plugin.
    return xacc::getCompiler(XACC_QOBJ_PLUGIN_NAME)->translate(function);
  }

  /**
   * @brief Translate IR tree (CompositeInstruction) to QObj Json string
   *
   * @param function CompositeInstruction
   * @param options Extra config option
   * @return QObj Json string
   */
  const std::string
  translate(std::shared_ptr<xacc::CompositeInstruction> function,
            xacc::HeterogeneousMap &options) override {
    // Forward the call to the underlying plugin.
    return xacc::getCompiler(XACC_QOBJ_PLUGIN_NAME)
        ->translate(function, options);
  }

  /**
   * @brief Name of the service (to retrieve from the service registry)
   *
   * Here, we create an alias for this service.
   * @return Name
   */
  const std::string name() const override { return "xacc-qobj"; }

  /**
   * @brief Description of this service
   *
   * @return Description
   */
  const std::string description() const override {
    return "XACC QObj compiler";
  }

  /// Destructor
  ~XaccQObjPimpl() override {}
};
}

namespace {
class US_ABI_LOCAL QObjCompilersActivator
    : public cppmicroservices::BundleActivator {

public:
  QObjCompilersActivator() {}

  // Start plugin bundle (loading the plugin shared lib)
  void Start(cppmicroservices::BundleContext context) {
    context.RegisterService<xacc::Compiler>(
        std::make_shared<qristal::QuantumBrillianceQobjCompiler>());
    context.RegisterService<xacc::Compiler>(
        std::make_shared<qristal::XaccQObjPimpl>());
  }

  void Stop(cppmicroservices::BundleContext /*context*/) {}
};

} // namespace

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(QObjCompilersActivator)
