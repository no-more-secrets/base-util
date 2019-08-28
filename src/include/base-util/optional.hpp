/****************************************************************
** std::optional utilities
*****************************************************************/
#pragma once

#include "base-util/macros.hpp"
#include "base-util/misc.hpp"

#include <iostream>
#include <optional>
#include <type_traits>

namespace util {

// In global namespace.
template<typename T>
std::ostream& operator<<( std::ostream&           out,
                          std::optional<T> const& opt ) {
  // In the implementation of  this  function we don't delegate
  // to  the  to_string method of optional that we have because
  // we  don't  want to assume that the type T can be converted
  // to a string necessarily  (all  we  need  is  for  it to be
  // streamable).
  if( opt ) return ( out << *opt );
  return ( out << "nullopt" );
}

// This will take the vectors of optionals and will gather all of
// them that are not nullopt and move their values into a  vector
// and return it.
template<typename T>
std::vector<T> cat_opts( std::vector<std::optional<T>>&& opts ) {
  // We might need up to this size.
  std::vector<T> res;
  res.reserve( opts.size() );
  for( auto& o : opts )
    if( o ) res.emplace_back( std::move( *o ) );

  return res;
}

// This will take the vectors of optionals and will gather all of
// them that are not nullopt and move their values into a  vector
// and return it.
template<typename T>
std::vector<T> cat_opts(
    std::vector<std::optional<T>> const& opts ) {
  // We might need up to this size.
  std::vector<T> res;
  res.reserve( opts.size() );
  for( auto const& o : opts )
    if( o ) res.push_back( *o );

  return res;
}

/****************************************************************
** Monadic Features
*****************************************************************/
// The functions below can be used in the following way:
//
// 1. fmap returning by value.  When the given function returns
//    by value then the resulting optional will hold that value.
//
//    std::optional<int> oi = ...;
//    double foo( std::optional<int> const& o );
//
//    using util::fmap;
//    std::optional<double> res = fmap( foo, oi );
//
//   or
//
//    using util::infix::fmap;
//    std::optional<double> res = oi | fmap( foo );
//
// 2. fmap returning by ref.  When the given function returns
//    an lvalue reference then the resulting optional will hold a
//    reference wrapper.
//
//    std::optional<int> oi = ...;
//    double const& foo( std::optional<int> const& o );
//
//    using util::fmap;
//    std::optional<std::reference_wrapper<double const>> res =
//      fmap( foo, oi );
//
//   or
//
//    using util::infix::fmap;
//    std::optional<std::reference_wrapper<double const>> res =
//      oi | fmap( foo );
//
// 3. fmap_join.  This is for when the given function returns
//    an optional. This would result in nested optionals if only
//    fmap were used, but fmap_join will collapse them.
//
//    std::optional<int> oi = ...;
//    std::optional<double> foo( std::optional<int> const& o );
//
//    using util::fmap_join;
//    std::optional<double> res = util::fmap_join( foo, oi );
//
//   or
//
//    using util::infix;
//    std::optional<double> res = oi | util::fmap_join( foo );
//
//    We do not have an fmap_join for functions that return ref-
//    erences (as we do for fmap above) since for fmap_join it is
//    required that the provided function return an optional,
//    which is never reference (even if it is holding a reference
//    wrapper).

template<typename Pred, typename T>
using OptInvokeResult =
    std::optional<std::invoke_result_t<Pred, T>>;

// For predicates that return by value.
template<typename Pred, typename T>
auto fmap( Pred&& f, std::optional<T> const& o )
    -> std::enable_if_t<                          //
        std::is_invocable_v<Pred, T> &&           //
            !std::is_rvalue_reference_v<          //
                std::invoke_result_t<Pred, T>> && //
            !std::is_lvalue_reference_v<          //
                std::invoke_result_t<Pred, T>>,   //
        OptInvokeResult<Pred, T>                  //
        >                                         //
{
  OptInvokeResult<Pred, T> res;
  if( o.has_value() )
    res.emplace( std::forward<Pred>( f )( *o ) );
  return res;
}

template<typename Pred, typename T>
using OptRefInvokeResult = std::optional<std::reference_wrapper<
    std::remove_reference_t<std::invoke_result_t<Pred, T>>>>;

// For predicates that return an lvalue reference.
template<typename Pred, typename T>
auto fmap( Pred&& f, std::optional<T> const& o )
    -> std::enable_if_t<                        //
        std::is_invocable_v<Pred, T> &&         //
            std::is_lvalue_reference_v<         //
                std::invoke_result_t<Pred, T>>, //
        OptRefInvokeResult<Pred, T>             //
        >                                       //
{
  OptRefInvokeResult<Pred, T> res;
  if( o.has_value() )
    res.emplace( std::forward<Pred>( f )( *o ) );
  return res;
}

template<typename T>
constexpr bool is_optional_v = false;

template<typename T>
constexpr bool is_optional_v<std::optional<T>> = true;

template<typename Pred, typename T>
using OptInvokeResultValue = std::optional<      //
    std::decay_t<                                //
        decltype(                                //
            std::declval<                        //
                std::invoke_result_t<Pred, T>>() //
                .value()                         //
            )                                    //
        >                                        //
    >;

// For predicates that return optionals. This will collapse the
// optionals (a la >>=).
template<typename Pred, typename T>
auto fmap_join( Pred&& f, std::optional<T> const& o ) ->    //
    std::enable_if_t<                                       //
        std::is_invocable_v<Pred, T> &&                     //
            is_optional_v<std::invoke_result_t<Pred, T>> && //
            !std::is_rvalue_reference_v<                    //
                std::invoke_result_t<Pred, T>> &&           //
            !std::is_lvalue_reference_v<                    //
                std::invoke_result_t<Pred, T>>,             //
        OptInvokeResultValue<Pred, T>                       //
        >                                                   //
{
  OptInvokeResultValue<Pred, T> res;
  if( o.has_value() ) {
    auto intermediate = std::forward<Pred>( f )( *o );
    if( intermediate.has_value() )
      res.emplace( std::move( *intermediate ) );
  }
  return res;
}

template<typename T>
bool maybe_truish_to_bool( std::optional<T> const& o ) {
  return o.has_value() ? bool( *o ) : false;
}

template<typename T>
std::optional<std::decay_t<T>> just( T&& value ) {
  return std::optional<std::decay_t<T>>( FWD( value ) );
}

template<typename T>
auto just_ref( T const& value ) {
  return std::optional<
      std::reference_wrapper<std::decay_t<T> const>>(
      FWD( value ) );
}

namespace infix {
namespace detail {

template<typename F>
struct infix_f : F {
  constexpr infix_f( F&& f ) noexcept( noexcept( F{FWD( f )} ) )
    : F{FWD( f )} {}
};

template<typename F>
struct infix : F {
  constexpr infix( F&& f ) noexcept( noexcept( F{FWD( f )} ) )
    : F{FWD( f )} {}
};

template<typename T, typename F>
constexpr auto operator|( std::optional<T> const& o,
                          infix_f<F>&& f ) THREE_TIMES( f( o ) );

template<typename T, typename F>
constexpr auto operator|( std::optional<T> const& o,
                          infix<F> const&         f )
    THREE_TIMES( f( o ) );

} // namespace detail

#define OPT_DEFINE_INFIX_FUNC( name )                   \
  template<typename Func>                               \
  constexpr auto name( Func&& f ) {                     \
    return detail::infix_f{[&]( auto&& o ) THREE_TIMES( \
        ::util::name( FWD( f ), FWD( o ) ) )};          \
  }

#define OPT_DEFINE_INFIX( name )              \
  inline constexpr auto name = detail::infix{ \
      []( auto&& o ) THREE_TIMES( ::util::name( FWD( o ) ) )};

OPT_DEFINE_INFIX( maybe_truish_to_bool )

OPT_DEFINE_INFIX_FUNC( fmap )
OPT_DEFINE_INFIX_FUNC( fmap_join )

} // namespace infix

} // namespace util
