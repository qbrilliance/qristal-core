// Copyright Quantum Brilliance Pty Ltd

#pragma once
#include <memory>

/// @file This file declares a util class supporting XACC <-> TKET interop.

// Forward declarations
namespace xacc { class CompositeInstruction; }
namespace tket { class Circuit; }

namespace qristal {
  /// @brief Util class supporting XACC IR to TKET IR conversion and vice versa.
  class tket_ir_converter {
    public:
      /// @brief Typedef of XACC IR (representations of quantum circuits)
      using xacc_ir_ptr_t = std::shared_ptr<xacc::CompositeInstruction>;
      /// @brief Typedef of TKET IR (representations of quantum circuits)
      using tket_ir_ptr_t = std::shared_ptr<tket::Circuit>;
      /// @brief Convert from XACC IR to TKET IR
      static tket_ir_ptr_t to_tket(xacc_ir_ptr_t xacc_ir);
      /// @brief Convert from TKET IR to XACC IR
      static xacc_ir_ptr_t to_xacc(tket_ir_ptr_t tket_ir);
  };
}
