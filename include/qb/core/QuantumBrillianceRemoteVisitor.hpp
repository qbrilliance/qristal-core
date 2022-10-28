/***
 *** Copyright (c) 2021 Quantum Brilliance Pty Ltd
 ***/
#ifndef QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEREMOTEVISITOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEREMOTEVISITOR_HPP_

#include "AllGateVisitor.hpp"
#include "CommonGates.hpp"
#include "xacc.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <sstream>
using json = nlohmann::json;

namespace xacc {
namespace quantum {

class QuantumBrillianceRemoteVisitor : public AllGateVisitor {
protected:
  double pi = xacc::constants::pi;

  // Number of qubits - use this order to eliminate a warning: -Werror=reorder
  int _nQubits;

  // Tolerance to convert continous angle into discrete angle - use this order to eliminate a warning: -Werror=reorder
  double qb_rtol_;

  /**
   * TBD: ...document the purpose of all member variables...
   **/
  std::map<int, int> qubitToClassicalBitIndex;

  // JSON for sequence of gates (XASM format)
  json sequence_;

  public:
    QuantumBrillianceRemoteVisitor(const int nQubits, const double rtol = 0.01) : _nQubits(nQubits), qb_rtol_(rtol) {}
    QuantumBrillianceRemoteVisitor() : QuantumBrillianceRemoteVisitor(4) {}
  virtual ~QuantumBrillianceRemoteVisitor() {}
    virtual const std::string name() const { return "quantumbrilliance-remote-visitor"; }
  virtual const std::string description() const {
    return "Maps XACC IR to QB XASM, output in JSON format";
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
    * U3(theta, phi, lambda) = Rz(phi)*Rx(-0.5*pi)*Rz(theta)*Rx(0.5*pi)*Rz(lambda)
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
         * Effect: push I to the back of JSON object: sequence_
         **/
        //
        // q0: --|I|--
        //
        void visit(Identity &id) override {
          std::stringstream ss;
          ss << "I"
             << "(" // Identity in XASM format
             << "q"
             << "[" << id.bits()[0] << "]" // target qubit
             << ")";
          sequence_.push_back(ss.str());
        }

        /**
         * Rx(angleStr) - rotate around the x-axis by angleStr radians
         *
         * Input: reference to IR object of class Rx
         *
         * Output: none
         *
         * Effect: push Rx to the back of JSON object: sequence_
         **/
        //
        // q0: --|Rx(angleStr)|--
        //
        void visit(Rx &rx) override {
          std::stringstream ss;
          double w_upperbound = 1 + qb_rtol_;
          double w_lowerbound = 1 - qb_rtol_;
          double angleStr = mpark::get<double>(rx.getParameter(0));

          // pi
          double w_1 = std::abs(angleStr / pi) / (1.0);

          // pi/2
          double w_2 = std::abs(angleStr / pi) / (0.5);

          // pi/4
          double w_4 = std::abs(angleStr / pi) / (0.25);

          // pi/8
          double w_8 = std::abs(angleStr / pi) / (0.125);

          if ((w_lowerbound < w_1) && (w_1 < w_upperbound)) {
            ss << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rx.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 1.0 * pi : -1.0 * pi)
               << ")"; // theta=pi
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_2) && (w_2 < w_upperbound)) {
            ss << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rx.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.5 * pi : -0.5 * pi)
               << ")"; // theta=pi/2
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_4) && (w_4 < w_upperbound)) {
            ss << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rx.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.25 * pi : -0.25 * pi)
               << ")"; // theta=pi/4
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_8) && (w_8 < w_upperbound)) {
            ss << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rx.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.125 * pi : -0.125 * pi)
               << ")"; // theta=pi/8
            sequence_.push_back(ss.str());
          } else {
            std::stringstream sser;
            sser << "ControlOS Rx only supports +/- pi, +/- pi/2, +/- "
                 << "pi/4, +/- pi/8"
                 << " (requested angle, in radians: " << angleStr << ")";
            xacc::error(sser.str());
          }
        }

        /**
         * Ry(angleStr) - rotate around the y-axis by angleStr radians
         *
         * Input: reference to IR object of class Ry
         *
         * Output: none
         *
         * Effect: push Ry to the back of JSON object: sequence_
         **/
        //
        // q0: --|Ry(angleStr)|--
        //
        void visit(Ry &ry) override {
          std::stringstream ss;
          double w_upperbound = 1 + qb_rtol_;
          double w_lowerbound = 1 - qb_rtol_;
          double angleStr = mpark::get<double>(ry.getParameter(0));

          // pi
          double w_1 = std::abs(angleStr / pi) / (1.0);
          // pi/2
          double w_2 = std::abs(angleStr / pi) / (0.5);
          // pi/4
          double w_4 = std::abs(angleStr / pi) / (0.25);
          // pi/8
          double w_8 = std::abs(angleStr / pi) / (0.125);

          if ((w_lowerbound < w_1) && (w_1 < w_upperbound)) {
            ss << "Ry"
               << "(" // Ry in XASM format
               << "q"
               << "[" << ry.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 1.0 * pi : -1.0 * pi)
               << ")"; // theta=pi
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_2) && (w_2 < w_upperbound)) {
            ss << "Ry"
               << "(" // Ry in XASM format
               << "q"
               << "[" << ry.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.5 * pi : -0.5 * pi)
               << ")"; // theta=pi/2
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_4) && (w_4 < w_upperbound)) {
            ss << "Ry"
               << "(" // Ry in XASM format
               << "q"
               << "[" << ry.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.25 * pi : -0.25 * pi)
               << ")"; // theta=pi/4
            sequence_.push_back(ss.str());
          } else if ((w_lowerbound < w_8) && (w_8 < w_upperbound)) {
            ss << "Ry"
               << "(" // Ry in XASM format
               << "q"
               << "[" << ry.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.125 * pi : -0.125 * pi)
               << ")"; // theta=pi/8
            sequence_.push_back(ss.str());
          } else {
            std::stringstream sser;
            sser << "ControlOS Ry only supports +/- pi, +/- pi/2, +/- "
                 << "pi/4, +/- pi/8"
                 << " (requested angle, in radians: " << angleStr << ")";
            xacc::error(sser.str());
          }
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
        void visit(CZ &cz) override {
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
         * Non-native gates
         **/

        /**
         * Rz(angleStr) - rotate around the z-axis by angleStr radians
         *
         * Input: reference to IR object of class Rz
         *
         * Output: none
         *
         * Effect: push Rz to the back of JSON object: sequence_
         **/
        //
        // q0:
        // --|Ry(0.5*pi)--|Rx(pi)||--|Rx(angleStr)|--|Ry(0.5*pi)|--|Rx(pi)|--
        //
        void visit(Rz &rz) override {
          std::stringstream s1, s2, s3;

          double w_upperbound = 1 + qb_rtol_;
          double w_lowerbound = 1 - qb_rtol_;
          double angleStr = mpark::get<double>(rz.getParameter(0));
          // pi
          double w_1 = std::abs(angleStr / pi) / (1.0);
          // pi/2
          double w_2 = std::abs(angleStr / pi) / (0.5);
          // pi/4
          double w_4 = std::abs(angleStr / pi) / (0.25);
          // pi/8
          double w_8 = std::abs(angleStr / pi) / (0.125);

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << rz.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")";  // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << rz.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";        // theta=pi
          sequence_.push_back(s1.str());

          if ((w_lowerbound < w_1) && (w_1 < w_upperbound)) {
            s3 << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rz.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 1.0 * pi : -1.0 * pi)
               << ")"; // theta=pi
            sequence_.push_back(s3.str());
          } else if ((w_lowerbound < w_2) && (w_2 < w_upperbound)) {
            s3 << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rz.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.5 * pi : -0.5 * pi)
               << ")"; // theta=pi/2
            sequence_.push_back(s3.str());
          } else if ((w_lowerbound < w_4) && (w_4 < w_upperbound)) {
            s3 << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rz.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.25 * pi : -0.25 * pi)
               << ")"; // theta=pi/4
            sequence_.push_back(s3.str());
          } else if ((w_lowerbound < w_8) && (w_8 < w_upperbound)) {
            s3 << "Rx"
               << "(" // Rx in XASM format
               << "q"
               << "[" << rz.bits()[0] << "]" // target qubit
               << "," << ((angleStr >= 0.0) ? 0.125 * pi : -0.125 * pi)
               << ")"; // theta=pi/8
            sequence_.push_back(s3.str());
          } else {
            std::stringstream sser;
            sser << "ControlOS Rz only supports +/- pi, +/- pi/2, +/- "
                 << "pi/4, +/- pi/8"
                 << " (requested angle, in radians: " << angleStr << ")";
            xacc::error(sser.str());
          }
          sequence_.push_back(s2.str()); // Ry(0.5*pi)
          sequence_.push_back(s1.str()); // Rx(pi)
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
        void visit(Hadamard &h) override {
          std::stringstream s1, s2;

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << h.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")"; // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
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
        void visit(CNOT &cn) override {
          std::stringstream s1, s2, s3;
          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << cn.bits()[1] << "]" // target qubit
             << "," << (0.5 * pi) << ")";  // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
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

        void visit(S &s) override {
          std::stringstream s1, s2, s3;

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << s.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")"; // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << s.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";       // theta=pi
          sequence_.push_back(s1.str());

          s3 << "Rx"
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

        void visit(Sdg &sdg) override {
          std::stringstream s1, s2, s3;

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << sdg.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")";   // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << sdg.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";         // theta=pi
          sequence_.push_back(s1.str());


          s3 << "Rx"
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

        void visit(T &t) override {
          std::stringstream s1, s2, s3;

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << t.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")"; // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << t.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";       // theta=pi
          sequence_.push_back(s1.str());

          s3 << "Rx"
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

        void visit(Tdg &tdg) override {
          std::stringstream s1, s2, s3;

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << tdg.bits()[0] << "]" // target qubit
             << "," << (0.5 * pi) << ")";   // theta=pi/2
          sequence_.push_back(s2.str());

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << tdg.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";         // theta=pi
          sequence_.push_back(s1.str());

          s3 << "Rx"
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
        void visit(X &x) override {
          std::stringstream s1;

          s1 << "Rx"
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
        void visit(Y &y) override {
          std::stringstream s1;

          s1 << "Ry"
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
        void visit(Z &z) override {
          std::stringstream s1, s2;

          s1 << "Rx"
             << "(" // Rx in XASM format
             << "q"
             << "[" << z.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";       // theta=pi
          sequence_.push_back(s1.str());

          s2 << "Ry"
             << "(" // Ry in XASM format
             << "q"
             << "[" << z.bits()[0] << "]" // target qubit
             << "," << (pi) << ")";       // theta=pi
          sequence_.push_back(s2.str());
        }

        void visit(CPhase &cp) override {
          xacc::error("qbOS does not support: CPhase");
        }

        void visit(Swap &s) override {
          CNOT c1(s.bits()), c2(s.bits()[1], s.bits()[0]), c3(s.bits());
          visit(c1);
          visit(c2);
          visit(c3);
        }

        /**
         * U - rotate in an arbitrary combination of (theta, phi, lambda)
         *
         * U3(theta, phi, lambda) =
         *Rz(phi)*Rx(-0.5*pi)*Rz(theta)*Rx(0.5*pi)*Rz(lambda)
         *
         * Input: reference to IR object of class Z
         *
         * Output: none
         *
         * Effect: push U to the back of JSON object: sequence_
         **/
        // q0:
        // --|Rz(lambda)|--|Rx(0.5*pi)|--|Rz(theta)|--|Rx(-0.5*pi)|--|Rz(phi)|--
        //

        void visit(U &u) override {

          double theta = mpark::get<double>(u.getParameter(0));
          double phi = mpark::get<double>(u.getParameter(1));
          double lambda = mpark::get<double>(u.getParameter(2));
          Rz rz_1(u.bits()[0], lambda);
          Rx rx_1(u.bits()[0], 0.5 * pi);
          Rz rz_2(u.bits()[0], theta);
          Rx rx_2(u.bits()[0], -0.5 * pi);
          Rz rz_3(u.bits()[0], phi);
          visit(rz_1);
          visit(rx_1);
          visit(rz_2);
          visit(rx_2);
          visit(rz_3);
        }

        void visit(Measure &m) override {
          qubitToClassicalBitIndex.insert(
              std::make_pair(m.bits()[0], classicalBitCounter));
          classicalBitCounter++;
        }

        /**
         * Return the finished qpu OpenQasm kernel
         */
        std::string getXasmString() {
          std::string finished = sequence_.dump(4);
          return finished;
        }
};

} // namespace quantum
} // namespace xacc
#endif
