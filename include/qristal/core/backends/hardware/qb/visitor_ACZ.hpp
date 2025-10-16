/**
 * Copyright Quantum Brilliance
 */

#pragma once

#include <qristal/core/backends/hardware/qb/visitor.hpp>

#include <AllGateVisitor.hpp>
#include <CommonGates.hpp>
#include <xacc.hpp>

#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>


namespace xacc {

  namespace quantum {

    class visitor_ACZ : public visitor {

      public:

        /// Static model name
        static constexpr char* model = "QB-QDK2-ACZ";

        /// Constructor
        visitor_ACZ(const int nQubits, const bool cut_angles = true)
          : visitor(nQubits, cut_angles) {}

        /// Destructor
        virtual ~visitor_ACZ() {}

        /// Return name of the visitor
        virtual const std::string name() const;

        /// Return description of the visitor
        virtual const std::string description() const;

        /// \name Visitor functions for gates and measurement.
        /// @{
        /// Controlled Z gate
        void visit(CZ&)       override;
        /// Anti-controlled Z gate
        virtual void visit(ACZ&) override;
        /// Controlled NOT gate
        void visit(CNOT&)     override;
        /// Controlled phase gate
        void visit(CPhase&)   override;
        /// Swap the values of two qubits
        void visit(Swap&)     override;
        /// @}
    };

  }
}
