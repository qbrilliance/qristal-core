#include <numbers>

#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/primitives.hpp"

namespace qristal {    

    qristal::CircuitBuilder& Pauli::append_circuit(qristal::CircuitBuilder& cb, const size_t q) const
    {
        switch (symbol_) {
            case Pauli::Symbol::X: {
                cb.RY(q, -1.0 * std::numbers::pi / 2.0);
                break;
            }
            case Pauli::Symbol::Y: {
                cb.RX(q, std::numbers::pi / 2.0);
                break;
            }
            default: {
                //do nothing for I, Z
            }
        }
        return cb;
    }


    template <> 
    Pauli get_identity() {
        return Pauli::Symbol::I;
    }

    std::ostream & operator << (std::ostream & os, const Pauli& p) {
        switch (p.get_symbol()) {
            case Pauli::Symbol::I: {
                os << "I";
                break;
            }
            case Pauli::Symbol::X: {
                os << "X";
                break;
            }
            case Pauli::Symbol::Y: {
                os << "Y";
                break;
            }
            case Pauli::Symbol::Z: {
                os << "Z";
                break;
            }
        }
        return os;
    }
    std::ostream & operator << (std::ostream & os, const std::vector<Pauli>& paulis) {
        for (const auto& i : paulis) {
            os << i;
        }
        return os;
    }    

    qristal::CircuitBuilder& BlochSphereUnitState::append_circuit(qristal::CircuitBuilder& cb, const size_t q) const
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
}