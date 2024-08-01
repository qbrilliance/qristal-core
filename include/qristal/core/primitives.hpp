#pragma once

#include <Eigen/Dense>
#include <unsupported/Eigen/KroneckerProduct>
#include <map>
#include <numeric>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"

namespace qristal {

    class CircuitBuilder;

    /**
    * @brief Helper function returning the sum over all values of an std::map.
    */
    template <typename Key, typename Value>
    inline Value sumMapValues(const std::map<Key, Value>& p) {
        return std::accumulate(p.begin(), p.end(), 0, [](const size_t previous, decltype(*p.begin()) x) { return previous+x.second; });
    }

    /**
    * @brief Concept of matrix translatable symbols, e.g., Pauli basis (I, X, Y, Z).
    *
    * @details This concept enforces a get_matrix member function for templated symbols @tparam Symbol. The tranlsatability of basis symbols to
    * matrix representations is required by the standard quantum state tomography procedure to calculate projections.
    */
    template <typename Symbol>
    concept MatrixTranslatable = requires( Symbol s ) {
        {s.get_matrix()} -> std::same_as<Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>>;
    };

    /**
    * @brief Concept of circuit appendable symbols, e.g., Pauli basis (I, X, Y, Z).
    *
    * @details This concept enforces an append_circuit member function for templated symbols @tparam Symbol. Each basis usable in the standard
    * quantum state tomography workflow is required to have a known basis transformation gate sequence appendable to qristal::CircuitBuilder.
    */
    template <typename Symbol>
    concept CircuitAppendable = requires( Symbol s, CircuitBuilder& cb, const size_t& q ) {
        {s.append_circuit(cb, q)} -> std::same_as<CircuitBuilder&>;
    };

    /**
    * @brief Templated global function to return the identity symbol for a given symbolized basis class @tparam Symbol (e.g. Paulis)
    */
    template <typename Symbol>
    Symbol get_identity();

    /**
    * @brief Concept of symbolized basis classes possessing an identity, e.g., Pauli (I, X, Y, Z).
    *
    * @details This concept enforces the existence of get_identity<Symbol> for templated symbols @tparam Symbol.
    */
    template <typename Symbol>
    concept HasIdentity = requires() {
        {get_identity<Symbol>()} -> std::same_as<Symbol>;
    };

    /**
    * @brief Helper function to convert any unsigned integer into a number of a given base and minimal length.
    *
    * Arguments:
    * @param number the unsigned integer to convert
    * @param base the targeted base of the converted number
    * @param min_length the minimal lenght of the converted number.
    *
    * @return std::vector<size_t> the converted number represented as a std::vector.
    */
    inline std::vector<size_t> convert_decimal(const size_t number, const size_t base, const size_t min_length)
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

    /**
    * @brief Calculate the tensor (Kronecker) product of a given vector of matrix translatable symbols.
    *
    * Arguments:
    * @param symbol_list a std::vector of matrix translatable Symbols.
    *
    * @return Eigen::Matrix a dense complex matrix containing the tensor (Kronecker) product of all given symbols.
    *
    * @details This global function consecutively envokes Eigen::kroneckerProduct on all given matrix translatable symbols (via get_matrix) to build the tensor (Kronecker) product.
    */
    template <MatrixTranslatable MatrixSymbol_>
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> calculate_Kronecker_product(const std::vector<MatrixSymbol_>& symbol_list) {
        //initialize result as dynamic matrix from first matrix in symbol list
        Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> result(1, 1);
        result(0, 0) = std::complex<double>(1.0, 0.0);
        for (const auto& symbol : symbol_list | std::views::reverse) {
            result = Eigen::kroneckerProduct(result, symbol.get_matrix()).eval();
        }
        return result;
    }

    /**
    * @brief Convenient handler for the standard Pauli measurement basis.
    *
    * @details This class builds upon the I, X, Y, Z symbols to define a convenient handler for the standard Pauli measurement basis.
    */
    class Pauli {
        public:
            /**
            * @brief The usable symbols of type Pauli::Symbol denoting Pauli I, X, Y, and Z matrices.
            */
            enum class Symbol {I, X, Y, Z};

            /**
            * @brief Constructor for Pauli object from given @param symbol of type Pauli::Symbol.
            */
            constexpr Pauli(const Symbol& symbol) : symbol_(symbol) {}
            /**
            * @brief Equality comparison operator for Pauli symbols.
            */
            bool operator == (const Pauli& p) const {
                return symbol_ == p.get_symbol();
            }
            /**
            * @brief Translate the Pauli symbol into its matrix representation.
            *
            * Arguments: ---
            *
            * @return Eigen::Matrix a dense complex matrix corresponding to the representation of the Pauli symbol.
            */
            Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> get_matrix() const
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
            /**
            * @brief Append a given quantum circuit by rotation gates transforming to the respective Pauli symbol measurement basis.
            *
            * Arguments:
            * @param cb the quantum circuit to be appended given as a qristal::CircuitBuilder object
            * @param q the unsigned integer qubit index on which the rotation gates are applied.
            *
            * @return qristal::Circuitbuilder reference to the appended circuit.
            */
            qristal::CircuitBuilder& append_circuit(qristal::CircuitBuilder& cb, const size_t q) const;

            /**
            * @brief Return a constant reference to the wrapped symbol.
            */
            const Symbol& get_symbol() const {return symbol_;}

        private:
            Symbol symbol_;

    };
    //explicitly instantiate identity
    template <>
    Pauli get_identity();
    /**
    * @brief Helper function to print Pauli symbols to std::ostream by overloading the << operator.
    *
    * Arguments:
    * @param os the std::ostream where the output is directed
    * @param p the Pauli symbol to print.
    *
    * @return std::ostream reference to the output stream.
    */
    std::ostream & operator << (std::ostream & os, const Pauli& p);
    /**
    * @brief Helper function to print std::vector of Pauli symbols to std::ostream by overloading the << operator.
    *
    * Arguments:
    * @param os the std::ostream where the output is directed
    * @param paulis the std::vector of Pauli symbols to print.
    *
    * @return std::ostream reference to the output stream.
    */
    std::ostream & operator << (std::ostream & os, const std::vector<Pauli>& paulis);

    /**
    * @brief Convenient handler for the unit Bloch sphere unit input states.
    *
    * @details This class builds upon the Z+, Z-, X+, X-, Y+, and Y- symbols to define a convenient handler for the Bloch sphere unit input states.
    */
    class BlochSphereUnitState {
        public:
            /**
            * @brief The usable symbols of type BlochSphereUnitState::Symbol denoting unit states along the direction of the three Bloch sphere axes.
            */
            enum class Symbol {Zp, Zm, Xp, Xm, Yp, Ym};

            /**
            * @brief Constructor for BlochSphereUnitState objects from given @param symbol of type BlochSphereUnitState::Symbol.
            */
            constexpr BlochSphereUnitState(const Symbol& symbol) : symbol_(symbol) {}

            /**
            * @brief Translate the BlochSphereUnitState symbol into its matrix representation.
            *
            * Arguments: ---
            *
            * @return Eigen::Matrix a dense complex matrix corresponding to the representation of the BlochSphereUnitState symbol.
            */
            Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> get_matrix() const
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
            /**
            * @brief Prepend a given quantum circuit by rotation gates initializing the respective BlochSphereUnitState symbol input basis.
            *
            * Arguments:
            * @param cb the quantum circuit to be appended given as a qristal::CircuitBuilder object
            * @param q the unsigned integer qubit index on which the rotation gates are applied.
            *
            * @return qristal::Circuitbuilder reference to the preprended circuit.
            */
            qristal::CircuitBuilder& append_circuit(qristal::CircuitBuilder& cb, const size_t q) const;
            /**
            * @brief Return a constant reference to the wrapped symbol.
            */
            const Symbol& get_symbol() const {return symbol_;}

        private:
            Symbol symbol_{};

    };
    /**
    * @brief Helper function to print BlochSphereUnitState symbols to std::ostream by overloading the << operator.
    *
    * Arguments:
    * @param os the std::ostream where the output is directed
    * @param bsu the BlochSphereUnitState symbol to print.
    *
    * @return std::ostream reference to the output stream.
    */
    std::ostream & operator << (std::ostream & os, const BlochSphereUnitState& bsu);
    /**
    * @brief Helper function to print std::vector of BlochSphereUnitState symbols to std::ostream by overloading the << operator.
    *
    * Arguments:
    * @param os the std::ostream where the output is directed
    * @param bsus the std::vector of BlochSphereUnitState symbols to print.
    *
    * @return std::ostream reference to the output stream.
    */
    std::ostream & operator << (std::ostream & os, const std::vector<BlochSphereUnitState>& bsus);

    /**
    * @brief Calculate the tensor (Kronecker) product of a given vector of matrix translatable symbols by implicitly calculating the string of basis symbols from a given index.
    *
    * Arguments:
    * @param index the unsigned integer index of the n-qubit basis symbol string to be constructed
    * @param basis a std::vector of matrix translatable Symbols
    * @param basis_string_length the length of the basis string to be constructed from the unsigned integer index.
    *
    * @return Eigen::Matrix a dense complex matrix containing the tensor (Kronecker) product of all given symbols.
    */
    template <typename Symbol>
    Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> build_up_matrix_by_Kronecker_product(const size_t index, const std::vector<Symbol>& basis, const size_t basis_string_length) {
        //first convert index to x-nary number to find the basis symbol for each repeat
        std::vector<size_t> indices = convert_decimal(index, basis.size(), basis_string_length);
        std::vector<Symbol> vec;
        for (const auto& i : indices) {
            vec.push_back(basis[i]);
        }
        return calculate_Kronecker_product<Symbol>(vec);
    }
}
