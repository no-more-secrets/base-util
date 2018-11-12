/****************************************************************
* Type aliases
****************************************************************/
#pragma once

// TODO: need to figure out how to forward declare the relevant
//       classes.

// Aliases here are made in the global namespace and  are  simply
// to save typing.

#include <chrono>
#include <experimental/filesystem>
#include <functional>
#include <optional>
#include <utility>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace fs = std::experimental::filesystem;

using OptStr      = std::optional<std::string>;
using OptPath     = std::optional<fs::path>;
using StrVec      = std::vector<std::string>;
using SVVec       = std::vector<std::string_view>;
using PathRef     = std::reference_wrapper<fs::path>;
using PathCRef    = std::reference_wrapper<fs::path const>;
using PathVec     = std::vector<fs::path>;
using PathCRefVec = std::vector<PathCRef>;

template<typename T>
using Ref = std::reference_wrapper<T>;

template<typename T>
using CRef = std::reference_wrapper<T const>;

template<typename T>
using OptRef  = std::optional<std::reference_wrapper<T>>;

template<typename T>
using OptCRef = std::optional<std::reference_wrapper<T const>>;

template<typename U, typename V>
using PairVec = std::vector<std::pair<U, V>>;

using SysTimePoint = std::chrono::system_clock::time_point;
