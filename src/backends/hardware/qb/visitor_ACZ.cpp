// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/backends/hardware/qb/visitor_ACZ.hpp>


namespace xacc
{

  namespace quantum
  {

    /// Return name of the visitor
    const std::string visitor_ACZ::name() const
    {
      return model;
    }

    /// Return description of the visitor
    const std::string visitor_ACZ::description() const
    {
      return "Maps XACC IR to QB XASM in terms of native gates Rx, Ry & ACZ, output in JSON format";
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
    void visitor_ACZ::visit(CZ &cz)
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
    void visitor_ACZ::visit(CNOT &cn)
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
     * CPhase - controlled-phase
     *
     * Input: reference to IR object of class CPhase
     *
     * Output: none
     *
     * Effect: push CPhase to the back of JSON object: sequence_
     **/
    //
    // q0: --|Rx(pi/2)|--|Ry(-theta/2)|--|Rx(-pi/2)|--|C|-------------------|C|--------------------------------
    //                                                 |                     |
    // q1: --|Ry(pi/2)|--|Rx(pi)|---------------------|CZ|--|Rx(-theta/2)|--|CZ|--|Rx(lambda)|--|Ry(-0.5*pi)|--
    //
    // lambda = sign(theta) * (|theta|/2 - pi)
    //
    void visitor_ACZ::visit(CPhase &cphase)
    {
      std::stringstream s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
      double angle = mpark::get<double>(cphase.getParameter(0));
      double lambda = (angle < 0.0 ? -1.0 : 1.0) * (0.5*std::abs(angle) - pi);

      s1 << std::fixed
         << "Rx"
         << "("
         << "q"
         << "[" << cphase.bits()[0] << "]" // control qubit
         << "," << (0.5*pi) << ")";
      sequence_.push_back(s1.str());

      s2 << std::fixed
         << "Ry"
         << "("
         << "q"
         << "[" << cphase.bits()[0] << "]"  // control qubit
         << "," << -0.5*angle << ")";
      sequence_.push_back(s2.str());

      s3 << std::fixed
         << "Rx"
         << "("
         << "q"
         << "[" << cphase.bits()[0] << "]" // control qubit
         << "," << (-0.5*pi) << ")";
      sequence_.push_back(s3.str());

      s4 << std::fixed
         << "Ry"
         << "("
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << "," << (0.5*pi) << ")";
      sequence_.push_back(s4.str());

      s5 << std::fixed
         << "Rx"
         << "("
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << "," << (pi) << ")";
      sequence_.push_back(s5.str());

      s6 << "CZ"
         << "("
         << "q"
         << "[" << cphase.bits()[0] << "]," // control qubit
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << ")";
      sequence_.push_back(s6.str());

      s7 << std::fixed
         << "Rx"
         << "("
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << "," << (-0.5*angle) << ")";
      sequence_.push_back(s7.str());

      s8 << "CZ"
         << "("
         << "q"
         << "[" << cphase.bits()[0] << "]," // control qubit
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << ")";
      sequence_.push_back(s8.str());

      s9 << std::fixed
         << "Rx"
         << "("
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << "," << (lambda) << ")";
      sequence_.push_back(s9.str());

      s10 << std::fixed
         << "Ry"
         << "("
         << "q"
         << "[" << cphase.bits()[1] << "]" // target qubit
         << "," << (-0.5*pi) << ")";
      sequence_.push_back(s10.str());
    }

    /// Swap the values of two qubits
    void visitor_ACZ::visit(Swap &s)
    {
      CNOT c1(s.bits()), c2(s.bits()[1], s.bits()[0]), c3(s.bits());
      visit(c1);
      visit(c2);
      visit(c3);
    }

  }

}

