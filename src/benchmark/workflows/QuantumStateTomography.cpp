// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "qb/core/benchmark/workflows/QuantumStateTomography.hpp"

#include <unsupported/Eigen/KroneckerProduct>
#include <ranges>


namespace qb 
{
    namespace benchmark 
    {
        Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> Pauli::get_matrix() const 
        {
            Eigen::Matrix<std::complex<double>, 2, 2> mat;
            switch (symbol_) {
                case Pauli::Symbol::I: {
                    mat << std::complex<double>(1, 0),  std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0);
                    break;
                }
                case Pauli::Symbol::X: {
                    mat << std::complex<double>(0, 0),  std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(0, 0);
                    break;
                }
                case Pauli::Symbol::Y: {
                    mat << std::complex<double>(0, 0), std::complex<double>(0, -1), std::complex<double>(0, 1), std::complex<double>(0, 0);
                    break;
                }
                case Pauli::Symbol::Z: {
                    mat << std::complex<double>(1, 0),  std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(-1, 0);
                    break;
                }
            }
            return mat;
        }

        qb::CircuitBuilder& Pauli::append_circuit(qb::CircuitBuilder& cb, const size_t q) const
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

        std::vector<size_t> convert_decimal(const size_t number, const size_t base, const size_t min_length)
        {
            std::vector<size_t> result(min_length, 0); 
            size_t index = 0;
            size_t curr = number / base;
            result[index] = number % base; 
            while (curr > 0)
            {
                result[++index] = curr % base; 
                curr = curr / base; 
            } 
            return result;
        }

    }
}