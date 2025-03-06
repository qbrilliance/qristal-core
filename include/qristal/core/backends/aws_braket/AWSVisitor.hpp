/*** 
 *** Copyright (c) Quantum Brilliance Pty Ltd
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

class AWSVisitor : public AllGateVisitor {
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

private:
  bool m_verbatim;


public:
    AWSVisitor(const int nQubits, 
                bool skipPreamble = false,
                bool verbatim = false
                ) : _nQubits(nQubits) {
        if (!skipPreamble) {
            native += "Circuit().";

            m_verbatim = verbatim;
        }
    }
    AWSVisitor() : AWSVisitor(4) {}
	virtual ~AWSVisitor() {}   
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
		ss << "i"          << "(["   // Identity in U3 format
           << id.bits()[0] << "])." ; // target qubit
                           //<< "\n";
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
        auto angle = rx.getParameter(0);
		auto angleStr = rx.getParameter(0).toString();
        auto bit = rx.bits()[0];

        if (m_verbatim){
            if(angle == pi || angle == pi/2.) {
            ss << "rx(" << bit << ", " << angleStr << ").";
            }
            else{
            ss  << "rz(" << bit  << ", " << pi/2 << ")."
                << "rx(" << bit  << ", " << pi/2 << ")."
                << "rz(" << bit  << ", " << angleStr << ")."
                << "rx(" << bit  << ", " << -pi/2 << ")."
                << "rz(" << bit  << ", " << -pi/2 << ").";
            }
        }
        else{
		ss << "rx"          << "("   // Rx in UAWS3 format
           << rx.bits()[0] << ", "  // target qubit
           << angleStr     << ").";  // theta
                           //<< "\n";
        }
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

        if (m_verbatim) {
            ss  << "rx" << "(" << ry.bits()[0] << ", " << pi/2 << ")."
                << "rz" << "(" << ry.bits()[0] << ", " << angleStr << ")."
                << "rx" << "(" << ry.bits()[0] << ", " << -pi/2 << ").";
        }
        else{
		    ss  << "ry"          << "("   // Ry in AWS format
                << ry.bits()[0] << ", "  // target qubit
                << angleStr     << ").";  // theta
                           //<< "\n";
        }
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
           << "cz"         << "("   // CZ in OpenQASM macro format           
           << cz.bits()[0] << ", "  // control qubit
           << cz.bits()[1] << ").";  // target qubit
                           //<< "\n";
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
        auto cbit = cn.bits()[0];
        auto tbit = cn.bits()[1];
        if (m_verbatim){
        ss << "rz" << "(" << tbit << "," << -pi/2 << ")."
           << "rx" << "(" << tbit << "," << pi/2 << ")."
           << "cz" << "(" << tbit << "," << cbit << ")."
           << "rz" << "(" << cbit << "," << pi/2 << ")."
           << "rx" << "(" << tbit << "," << -pi/2 << ")."
           << "rz" << "(" << tbit << "," << pi/2 << ").";
        }
        else{
		ss << "cnot"         << "("   // CZ in OpenQASM macro format           
           << cbit << ", "  // control qubit
           << tbit << ").";  // target qubit
                           //<< "\n";
        }
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
        auto bit = h.bits()[0];

        if (m_verbatim){
        ss << "rz" << "(" << bit << "," << pi/2 << ")."
           << "rx" << "(" << bit << "," << pi/2 << ")."
           << "rz" << "(" << bit << "," << pi/2 << ").";
        }
        else{
        ss << "h"          << "("   // Ry in U3 format
           << h.bits()[0]  << ").";  // target qubit
                           //<< "\n";
        }   
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
    // q0: --|Ry(0.5*pi)|--|Rx(pi)|--|Rx(angleStr)|--|Ry(0.5*pi)|--|Rx(pi)|--
    //
    void visit(Rz& rz) {
		std::stringstream ss;
		auto angleStr = rz.getParameter(0).toString();
        ss << "rz"          << "("   // Rz in AWS format
           << rz.bits()[0] << ", "  // target qubit
           << angleStr     << ").";  // theta
                           //<< "\n";
         
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
        ss << "s"          << "(["   // s in AWS format
           << s.bits()[0] << "])." ; // target qubit
                           //<< "\n";
           
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
        ss << "si"          << "(["   // s_dagger in AWS format
           << sdg.bits()[0] << "])";  // target qubit
                          // << "\n"  ;     
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
        ss << "t"          << "(["   // t gate in AWS format
           << t.bits()[0] << "])." ; // target qubit
                       //<< "\n";
           
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
        ss << "ti"          << "(["   // t_dagger in U3 format
           << tdg.bits()[0] << "]).";  // target qubit
                           //<< "\n";
           
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
        auto bit = x.bits()[0];

        if(m_verbatim){
            ss << "rx(" << bit << ", pi).";
        }
        else{
		ss << "x"          << "(["   // X in U3 format
           << x.bits()[0]  << "]).";  // target qubit
                           //<< "\n";
        }
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
        auto bit = y.bits()[0];

        if (m_verbatim){
        ss << "rz(" << bit << ", pi)."
           << "rx(" << bit << ", pi).";
        }
        else{
		ss << "y"          << "(["   // X in U3 format
           << y.bits()[0]  << "]).";  // target qubit
                           //<< "\n";
        }
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
        auto bit = z.bits()[0];

        if (m_verbatim){
        ss << "rz(" << bit << "pi).";
        }
        else{
        ss << "z"          << "(["   // X in U3 format
           << z.bits()[0]  << "]).";  // target qubit
                          // << "\n";
        }
        native += ss.str();
	}

	void visit(CPhase& cp) {
        std::stringstream ss;
        auto angleStr = cp.getParameter(0).toString();
		ss << "cphaseshift"         << "("   // CZ in OpenQASM macro format           
           << cp.bits()[0] << ", "  // control qubit
           << cp.bits()[1] << ", "  // target qubit
           << angleStr << ").";     // Angle theta
                           //<< "\n";
        
		native += ss.str();
	}

    void visit(XY& xy) {
        std::stringstream ss;
        auto angleStr = xy.getParameter(0).toString();
		ss << "xy"         << "("   // CZ in OpenQASM macro format           
           << xy.bits()[0] << ", "  // control qubit
           << xy.bits()[1] << ", "  // target qubit
           << angleStr << ").";     // Angle theta
                           //<< "\n";
        
		native += ss.str();
	}

	void visit(Swap& s) {
        std::stringstream ss;
        CNOT cn1 (s.bits());
        CNOT cn2 (s.bits());
        CNOT cn3 (s.bits());
    
        if (m_verbatim){
            visit(cn1);
            visit(cn2);
            visit(cn3);
        }
        else{
        ss << "swap"          << "("   // X in U3 format
           << s.bits()[0]  << ", "  // target qubit
           << s.bits()[1]  << ").";
                           //<< "\n";
        }
        native += ss.str();
	}

    void visit(U& u) {
        std::stringstream ss;
        auto t = u.getParameter(0).toString();
        auto p = u.getParameter(1).toString();
        auto l = u.getParameter(2).toString(); 

        Rz rz(u.bits()[0], t);
        Ry ry(u.bits()[0], p);
        Rz rz2(u.bits()[0], l);
        visit(rz);
        visit(ry);
        visit(rz2);

        /*
        ss << "u"          << "("   // u in U3 format
           << theta_str    << ", "  // theta
           << phi_str      << ", "  // phi
           << lambda_str   << ") "  // lambda
           << "q"          << "[" 
           << u.bits()[0]  << "];"  // target qubit
                           << "\n";
        native += ss.str();
        */
    }


	/**
	 * Return the finished qpu OpenQasm kernel
	 */
	std::string getFinishedOpenQasmQpu() {
        std::string finished = native;

        if(!finished.empty())
            finished.pop_back();

        return finished;
	}
};

} // namespace quantum
} // namespace xacc

#endif

