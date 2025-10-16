// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/backends/hardware/qb/visitor_CZ.hpp>


namespace xacc
{

  namespace quantum
  {

    /// Return name of the visitor
    const std::string visitor_CZ::name() const
    {
      return model;
    }

    /// Return description of the visitor
    const std::string visitor_CZ::description() const
    {
      return "Maps XACC IR to QB XASM in terms of native gates Rx, Ry & CZ, output in JSON format";
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
    //                  |
    // q1: ------------|Z|--------------
    //
    void visitor_CZ::visit(CZ &cz)
    {
      std::stringstream ss;
      ss << "CZ"
         << "(" // CZ in XASM
         << "q"
         << "[" << cz.bits()[0] << "]," // control qubit
         << "q"
         << "[" << cz.bits()[1] << "]" // target qubit
         << ")";
      sequence_.push_back(ss.str());
    }

    /**
     * ACZ - anti-controlled Z
     *
     * Input: reference to IR object of class ACZ
     *
     * Output: none
     *
     * Effect: push ACZ to the back of JSON object: sequence_
     *
     **/
    //
    // q0: -------|X|--|C|--|X|---------
    //                  |
    // q1: ------------|Z|--------------
    //
    void visitor_CZ::visit(ACZ &acz)
    {
      X x(acz.bits()[0]);
      CZ cz(acz.bits()[0], acz.bits()[1]);
      visitor::visit(x);
      visit(cz);
      visitor::visit(x);
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
    //
    // q0: --------------------------|C|---------------------------
    //                                |
    // q1: --|Ry(0.5*pi)|--|Rx(pi)|--|Z|--|Ry(0.5*pi)|--|Rx(pi)|--
    //
    void visitor_CZ::visit(CNOT &cn)
    {
      Ry r1(cn.bits()[1], 0.5 * pi);
      Rx r2(cn.bits()[1], pi);
      CZ cz(cn.bits()[0], cn.bits()[1]);
      visitor::visit(r1);
      visitor::visit(r2);
      visit(cz);
      visitor::visit(r1);
      visitor::visit(r2);
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
    // q0: --|Rx(pi/2)|--|Ry(-theta/2)|--|Rx(-pi/2)|--|C|------------------|C|--------------------------------
    //                                                 |                    |
    // q1: --|Ry(pi/2)|--|Rx(pi)|---------------------|Z|--|Rx(-theta/2)|--|Z|--|Rx(lambda)|--|Ry(-0.5*pi)|--
    //
    // lambda = sign(theta) * (|theta|/2 - pi)
    //
    void visitor_CZ::visit(CPhase &cphase)
    {
      double angle = mpark::get<double>(cphase.getParameter(0));
      double lambda = (angle < 0.0 ? -1.0 : 1.0) * (0.5*std::abs(angle) - pi);
      Rx r1(cphase.bits()[0], 0.5 * pi);
      Ry r2(cphase.bits()[0], -0.5 * angle);
      Rx r3(cphase.bits()[0], -0.5 * pi);
      Ry r4(cphase.bits()[1], 0.5 * pi);
      Rx r5(cphase.bits()[1], pi);
      Rx r6(cphase.bits()[1], -0.5 * angle);
      Rx r7(cphase.bits()[1], lambda);
      Ry r8(cphase.bits()[1], -0.5 * pi);
      CZ cz(cphase.bits()[0], cphase.bits()[1]);
      visitor::visit(r1);
      visitor::visit(r2);
      visitor::visit(r3);
      visitor::visit(r4);
      visitor::visit(r5);
      visit(cz);
      visitor::visit(r6);
      visit(cz);
      visitor::visit(r7);
      visitor::visit(r8);
    }

    /**
     * Swap - Swap the values of two qubits
     *
     * Input: reference to IR object of class Swap
     *
     * Output: none
     *
     * Effect: push Swap to the back of JSON object: sequence_
     **/
    //
    // q0: --|Rx(pi/2)|--|C|--|Rx(pi/2)|--|C|--|Rx(pi/2)|--|C|--
    //                    |                |                |
    // q1: --|Ry(pi/2)|--|Z|--|Rx(pi/2)|--|Z|--|Rx(pi/2)|--|Z|--
    void visitor_CZ::visit(Swap &s)
    {
      Rx r1(s.bits()[0], 0.5 * pi);
      Rx r2(s.bits()[1], 0.5 * pi);
      CZ cz(s.bits()[0], s.bits()[1]);
      for (int i=0; i<3; i++) {
        visitor::visit(r1);
        visitor::visit(r2);
        visit(cz);
      }
    }

  }

}

