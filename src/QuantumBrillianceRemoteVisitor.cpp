/**
 * Copyright Quantum Brilliance
 */

#include "qb/core/QuantumBrillianceRemoteVisitor.hpp"


/**
* Useful reference for U3-gate (the most general of all single-qubit quantum gates)
*
* U3(theta, phi, lambda) =
*    [                cos(0.5*theta)   -exp(1.0im*lambda)*sin(0.5*theta);
*      exp(1.0im*phi)*sin(0.5*theta)    exp(1.0im*lambda + 1.0im*phi)*cos(0.5*theta)   ]
*
* U3(theta, phi, lambda) = Rz(phi + 3.0*pi)*Rx(0.5*pi)*Rz(theta + pi)*Rx(0.5*pi)*Rz(lambda)
* U3(theta, phi, lambda) = Rz(phi)*Rx(-0.5*pi)*Rz(theta)*Rx(0.5*pi)*Rz(lambda)
*
*
* U2(phi, lambda) = U3(0.5*pi, phi, lambda) =
*    inv(sqrt(2))*[  1.0               -exp(1.0im*lambda);
*                    exp(1.0im*phi)     exp(1.0im*lambda + 1.0im*phi) ]
*
* U2(phi, lambda) = Rz(phi + 0.5*pi)*Rx(0.5*pi)*Rz(lambda - 0.5*pi)
*
* 
* U1(lambda) = U3(0, 0, lambda) =
*                 [  1.0     0.0;
*                    0.0     exp(1.0im*lambda) ]
*
* U1(lambda) ~ Rz(lambda)
**/


namespace xacc
{

  namespace quantum
  {

    /// Return name of the visitor
    const std::string QuantumBrillianceRemoteVisitor::name() const
    {
      return "quantumbrilliance-remote-visitor";
    }
      
    /// Return description of the visitor
    const std::string QuantumBrillianceRemoteVisitor::description() const
    {
      return "Maps XACC IR to QB XASM, output in JSON format";
    }
      
    /// Normalise angles to the interval [-pi,pi]
    double QuantumBrillianceRemoteVisitor::norm(const double& a)
    {
      if (not restrict_angles_to_pmpi_) return a;
      return std::fmod(a+std::copysign(pi,a),2*pi)-std::copysign(pi,a);
    }

    /**
     * Identity - this is the equivalent of a no-op
     *
     * Input: reference to IR object of class Identity
     *
     * Output: none
     *
     * Effect: push I to the back of JSON object: sequence_
     **/
    //
    // q0: --|I|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Identity &id)
    {
      std::stringstream ss;
      ss << "I"
         << "(" // Identity in XASM format
         << "q"
         << "[" << id.bits()[0] << "]" // target qubit
         << ")";
      sequence_.push_back(ss.str());
    }

    /**
     * Rx(theta) - rotate around the x-axis by theta radians
     *
     * Input: reference to IR object of class Rx
     *
     * Output: none
     *
     * Effect: push Rx to the back of JSON object: sequence_
     **/
    //
    // q0: --|Rx(theta)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Rx &rx)
    {
      std::stringstream ss;
      // IMPORTANT: the XASM grammar only supports the fixed point format real
      // numbers, e.g., "0.0123", not "1.23e-2", hence, we use std::fixed format
      // for the stringstream when constructing XASM.
      ss << std::fixed;
      double angle = norm(mpark::get<double>(rx.getParameter(0)));
      ss << "Rx"
           << "("  // Rx in XASM format
           << "q"
           << "[" << rx.bits()[0] << "]"  // target qubit
           << "," << angle << ")";
      sequence_.push_back(ss.str());
    }

    /**
     * Ry(theta) - rotate around the y-axis by theta radians
     *
     * Input: reference to IR object of class Ry
     *
     * Output: none
     *
     * Effect: push Ry to the back of JSON object: sequence_
     **/
    //
    // q0: --|Ry(theta)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Ry &ry)
    {
      std::stringstream ss; 
      ss << std::fixed;
      double angle = norm(mpark::get<double>(ry.getParameter(0)));
        ss << "Ry"
           << "("  // Ry in XASM format
           << "q"
           << "[" << ry.bits()[0] << "]"  // target qubit
           << "," << angle << ")";
        sequence_.push_back(ss.str());
    }

    /**
     * CZ - controlled Z
     *
     * Input: reference to IR object of class CZ
     *
     * Output: none
     *
     * Effect: push CZ to the back of JSON object: sequence_
     *
     **/
    //
    // q0: ------------|C|--------------
    //
    // q1: ------------|CZ|-------------
    //
    void QuantumBrillianceRemoteVisitor::visit(CZ &cz)
    {
      std::stringstream ss;
      ss << "CZ"
         << "(" // CZ in OpenQASM macro format
         << "q"
         << "[" << cz.bits()[0] << "]," // control qubit
         << "q"
         << "[" << cz.bits()[1] << "]" // target qubit
         << ")";
      sequence_.push_back(ss.str());
    }

    /**
     * Rz(theta) - rotate around the z-axis by theta radians
     *
     * Input: reference to IR object of class Rz
     *
     * Output: none
     *
     * Effect: push Rz to the back of JSON object: sequence_
     **/
    //
    // q0:
    // --|Ry(0.5*pi)--|Rx(theta)|--|Ry(-0.5*pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Rz &rz)
    {
      std::stringstream s1, s2, s3;
      double angle = norm(mpark::get<double>(rz.getParameter(0)));

      s1 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << rz.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")"; // theta=pi/2
      s2 << std::fixed
         << "Rx"
         << "("  // Rx in XASM format
         << "q"
         << "[" << rz.bits()[0] << "]"  // target qubit
         << "," << angle << ")";
      s3 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << rz.bits()[0] << "]" // target qubit
         << "," << (-0.5 * pi) << ")";  // theta=-pi/2
      sequence_.push_back(s1.str());
      sequence_.push_back(s2.str());
      sequence_.push_back(s3.str());
    }

    /**
     * Hadamard  - Hadamard gate
     *
     * Input: reference to IR object of class Hadamard
     *
     * Output: none
     *
     * Effect: push H to the back of JSON object: sequence_
     **/
    //
    // q0: --|Ry(0.5*pi)|--|Rx(pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Hadamard &h)
    {
      std::stringstream s1, s2;

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << h.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")"; // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << h.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());

    }

    /**
     * CNOT - controlled NOT
     *
     * Input: reference to IR object of class CNOT
     *
     * Output: none
     *
     * Effect: push CNOT to the back of JSON object: sequence_
     *
     **/
    // Note: uses QB Escaped Gate Sequences
    //
    // q0: -------------------------|C|----------------------------
    //
    // q1: --|Rx(pi)|--|Ry(0.5*pi)|--|CZ|--|Rx(pi)|--|Ry(0.5*pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(CNOT &cn)
    {
      std::stringstream s1, s2, s3;
      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << cn.bits()[1] << "]" // target qubit
         << "," << (0.5 * pi) << ")";  // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << cn.bits()[1] << "]" // target qubit
         << "," << (pi) << ")";        // theta=pi
      sequence_.push_back(s1.str());

      s3 << "CZ"
         << "(" // CZ in OpenQASM macro format
         << "q"
         << "[" << cn.bits()[0] << "]," // control qubit
         << "q"
         << "[" << cn.bits()[1] << "]" // target qubit
         << ")";
      sequence_.push_back(s3.str());
      sequence_.push_back(s2.str()); // Ry(0.5*pi)
      sequence_.push_back(s1.str()); // Rx(pi)
    }

    /**
     * S - rotate around the z-axis by 0.5*pi
     *
     * Input: reference to IR object of class S
     *
     * Output: none
     *
     * Effect: push S to the back of JSON object: sequence_
     **/
    void QuantumBrillianceRemoteVisitor::visit(S &s)
    {
      std::stringstream s1, s2, s3;

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << s.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")"; // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << s.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());

      s3 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << s.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")"; // theta=pi/2
      sequence_.push_back(s3.str());
      sequence_.push_back(s2.str()); // Ry(0.5*pi)
      sequence_.push_back(s1.str()); // Rx(pi)
    }

    /**
     * Sdg - rotate around the z-axis by -0.5*pi
     *
     * Input: reference to IR object of class Sdg
     *
     * Output: none
     *
     * Effect: push Sdg to the back of JSON object: sequence_
     **/
    void QuantumBrillianceRemoteVisitor::visit(Sdg &sdg)
    {
      std::stringstream s1, s2, s3;

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << sdg.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")";   // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << sdg.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";         // theta=pi
      sequence_.push_back(s1.str());


      s3 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << sdg.bits()[0] << "]" // target qubit
         << "," << (-0.5 * pi) << ")";  // theta=-pi/2
      sequence_.push_back(s3.str());
      sequence_.push_back(s2.str()); // Ry(0.5*pi)
      sequence_.push_back(s1.str()); // Rx(pi)
    }

    /**
     * T - rotate around the z-axis by 0.25*pi
     *
     * Input: reference to IR object of class T
     *
     * Output: none
     *
     * Effect: push T to the back of JSON object: sequence_
     **/
    void QuantumBrillianceRemoteVisitor::visit(T &t)
    {
      std::stringstream s1, s2, s3;

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << t.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")"; // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << t.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());

      s3 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << t.bits()[0] << "]"  // target qubit
         << "," << (0.25 * pi) << ")"; // theta=pi/4
      sequence_.push_back(s3.str());
      sequence_.push_back(s2.str()); // Ry(0.5*pi)
      sequence_.push_back(s1.str()); // Rx(pi)
    }

    /**
     * Tdg - rotate around the z-axis by -0.25*pi
     *
     * Input: reference to IR object of class Tdg
     *
     * Output: none
     *
     * Effect: push Tdg to the back of JSON object: sequence_
     **/
    void QuantumBrillianceRemoteVisitor::visit(Tdg &tdg)
    {
      std::stringstream s1, s2, s3;

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << tdg.bits()[0] << "]" // target qubit
         << "," << (0.5 * pi) << ")";   // theta=pi/2
      sequence_.push_back(s2.str());

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << tdg.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";         // theta=pi
      sequence_.push_back(s1.str());

      s3 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << tdg.bits()[0] << "]" // target qubit
         << "," << (-0.25 * pi) << ")"; // theta=-pi/4
      sequence_.push_back(s3.str());
      sequence_.push_back(s2.str()); // Ry(0.5*pi)
      sequence_.push_back(s1.str()); // Rx(pi)
    }

    /**
     * X - rotate around the x-axis by pi radians
     *
     * Input: reference to IR object of class X
     *
     * Output: none
     *
     * Effect: push X to the back of JSON object: sequence_
     **/
    //
    // q0: --|Rx(pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(X &x)
    {
      std::stringstream s1;

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << x.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());
    }

    /**
     * Y - rotate around the y-axis by pi radians
     *
     * Input: reference to IR object of class Y
     *
     * Output: none
     *
     * Effect: push Y to the back of JSON object: sequence_
     **/
    //
    // q0: --|Ry(pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Y &y)
    {
      std::stringstream s1;

      s1 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << y.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());
    }

    /**
     * Z - rotate around the z-axis by pi radians
     *
     * Input: reference to IR object of class Z
     *
     * Output: none
     *
     * Effect: push Z to the back of JSON object: sequence_
     **/
    //
    // q0: --|Rx(pi)|--|Ry(pi)|--
    //
    void QuantumBrillianceRemoteVisitor::visit(Z &z)
    {
      std::stringstream s1, s2;

      s1 << std::fixed
         << "Rx"
         << "(" // Rx in XASM format
         << "q"
         << "[" << z.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s1.str());

      s2 << std::fixed
         << "Ry"
         << "(" // Ry in XASM format
         << "q"
         << "[" << z.bits()[0] << "]" // target qubit
         << "," << (pi) << ")";       // theta=pi
      sequence_.push_back(s2.str());
    }

    /// Controlled phase gate
    void QuantumBrillianceRemoteVisitor::visit(CPhase &)
    {
      xacc::error("QB SDK does not support: CPhase");
    }

    /// Swap the values of two qubits
    void QuantumBrillianceRemoteVisitor::visit(Swap &s)
    {
      CNOT c1(s.bits()), c2(s.bits()[1], s.bits()[0]), c3(s.bits());
      visit(c1);
      visit(c2);
      visit(c3);
    }

    /**
     * U - rotate in an arbitrary combination of (theta, phi, lambda)
     *
     * U3(theta, phi, lambda) =
     * Ry(-pi/2)*Rx(phi)*Ry(theta)*Rx(lambda)*Ry(pi/2)
     *
     * Input: reference to IR object of class Z
     *
     * Output: none
     *
     * Effect: push U to the back of JSON object: sequence_
     **/
    // q0:
    // --Ry(pi/2)--Rx(lambda)--Ry(theta)--Rx(phi)--Ry(-pi/2)--
    //
    void QuantumBrillianceRemoteVisitor::visit(U &u)
    {
      double theta = norm(mpark::get<double>(u.getParameter(0)));
      double phi = norm(mpark::get<double>(u.getParameter(1)));
      double lambda = norm(mpark::get<double>(u.getParameter(2)));

      // Special case Rx(theta) = U(theta, -0.5pi, 0.5pi)
      double tol = 1e-5;
      if (std::abs(lambda + phi) < tol and std::abs(lambda - 0.5*pi) < tol)
      {
        Rx r(u.bits()[0], theta);
        visit(r);
        return;
      }

      // General case + special cases Ry(theta) = U(theta, 0, 0) and Rz(theta) = U(0, theta, 0) = U (0, 0, theta)
      if (phi != 0. or lambda != 0.) {Ry r(u.bits()[0], 0.5 * pi);  visit(r);}
      if (lambda != 0.)              {Rx r(u.bits()[0], lambda);    visit(r);}
      if (theta != 0.)               {Ry r(u.bits()[0], theta);     visit(r);}
      if (phi != 0.)                 {Rx r(u.bits()[0], phi);       visit(r);}
      if (phi != 0. or lambda != 0.) {Ry r(u.bits()[0], -0.5 * pi); visit(r);}     
    }

    /// Measure a qubit 
    void QuantumBrillianceRemoteVisitor::visit(Measure &m)
    {
      unsigned int qubit = m.bits()[0];
      if (qubit >= nQubits_) xacc::error("Requested to measure qubit that does not exist in this circuit.");  
      qubitToClassicalBitIndex_.insert(std::make_pair(qubit, classicalBitCounter_));
      classicalBitCounter_++;
    }

    /// Return the finished qpu OpenQasm kernel
    std::string QuantumBrillianceRemoteVisitor::getXasmString()
    {
      return sequence_.dump(4);
    }
    
    std::shared_ptr<xacc::CompositeInstruction> QuantumBrillianceRemoteVisitor::getTranspiledIR() const 
    {
      std::stringstream ss;
      ss << "__qpu__ void __temp__xasm__kernel__(qbit q) {\n";
      for (const auto &it : sequence_) {
      ss << it.get<std::string>() << ";\n";
      }
      ss << "}";
      auto xasmCompiler = xacc::getCompiler("xasm");
      return xasmCompiler->compile(ss.str(), nullptr)->getComposites()[0];
    }

  } // namespace quantum

} // namespace xacc

