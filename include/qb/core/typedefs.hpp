// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#pragma once
#include <vector>
#include <map>
#include <string>
#include <complex>
#include <nlohmann/json.hpp>

/// Define typedefs for qbOS Python bindings
namespace qbOS {
using N            = std::vector<size_t>;
using VectorN      = std::vector<N>;

using String       = std::vector<std::string>;
using VectorString = std::vector<String>;

using Bool         = std::vector<bool>;
using VectorBool   = std::vector<Bool>;

using NN           = std::map<int,int>;
using MapNN        = std::vector<NN>;
using VectorMapNN  = std::vector<MapNN>;

using NC           = std::map<int,std::complex<double>>;
using MapNC        = std::vector<NC>;
using VectorMapNC  = std::vector<MapNC>;

using ND           = std::map<int,double>;
using MapND        = std::vector<ND>;
using VectorMapND  = std::vector<MapND>;

using json = nlohmann::json;
} // namespace qbOS
