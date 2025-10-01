// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/backends/hardware/qb/visitor.hpp>

#include <cmath>

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

    /// Normalise angles to the interval [-pi,pi]
    double visitor::norm(const double& a)
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
    void visitor::visit(Identity &id)
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
    void visitor::visit(Rx &rx)
    {
      double angle = norm(mpark::get<double>(rx.getParameter(0)));
      // IMPORTANT: the XASM grammar only supports the fixed point format real
      // numbers, e.g., "0.0123", not "1.23e-2", hence, we use std::fixed format
      // for the stringstream when constructing XASM.
      std::stringstream ss;
      ss << std::fixed
         << "Rx"
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
    void visitor::visit(Ry &ry)
    {
      double angle = norm(mpark::get<double>(ry.getParameter(0)));
      // IMPORTANT: the XASM grammar only supports the fixed point format real
      // numbers, e.g., "0.0123", not "1.23e-2", hence, we use std::fixed format
      // for the stringstream when constructing XASM.
      std::stringstream ss;
      ss << std::fixed
         << "Ry"
         << "("  // Ry in XASM format
         << "q"
         << "[" << ry.bits()[0] << "]"  // target qubit
         << "," << angle << ")";
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
    void visitor::visit(Rz &rz)
    {
      Ry r1(rz.bits()[0], 0.5 * pi);
      Rx r2(rz.bits()[0], norm(mpark::get<double>(rz.getParameter(0))));
      Ry r3(rz.bits()[0], -0.5 * pi);
      visit(r1);
      visit(r2);
      visit(r3);
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
    void visitor::visit(Hadamard &h)
    {
      Ry r1(h.bits()[0], 0.5 * pi);
      Rx r2(h.bits()[0], pi);
      visit(r1);
      visit(r2);
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
    void visitor::visit(S &s)
    {
      Rz r(s.bits()[0], 0.5 * pi);
      visit(r);
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
    void visitor::visit(Sdg &sdg)
    {
      Rz r(sdg.bits()[0], -0.5 * pi);
      visit(r);
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
    void visitor::visit(T &t)
    {
      Rz r(t.bits()[0], 0.25 * pi);
      visit(r);
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
    void visitor::visit(Tdg &tdg)
    {
      Rz r(tdg.bits()[0], -0.25 * pi);
      visit(r);
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
    void visitor::visit(X &x)
    {
      Rx r(x.bits()[0], pi);
      visit(r);
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
    void visitor::visit(Y &y)
    {
      Ry r(y.bits()[0], pi);
      visit(r);
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
    void visitor::visit(Z &z)
    {
      Rx r1(z.bits()[0], pi);
      Ry r2(z.bits()[0], pi);
      visit(r1);
      visit(r2);
    }

    /**
     * U - rotate in an arbitrary combination of (theta, phi, lambda)
     *
     * U3(theta, phi, lambda) = Rx(alpha)*Ry(beta)*Ry(gamma)
     * for appropriate alpha, beta, gamma as functions of theta, phi, lambda.
     * Note that this also introduces a global phase delta = (lambda + phi)/2.
     *
     * Input: reference to IR object of class Z
     *
     * Output: none
     *
     * Effect: push U to the back of JSON object: sequence_.
     **/
    //
    // q0: --|Rx(t1+t2)|--|Ry(beta)|--|Rx(t1-t2)|--
    // See code for definitions of t1, t2 and beta.
    //
    void visitor::visit(U &u)
    {
      using namespace std;
      double theta = mpark::get<double>(u.getParameter(0));
      double phi = mpark::get<double>(u.getParameter(1));
      double lambda = mpark::get<double>(u.getParameter(2));

      double tol = 1e-5;
      // Special case Rx(theta) = U(theta, -0.5pi, 0.5pi)
      if (std::abs(lambda + phi) < tol and std::abs(lambda - 0.5*pi) < tol)
      {
        Rx r(u.bits()[0], theta);
        visit(r);
        return;
      }
      // Special case Ry(theta) = U(theta, 0, 0)
      if (std::abs(phi) < tol and std::abs(lambda) < tol)
      {
        Ry r(u.bits()[0], theta);
        visit(r);
        return;
      }

      // General case + special case Rz(theta) = U(0, theta, 0) = U (0, 0, theta).
      //
      // The following expressions follow from setting
      //                          ╭                                                                                ╮
      // U3(theta, phi, lambda) = |         cos(0.5*theta)           -exp(1.0im*lambda)*sin(0.5*theta)             | = exp(1.0im*delta)*Rx(alpha)*Ry(beta)*Rx(gamma)
      //                          |  exp(1.0im*phi)*sin(0.5*theta)    exp(1.0im*lambda + 1.0im*phi)*cos(0.5*theta) |
      //                          ╰                                                                                ╯
      // and then solving for alpha, beta, gamma and delta, where delta is a global phase that we can eventually ignore. The
      // derivation is long and not very illuminating, but comes from solving the 8 implied equations (4 matrix entries, each
      // with real and imaginary parts) simultaneously. The solution turns out to be of the form alpha = t1 - t2, gamma = t1 + t2.
      double delta = 0.5*(lambda + phi);
      double delta_prime = 0.5*(lambda - phi);
      double beta = 2.0*acos(pow(pow(cos(delta),2)*pow(cos(0.5*theta),2) + pow(sin(delta_prime),2)*pow(sin(0.5*theta),2),0.5));
      double term1 = atan(sin(delta_prime)*tan(0.5*theta)/cos(delta));
      double term2 = atan(-sin(delta)/(cos(delta_prime)*tan(0.5*theta)));
      if (cos(delta)*cos(0.5*theta)/(cos(0.5*beta)*cos(term1)) < 0) term1 -= copysign(pi, term1);
      if (cos(delta_prime)*sin(0.5*theta)/(sin(0.5*beta)*cos(term2)) < 0) term2 -= copysign(pi, term2);
      Rx r1(u.bits()[0], term1 + term2);
      Ry r2(u.bits()[0], beta);
      Rx r3(u.bits()[0], term1 - term2);
      visit(r1);
      visit(r2);
      visit(r3);
    }

    /// Measure a qubit
    void visitor::visit(Measure &m)
    {
      unsigned int qubit = m.bits()[0];
      if (qubit >= nQubits_) xacc::error("Requested to measure qubit that does not exist in this circuit.");
      qubitToClassicalBitIndex_.insert(std::make_pair(qubit, classicalBitCounter_));
      classicalBitCounter_++;
    }

    /// Return the finished qpu OpenQasm kernel
    std::string visitor::getXasmString()
    {
      return sequence_.dump(4);
    }

    std::shared_ptr<xacc::CompositeInstruction> visitor::getTranspiledIR() const
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

