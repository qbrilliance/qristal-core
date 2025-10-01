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
     * ACZ - anti-controlled Z
     *
     * Input: q0: control bit
     *        q1: target bit
     *
     * Output: ACZ gate string in QB extended XASM format
     *
     **/
    std::string acz(uint q0, uint q1)
    {
      std::stringstream ss;
      // The third optional argument to CZ() is the control state. It is used to select a specific implementation of the CZ gate
      // on hardware. This argument is not standard XASM, but a QB-specific extension defined and recognised only by qcstack. There
      // are four possible values: c[00], c[01], c[10] and c[11]. These correspond to quantum numbers of specific transitions
      // (0 --> 0, 0 --> 1, etc) that may be used to implement a CZ-like gate. c[01] is the highest fidelity option, and produces an
      // anti-controlled CZ gate, i.e. a gate where 0 on the control qubit causes a Z gate to be applied to the target qubit, and 1
      // on the control qubit causes the target qubit to be left unaltered.
      ss << "CZ(q[" << q0 << "],q[" << q1 << "],c[01])";
      return ss.str();
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
    // q0: -------|X|--|AC|--|X|-----------
    //                  |
    // q1: ------------|Z|-----------------
    //
    void visitor_ACZ::visit(CZ &cz)
    {
      X x(cz.bits()[0]);
      visitor::visit(x);
      sequence_.push_back(acz(cz.bits()[0], cz.bits()[1]));
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
    // q0: --------------|Z|----------------
    //                    |
    // q1: --|Ry(pi/2)|--|AC|--|Ry(-pi/2)|--
    //
    void visitor_ACZ::visit(CNOT &cn)
    {
      Ry r1(cn.bits()[1], 0.5*pi);
      Ry r2(cn.bits()[1], -0.5*pi);
      visitor::visit(r1);
      sequence_.push_back(acz(cn.bits()[1], cn.bits()[0]));
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
    // q0: --|Rx(pi/2)|--|Ry(-theta/2)|--|Rx(-pi/2)|--|Z|-------------------|Z|--------------------------------
    //                                                 |                     |
    // q1: --|Ry(pi/2)|-------------------------------|AC|--|Rx(-theta/2)|--|AC|--|Rx(theta/2)|--|Ry(-pi/2)|--
    //
    void visitor_ACZ::visit(CPhase &cphase)
    {
      double angle = mpark::get<double>(cphase.getParameter(0));
      Rx r1(cphase.bits()[0], 0.5 * pi);
      Ry r2(cphase.bits()[0], -0.5 * angle);
      Rx r3(cphase.bits()[0], -0.5 * pi);
      Ry r4(cphase.bits()[1], 0.5 * pi);
      Rx r5(cphase.bits()[1], -0.5 * angle);
      Rx r6(cphase.bits()[1], 0.5*angle);
      Ry r7(cphase.bits()[1], -0.5 * pi);
      visitor::visit(r1);
      visitor::visit(r2);
      visitor::visit(r3);
      visitor::visit(r4);
      sequence_.push_back(acz(cphase.bits()[1], cphase.bits()[0]));
      visitor::visit(r5);
      sequence_.push_back(acz(cphase.bits()[1], cphase.bits()[0]));
      visitor::visit(r6);
      visitor::visit(r7);
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
    // q0: --|Rx(pi/2)|---|Z|---|Rx(pi/2)|--|Z|---|Rx(pi/2)|--|Z|--------
    //                     |                 |                 |
    // q1: --|Ry(-pi/2)|--|AC|--|Rx(pi/2)|--|AC|--|Rx(pi/2)|--|AC|--|X|--
    void visitor_ACZ::visit(Swap &s)
    {
      Rx r1(s.bits()[0], 0.5 * pi);
      Rx r2(s.bits()[1], -0.5 * pi);
      Rx r3(s.bits()[1], 0.5 * pi);
      X x(s.bits()[1]);
      visitor::visit(r1);
      visitor::visit(r2);
      sequence_.push_back(acz(s.bits()[1], s.bits()[0]));
      for (int i=0; i<2; i++) {
        visitor::visit(r1);
        visitor::visit(r3);
        sequence_.push_back(acz(s.bits()[1], s.bits()[0]));
      }
      visitor::visit(x);
    }

  }

}

