/****************************************************************
** Template Metaprogramming Utilities
*****************************************************************/
#pragma once

// C++ standard library
#include <optional>
#include <vector>

namespace mp {

template<bool...>
constexpr const bool and_v = false;

template<bool Bool>
constexpr const bool and_v<Bool> = Bool;

template<bool First, bool... Bools>
constexpr bool and_v<First, Bools...> = First&& and_v<Bools...>;

static_assert( and_v<true> == true );
static_assert( and_v<false> == false );
static_assert( and_v<true, false> == false );
static_assert( and_v<false, true> == false );
static_assert( and_v<true, true> == true );
static_assert( and_v<false, false> == false );
static_assert( and_v<true, true, true> == true );
static_assert( and_v<true, true, false> == false );

template<typename T>
constexpr bool is_optional_v = false;

template<typename T>
constexpr bool is_optional_v<std::optional<T>> = true;

template<typename T, typename...>
constexpr bool is_vector_v = false;

template<typename T, typename... Args>
constexpr bool is_vector_v<std::vector<T, Args...>> = true;

// Return the type T but with const if and only if `is_it_const`
// is true.
template<typename T, bool is_it_const>
struct const_if {
  using type = std::remove_const_t<T>;
};

template<typename T>
struct const_if<T, true> {
  using type = std::remove_const_t<T> const;
};

template<typename T, bool is_it_const>
using const_if_t = typename const_if<T, is_it_const>::type;

static_assert(
    std::is_same_v<const_if_t<int, true>, int const> );
static_assert( //
    std::is_same_v<const_if_t<int, false>, int> );
static_assert(
    std::is_same_v<const_if_t<int const, true>, int const> );
static_assert(
    std::is_same_v<const_if_t<int const, false>, int> );

} // namespace mp
