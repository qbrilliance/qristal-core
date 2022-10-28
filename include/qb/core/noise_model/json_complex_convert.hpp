// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#pragma once
#include <complex>
#include <vector>
#include <nlohmann/json.hpp>

//============================================================================
// JSON Conversion for complex STL types (std::complex)
// Adapted from Qiskit 
//============================================================================
/**
 *
 * (C) Copyright IBM 2018, 2019.
 *
 * This code is licensed under the Apache License, Version 2.0. You may
 * obtain a copy of this license in the LICENSE.txt file in the root directory
 * of this source tree or at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * Any modifications or derivative works of this code must retain this
 * copyright notice, and modified files need to carry a notice indicating
 * that they have been altered from the originals.
 */
namespace std
{
    /**
     * Convert a complex number to a json list z -> [real(z), imag(z)].
     * @param js a nlohmann::json object to contain converted type.
     * @param z a complex number to convert.
     */
    template <typename RealType>
    void to_json(nlohmann::json &js, const std::complex<RealType> &z)
    {
        js = std::pair<RealType, RealType>{z.real(), z.imag()};
    }

    /**
     * Convert a JSON value to a complex number z. If the json value is a float
     * it will be converted to a complex z = (val, 0.). If the json value is a
     * length two list it will be converted to a complex z = (val[0], val[1]).
     * @param js a nlohmann::json object to convert.
     * @param z a complex number to contain result.
     */
    template <typename RealType>
    void from_json(const nlohmann::json &js, std::complex<RealType> &z)
    {
        if (js.is_number())
            z = std::complex<RealType>{js.get<RealType>()};
        else if (js.is_array() && js.size() == 2)
        {
            z = std::complex<RealType>{js[0].get<RealType>(), js[1].get<RealType>()};
        }
        else
        {
            throw std::invalid_argument(std::string("JSON: invalid complex number"));
        }
    }

    /**
     * Convert a complex vector to a json list
     * v -> [ [real(v[0]), imag(v[0])], ...]
     * @param js a nlohmann::json object to contain converted type.
     * @param vec a complex vector to convert.
     */
    template <typename RealType>
    void to_json(nlohmann::json &js,
                 const std::vector<std::complex<RealType>> &vec)
    {
        std::vector<std::vector<RealType>> out;
        for (auto &z : vec)
        {
            out.push_back(std::vector<RealType>{real(z), imag(z)});
        }
        js = out;
    }

    /**
     * Convert a JSON list to a complex vector. The input JSON value may be:
     * - an object with complex pair values: {'00': [re, im], ... }
     * - an object with real pair values: {'00': n, ... }
     * - an list with complex values: [ [a0re, a0im], ...]
     * - an list with real values: [a0, a1, ....]
     * @param js a nlohmann::json object to convert.
     * @param vec a complex vector to contain result.
     */
    template <typename RealType>
    void from_json(const nlohmann::json &js,
                   std::vector<std::complex<RealType>> &vec)
    {
        std::vector<std::complex<RealType>> ret;
        if (js.is_array())
        {
            for (auto &elt : js)
                ret.push_back(elt);
            vec = ret;
        }
        else
        {
            throw std::invalid_argument(std::string("JSON: invalid complex vector."));
        }
    }
} // end namespace std.