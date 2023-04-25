/***
 *** Copyright (c) 2020 Quantum Brilliance Pty Ltd
 ***/

#ifndef QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEVISITOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEVISITOR_HPP_

#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "xacc.hpp"
#include <memory>
#include <sstream>

namespace xacc {
namespace quantum {

class QuantumBrillianceVisitor : public AllGateVisitor {
protected:
  double pi = xacc::constants::pi;
  /**
   * Reference to the classical memory address indices
   * where measurements are recorded.
   **/
  std::string classicalAddresses;

  /**
   * TBD: ...document the purpose of all member variables...
   **/
  std::map<int, int> qubitToClassicalBitIndex;

  /**
   * TBD: ...document the purpose of all member variables...
   **/
  int numAddresses = 0;

  /**
   * Number of qubits
   **/
  int _nQubits;

public:
    QuantumBrillianceVisitor(const int nQubits, bool skipPreamble = false) : _nQubits(nQubits) {
        if (!skipPreamble) {
            native += "\n__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude \"qelib1.inc\";\n";
        }
    }
    QuantumBrillianceVisitor() : QuantumBrillianceVisitor(4) {}
  virtual ~QuantumBrillianceVisitor() {}
    virtual const std::string name() const { return "quantumbrilliance-openqasm-visitor"; }
  virtual const std::string description() const {
    return "Maps XACC IR to the native gates available in the Quantum Brilliance technology, output in the OpenQASM format";
  }
  int classicalBitCounter = 0;

    /**
    * Useful reference for U3-gate (the most general of all single-qubit quantum gates)
    *
    * U3(theta, phi, lambda) =
    *    [                cos(0.5*theta)   -exp(1.0im*lambda)*sin(0.5*theta);
    *      exp(1.0im*phi)*sin(0.5*theta)    exp(1.0im*lambda + 1.0im*phi)*cos(0.5*theta)   ]
    *
    * U3(theta, phi, lambda) = Rz(phi + 3.0*pi)*Rx(0.5*pi)*Rz(theta + pi)*Rx(0.5*pi)*Rz(lambda)
    **/

    /**
    *
    * U2(phi, lambda) = U3(0.5*pi, phi, lambda) =
    *    inv(sqrt(2))*[  1.0               -exp(1.0im*lambda);
    *                    exp(1.0im*phi)     exp(1.0im*lambda + 1.0im*phi) ]
    *
    * U2(phi, lambda) = Rz(phi + 0.5*pi)*Rx(0.5*pi)*Rz(lambda - 0.5*pi)
    **/

    /**
    *
    * U1(lambda) = U3(0, 0, lambda) =
    *                 [  1.0     0.0;
    *                    0.0     exp(1.0im*lambda) ]
    *
    * U1(lambda) ~ Rz(lambda)
    **/

    /**
    * Gates that are native to the Quantum Brilliance technology
    **/

    /**
    * Identity - this is the equivalent of a no-op
    *
    * Input: reference to IR object of class Identity
    *
    * Output: none
    *
    * Effect: none
    **/
    //
    // q0: --|I|--
    //
  void visit(Identity& id) {
    std::stringstream ss;
    ss << "u"          << "("   // Identity in U3 format
           << 0.0          << ", "  // theta
           << 0.0          << ", "  // phi
           << 0.0          << ") "  // lambda
           << "q"          << "["
           << id.bits()[0] << "];"  // target qubit
                           << "\n";
        native += ss.str();
  }

    /**
    * Rx(angleStr) - rotate around the x-axis by angleStr radians
    *
    * Input: reference to IR object of class Rx
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Rx(angleStr)|--
    //
  void visit(Rx& rx) {
    std::stringstream ss;
    auto angleStr = rx.getParameter(0).toString();
    ss << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << rx.bits()[0] << "];"  // target qubit
                           << "\n";
        native += ss.str();
  }

    /**
    * Ry(angleStr) - rotate around the y-axis by angleStr radians
    *
    * Input: reference to IR object of class Ry
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Ry(angleStr)|--
    //
  void visit(Ry& ry) {
    std::stringstream ss;
    auto angleStr = ry.getParameter(0).toString();
    ss << "u"          << "("   // Ry in U3 format
           << angleStr     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << ry.bits()[0] << "];"  // target qubit
                           << "\n";
    native += ss.str();
  }

    /**
    * CZ - controlled Z
    *
    * Input: reference to IR object of class CZ
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    *
    **/
    // Note: uses QB Escaped Gate Sequences
    //
    // q0: ------------|C|--------------
    //
    // q1: ------------|CZ|-------------
    //
    void visit(CZ &cz) {
    std::stringstream ss;
    ss
           << "cz"         << " "   // CZ in OpenQASM macro format
           << "q"          << "["
           << cz.bits()[0] << "],"  // control qubit
           << "q"          << "["
           << cz.bits()[1] << "];"  // target qubit
                           << "\n";
    native += ss.str();
    }

    /**
    * Non-native gates
    **/

    /**
    * CNOT - controlled NOT
    *
    * Input: reference to IR object of class CNOT
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    *
    **/
    // Note: uses QB Escaped Gate Sequences
    //
    // q0: -------------------------|C|----------------------------
    //
    // q1: --|Ry(0.5*pi)|--|Rx(pi)|--|CNOT|--|Ry(0.5*pi)|--|Rx(pi)|--
    //
    void visit(CNOT& cn) {
    std::stringstream ss;
    ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi)    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << cn.bits()[1] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << cn.bits()[1] << "];"  // target qubit
                           << "\n"
           << "cz"         << " "   // CZ in OpenQASM macro format
           << "q"          << "["
           << cn.bits()[0] << "],"  // control qubit
           << "q"          << "["
           << cn.bits()[1] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (0.5*pi)    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << cn.bits()[1] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << cn.bits()[1] << "];"  // target qubit
                           << "\n";

    native += ss.str();
}

  /**
  * Hadamard  - Hadamard gate
    *
    * Input: reference to IR object of class Hadamard
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Ry(0.5*pi)--|Rx(pi)||--
    //
  void visit(Hadamard& h) {
    std::stringstream ss;
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << h.bits()[0]  << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << h.bits()[0]  << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * Rz(angleStr) - rotate around the z-axis by angleStr radians
    *
    * Input: reference to IR object of class Rz
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // --|Ry(0.5*pi)--|Rx(angleStr)|--|Ry(-0.5*pi)|--
    //
    void visit(Rz& rz) {
    std::stringstream ss;
    auto angleStr = rz.getParameter(0).toString();
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi )   << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << rz.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << rz.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Ry in U3 format
           << (-0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << rz.bits()[0] << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * S - rotate around the z-axis by 0.5*pi
    *
    * Input: reference to IR object of class S
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/

  void visit(S& s) {
    std::stringstream ss;
    auto angleStr = 0.5*pi;
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi )    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << s.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << s.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << s.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << s.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << s.bits()[0] << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * Sdg - rotate around the z-axis by -0.5*pi
    *
    * Input: reference to IR object of class Sdg
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/

  void visit(Sdg& sdg) {
    std::stringstream ss;
    auto angleStr = -0.5*pi;
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi )    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << sdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << sdg.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << sdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << sdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << sdg.bits()[0] << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * T - rotate around the z-axis by 0.25*pi
    *
    * Input: reference to IR object of class T
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/

  void visit(T& t) {
    std::stringstream ss;
    auto angleStr = 0.25*pi;
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi )    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << t.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << t.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << t.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << t.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << t.bits()[0] << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * Tdg - rotate around the z-axis by -0.25*pi
    *
    * Input: reference to IR object of class Tdg
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/

  void visit(Tdg& tdg) {
    std::stringstream ss;
    auto angleStr = -0.25*pi;
        ss << "u"          << "("   // Ry in U3 format
           << (0.5*pi )    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << tdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << tdg.bits()[0] << "];"  // target qubit
                           << "\n"

           << "u"          << "("   // Rx in U3 format
           << angleStr     << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << tdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (0.5*pi)     << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << tdg.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << tdg.bits()[0] << "];"  // target qubit
                           << "\n";

    native += ss.str();
  }

    /**
    * X - rotate around the x-axis by pi radians
    *
    * Input: reference to IR object of class X
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Rx(pi)|--
    //
  void visit(X& x) {
    std::stringstream ss;
    ss << "u"          << "("   // X in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << x.bits()[0]  << "];"  // target qubit
                           << "\n";
        native += ss.str();
  }

    /**
    * Y - rotate around the y-axis by pi radians
    *
    * Input: reference to IR object of class Y
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Ry(pi)|--
    //
  void visit(Y& y) {
    std::stringstream ss;
    ss << "u"          << "("   // Y in U3 format
           << (pi)         << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << y.bits()[0]  << "];"  // target qubit
                           << "\n";
        native += ss.str();
  }

    /**
    * Z - rotate around the z-axis by pi radians
    *
    * Input: reference to IR object of class Z
    *
    * Output: none
    *
    * Effect: appends to the std::string 'native'
    **/
    //
    // q0: --|Rx(pi)|--|Ry(pi)|--
    //
  void visit(Z& z) {
    std::stringstream ss;
        ss << "u"          << "("   // Rx in U3 format
           << (pi)         << ", "  // theta
           << (-0.5*pi)    << ", "  // phi
           << (0.5*pi)     << ") "  // lambda
           << "q"          << "["
           << z.bits()[0] << "];"  // target qubit
                           << "\n"
           << "u"          << "("   // Ry in U3 format
           << (pi)    << ", "  // theta
           << 0            << ", "  // phi
           << 0            << ") "  // lambda
           << "q"          << "["
           << z.bits()[0] << "];"  // target qubit
                           << "\n";
        native += ss.str();
  }

  void visit(CPhase& cp) {
        xacc::error("QB SDK does not support: CPhase");
    }

  void visit(Swap& s) {
        CNOT c1(s.bits()),
             c2(s.bits()[1],s.bits()[0]),
             c3(s.bits());
        visit(c1);
        visit(c2);
        visit(c3);
  }

    void visit(U& u) {
        std::stringstream ss;
        auto theta_str  = u.getParameter(0).toString();
        auto phi_str    = u.getParameter(1).toString();
        auto lambda_str = u.getParameter(2).toString();
        ss << "u"          << "("   // u in U3 format
           << theta_str    << ", "  // theta
           << phi_str      << ", "  // phi
           << lambda_str   << ") "  // lambda
           << "q"          << "["
           << u.bits()[0]  << "];"  // target qubit
                           << "\n";
        native += ss.str();

    }

    void visit(Measure& m) {
        std::stringstream ss;
    ss << "creg c" << classicalBitCounter << "[1];\n";
    ss << "measure q[" << m.bits()[0] << "] -> c" << classicalBitCounter << "[0];\n";
    native += ss.str();
    qubitToClassicalBitIndex.insert(std::make_pair(m.bits()[0], classicalBitCounter));
    classicalBitCounter++;
    }

  /**
   * Return the finished qpu OpenQasm kernel
   */
  std::string getFinishedOpenQasmQpu() {
        std::string finished = native + "\n}\n";
        return finished;
  }
};

} // namespace quantum
} // namespace xacc

#endif

