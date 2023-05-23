/**
 * Copyright Quantum Brilliance
 */

#pragma once

#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "xacc.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>


namespace xacc {

  namespace quantum {
   
    class QuantumBrillianceRemoteVisitor : public AllGateVisitor {
    
      protected:
      
        /// Number of qubits
        int nQubits_;
      
        /// Number of classical bit registers to which measurements have been assigned
        unsigned int classicalBitCounter_ = 0;

        /// Map from qubit indices to assigned classical readout bit indices
        std::map<unsigned int, unsigned int> qubitToClassicalBitIndex_;
      
        /// JSON for sequence of gates (XASM format)
        nlohmann::json sequence_;
      
        /// No point reinventing the circle is there?     
        const double pi = xacc::constants::pi;

        /// Restrict native gate rotation angles to (-pi,pi]
        bool restrict_angles_to_pmpi_;

      public:
      
        /// Constructor
        QuantumBrillianceRemoteVisitor(const int nQubits, const bool cut_angles = true)
            : nQubits_(nQubits),
              restrict_angles_to_pmpi_(cut_angles) {}
      
        /// Destructor
        virtual ~QuantumBrillianceRemoteVisitor() {}
      
        /// Return name of the visitor 
        virtual const std::string name() const;
      
        /// Return description of the visitor
        virtual const std::string description() const;
          
        /// Normalise angles to the interval (-pi,pi]
        double norm(const double&);

        /// \name Visitor functions for gates and measurement.
        /// @{

        /// Identity (no-op)
        void visit(Identity&) override;
        /// Rotation about x axis
        void visit(Rx&)       override;
        /// Rotation about y axis
        void visit(Ry&)       override;
        /// Controlled Z gate
        void visit(CZ&)       override;
        /// Rotation about z axis
        void visit(Rz&)       override;
        /// Hadamard gate
        void visit(Hadamard&) override;
        /// Controlled NOT gate
        void visit(CNOT&)     override;
        /// S gate
        void visit(S&)        override;
        /// Inverse S gate
        void visit(Sdg&)      override;
        /// T gate
        void visit(T&)        override;
        /// Inverse T gate
        void visit(Tdg&)      override;
        /// Pauli X gate
        void visit(X&)        override;
        /// Pauli Y gate
        void visit(Y&)        override;
        /// Pauli Z date
        void visit(Z&)        override;
        /// Controlled phase gate
        void visit(CPhase&)   override;
        /// Swap the values of two qubits
        void visit(Swap&)     override;
        /// General 1-qubit unitary
        void visit(U&)        override;
        /// Measure a single qubit
        void visit(Measure&)  override;

        /// @}

        /// Return the finished qpu OpenQasm kernel
        std::string getXasmString();

        /// Retrieved the IR tree in the basis gate set.
        std::shared_ptr<xacc::CompositeInstruction> getTranspiledIR() const;
    };

} // namespace quantum
} // namespace xacc

