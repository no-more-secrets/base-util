/****************************************************************
* Colors
****************************************************************/
#pragma once

#include <string_view>

namespace util {

inline constexpr std::string_view c_norm   = "\033[00m";
inline constexpr std::string_view c_green  = "\033[32m";
inline constexpr std::string_view c_red    = "\033[31m";
inline constexpr std::string_view c_yellow = "\033[33m";

} // namespace util
