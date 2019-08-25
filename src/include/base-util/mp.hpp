/****************************************************************
** Template Metaprogramming Utilities
*****************************************************************/
#pragma once

namespace mp {

template<bool...>
constexpr const bool all_of_v = false;

template<bool Bool>
constexpr const bool all_of_v<Bool> = Bool;

template<bool First, bool... Bools>
constexpr bool all_of_v<First, Bools...> =
    First&&    all_of_v<Bools...>;

static_assert( all_of_v<true> == true );
static_assert( all_of_v<false> == false );
static_assert( all_of_v<true, false> == false );
static_assert( all_of_v<false, true> == false );
static_assert( all_of_v<true, true> == true );
static_assert( all_of_v<false, false> == false );
static_assert( all_of_v<true, true, true> == true );
static_assert( all_of_v<true, true, false> == false );

} // namespace mp
