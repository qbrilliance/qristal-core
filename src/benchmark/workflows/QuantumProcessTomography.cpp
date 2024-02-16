#include "qb/core/benchmark/workflows/QuantumProcessTomography.hpp"
#include <numbers>


namespace qb 
{
    namespace benchmark 
    {

        Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> BlochSphereUnitState::get_matrix() const 
        {
            Eigen::Matrix<std::complex<double>, 2, 2> mat;
            switch (symbol_) {
                case BlochSphereUnitState::Symbol::Zp: {
                    mat << std::complex<double>(1, 0),  std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Zm: {
                    mat << std::complex<double>(0, 0),  std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Xp: {
                    //created by Ry(+pi/2)|0>
                    mat << std::complex<double>(0.5, 0),  std::complex<double>(0.5, 0), std::complex<double>(0.5, 0), std::complex<double>(0.5, 0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Xm: {
                    //created by Ry(-pi/2)|0>
                    mat << std::complex<double>(0.5, 0),  std::complex<double>(-0.5, 0), std::complex<double>(-0.5, 0), std::complex<double>(0.5, 0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Yp: {
                    //created by Rx(-pi/2)|0>
                    mat << std::complex<double>(0.5, 0),  std::complex<double>(0, -0.5), std::complex<double>(0, 0.5), std::complex<double>(0.5, 0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Ym: {
                    //created by Rx(pi/2)|0>
                    mat << std::complex<double>(0.5, 0),  std::complex<double>(0, 0.5), std::complex<double>(0, -0.5), std::complex<double>(0.5, 0);
                    break;
                }
            }
            return mat;
        }

        qb::CircuitBuilder& BlochSphereUnitState::append_circuit(qb::CircuitBuilder& cb, const size_t q) const
        {
            switch (symbol_) {
                case BlochSphereUnitState::Symbol::Zm: {
                    cb.X(q);
                    break;
                }
                case BlochSphereUnitState::Symbol::Xp: {
                    cb.RY(q, std::numbers::pi / 2.0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Xm: {
                    cb.RY(q, -1.0 * std::numbers::pi / 2.0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Yp: {
                    cb.RX(q, -1.0 * std::numbers::pi / 2.0);
                    break;
                }
                case BlochSphereUnitState::Symbol::Ym: {
                    cb.RX(q, std::numbers::pi / 2.0);
                    break;
                }
                default: {
                    //do nothing for Zp
                    break;
                }
            }
            return cb;
        }

        std::ostream & operator << (std::ostream & os, const BlochSphereUnitState& bsu) {
            switch (bsu.get_symbol()) {
                case BlochSphereUnitState::Symbol::Zp: {
                    os << "Z+";
                    break;
                }
                case BlochSphereUnitState::Symbol::Zm: {
                    os << "Z-";
                    break;
                }
                case BlochSphereUnitState::Symbol::Xp: {
                    os << "X+";
                    break;
                }
                case BlochSphereUnitState::Symbol::Xm: {
                    os << "X-";
                    break;
                }
                case BlochSphereUnitState::Symbol::Yp: {
                    os << "Y+";
                    break;
                }
                case BlochSphereUnitState::Symbol::Ym: {
                    os << "Y-";
                    break;
                }
            }
            return os;
        }
        std::ostream & operator << (std::ostream & os, const std::vector<BlochSphereUnitState>& bsus) {
            for (const auto& i : bsus) {
                os << i;
            }
            return os;
        }


        std::complex<double> HilbertSchmidtInnerProduct(const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>& a, const Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>& b)
        {
            return (a.adjoint() * b).trace();
        }

    }
}