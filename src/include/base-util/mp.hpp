/****************************************************************
** Template Metaprogramming Utilities
*****************************************************************/
#pragma once

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

} // namespace mp
