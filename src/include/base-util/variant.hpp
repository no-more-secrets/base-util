/****************************************************************
* std::variant utilities
****************************************************************/
#pragma once

#include "base-util/mp.hpp"
#include "base-util/pp.hpp"

#include <type_traits>
#include <variant>

namespace util {

// Does the variant hold the specified value. It will first be
// tested to see if it holds the type of the value, which is ob-
// viously a prerequisite for holding the value.
template<typename T, typename... Vs>
bool holds( std::variant<Vs...> const& v, T const& val ) {
  return std::holds_alternative<T>( v ) &&
         std::get<T>( v ) == val;
}

// This is just to have consistent notation with the above in
// the case when we just want to test if it holds a type and
// get a pointer to the value.  That said, since this would
// usually be used in the context of an if-statement, you are
// advised to used the below macro instead.
template<typename T, typename... Vs>
auto const* holds( std::variant<Vs...> const& v ) {
  return std::get_if<T>( &v );
}

template<typename T, typename... Vs>
auto* holds( std::variant<Vs...>& v ) {
  return std::get_if<T>( &v );
}

// This is a helper for testing/extracting values from variants.
// Commonly, we want to say: "if the variant holds a value of
// type X then get a pointer to its value and then do something
// with it. This macro allows us to do that:
//
//   variant<int, string> v = ...;
//
//   GET_IF( v, string, number ) {
//     /* v holds type string and number
//      * is a pointer to it. */
//     cout << number->size();
//   }
//
// Note that this macro must be placed inside an if statement and
// that it requires C++17 because it declares the variable inside
// the if statement.
#define GET_IF( subject, type, var ) \
  if( auto* var = std::get_if<type>( &subject ); var )

#define if_v( subject, type, var ) GET_IF( subject, type, var )

// For convenience, just swaps the ordering of the parameters.
template<typename Variant, typename VisitorFunc>
auto visit( Variant& v, VisitorFunc const& func ) {
  return std::visit( func, v );
}

/****************************************************************
* Variant Pattern Matchers
****************************************************************/
// Macro for visiting/dispatching on variants using a switch-like
// syntax. Must only be used on variants without repeating types.
//
// BASIC USAGE
// ===========
//
//   struct Point { int x; int y; };
//   auto v = std::variant<int, Point, string>{ ... };
//
//   switch_( v ) {
//     case_( int ) {
//       cout << "int value: " << val << "\n";
//     }
//     case_( Point ) {
//       cout << "x, y = " << val.x << ", " << val.y << "\n";
//     }
//     case_( string ) {
//       cout << "string value: " << val << "\n";
//     }
//     switch_exhaustive;
//   }
//
// The curly braces are required as is the switch_exhaustive.
//
// The default case is required and will throw a compile error if
// the type list is not exhaustive. FIXME: however, it will not
// trigger an error when a type is extraneous.
//
// The bodies of the case_'s have access to all variables in the
// surrounding scope (they capture them by reference).
//
// Important: Note that, unlike a standard `switch` statement,
// the concept of fallthrough does not work here (indeed it
// wouldn't make sense since the `val` variable, which is avail-
// able in the body of the case_, can only have one type). An-
// other important difference is that a `return` statement issued
// from within one of the case blocks will NOT return from the
// surrounding function, it will just act as a `break` in a
// normal switch.
//
// NON-EXHAUSTIVE CASES
// ====================
//
// If only a subset of the variant's types need to be handled
// then one can use `default_switch` to avoid requiring that all
// cases be handled:
//
//   struct Point { int x; int y; };
//   auto v = std::variant<int, Point, string>{ ... };
//
//   switch_( v ) {
//     case_( string ) {
//       cout << "string value: " << val << "\n";
//     }
//     default_switch( {
//       cout << "default action.\n";
//     } );
//   }
//
// Likewise, for matcher_, one can use `default_matcher` and re-
// turn a default value.
//
// PATTERN MATCHING
// ================
//
// The case_ can optionally take additional parameters which must
// correspond to the struct field names and will be available
// within the case_ as references to the respective members. The
// members can be specified in any order, but all of them must be
// specified.
//
//   struct Point { int x; int y; };
//   auto v = std::variant<int, Point, string>{ ... };
//
//   switch_( v ) {
//     ...
//     // y,x are "out of order"; this is ok.
//     case_( Point, y, x ) {
//       cout << "x, y = " << x << ", " << y << "\n";
//     }
//     ...
//   }
//
// RETURNING VALUES
// ================
//
// The `switch_` macro prevents the user from attempting to re-
// turn (or return values); it is intended only for side effects.
// To yield a value from a variant switch_ you should instead use
// the `matcher_` and `resu1t` macros:
//
//   auto n = matcher_( my_var ) {
//     case_( MyVar::state_1 ) {
//       resu1t 5;
//     }
//     case_( MyVar::state_2, var_1 ) {
//       if( var_1 ) break_ *var_1;
//       resu1t 6;
//     }
//     case_( MyVar::state_3, var_2, var_3 ) {
//       resu1t (int)var_3;
//     }
//     matcher_exhaustive;
//   }
//
// We use `resu1t` instead of `return` to return values; the
// former is simply an alias for the latter, but should be used
// because it helps to drive home to the reader that the return
// statement is not returning from the outter scope (as would
// happen by returning in a real switch block) but instead is
// just yield a value from the `matcher_` statement. This is also
// why the `switch_` macro prevents returning at all.
//
// When using return values, `switch_exhaustive` must be used
// since all possibilities must be handled in order to produce a
// return value in all cases. TODO: maybe have it automatically
// return an `optional` of the return value if switch_exhaustive
// is used.
//
// RETURN TYPE HINTS
// =================
//
// When returning types such as std::optional<int> it can be de-
// sirable to have the individual case_ functions return e.g.
// ints or nullopts, i.e., things that can be converted to
// std::optional<int>. However, if we do this, we get compiler
// errors since each of the case_ functions have different return
// types. To fix this we can specify what the return type should
// be by passing it as the third argument to the matcher_ macro:
//
//   auto n = matcher_( my_var, ->, std::optional<int> ) {
//     case_( MyVar::state_1 ) {
//       break_ 5;
//     }
//     case_( MyVar::state_2, var_1 ) {
//       if( var_1 ) break_ *var_1;
//       break_ std::nullopt;
//     }
//     case_( MyVar::state_3, var_2, var_3 ) {
//       break_ (int)var_3;
//     }
//     matcher_exhaustive;
//   }
//
// which would not compile without specifying the return type.
//
// The second macro argument is just a dummy and can be filled
// with the -> operator to make the appearance reminiscent of
// C++'s syntax for specifying trailing return types.
//
// variant_function
// ================
//
// A variant on the above (no pun intended) that works better for
// defining free-standing functions that pattern match on variant
// values is the `variant_function` macro. Use it like so:
//
//   struct A {};
//   variant<int, string, A> v = A{};
//
//   auto to_string = variant_function( item, ->, std::string ) {
//     case_( int ) return std::to_string( item );
//     case_( string ) return item;
//     case_( A ) return "A";
//     variant_function_exhaustive;
//   };
//
// where the second/third arguments to the variant_function,
// specifying return type, are optional.
//
// Note that this example demonstrates that the case_ statements
// (and indeed case_ statements also) do not need parenthases.
// Also, we use naked `return` keywords because the effect of
// those statements is equivalent to returning from the outter
// context, unlike from within a `switch_` or `matcher_` block.
//
// CLOSING THOUGHTS
// ================
//
// TODO: in C++20 there will be a new overload of std::visit that
// takes the return type as a template argument and will convert
// any return value to that type, which might be useful here, al-
// though currently we don't seem to need this given the
// two-argument switch_ macro.
//
// The structure of curly braces is a bit strange in these
// macros, but that is to allow the user to write curly braces as
// in the example above.
//
/////////////////////////////////////////////////////////////////

/*                      === switch_v ===                       */

// The val0 is a variable that we would use inside the case_
// statement to refer to the value of the current alternative,
// but instead we call it val0 because we will later define a
// `val` for this purpose. The reason is because the `val0` is
// declared with `auto&&` and so the language intelligence has a
// hard time determining the type of it when hovering over a
// variable in an editor. So as mentioned we will define a `val`
// (in the case_ macro) with an explicit type which we will know
// at that point (which does not depend on any auto parameters)
// that will solve this issue.
#define switch_v( v )                                           \
  [&]{ auto& __v = v;                                           \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& val0 ) ->                            \
          ::util::detail::switch_ret_do_not_use_t {             \
        auto& __val = val0;                                     \
        if constexpr( false )

#define switch_exhaustive                                       \
  } else static_assert(                                         \
        ::util::detail::parametrized_false_v<decltype( __val )>,\
        "non-exhaustive variant visitor type list" );           \
  return ::util::detail::VSRDNU;                                \
  }; { return std::visit( __f, __v ); } }(); {

#define default_switch( ... )                                   \
  } else { __VA_ARGS__ }                                        \
  return ::util::detail::VSRDNU;                                \
  }; { return std::visit( __f, __v ); } }(); {

#define switch_non_exhaustive                                   \
  } return ::util::detail::VSRDNU; };                           \
  { return std::visit( __f, __v ); } }(); {

/*                     === matcher_v ===                       */

#define matcher_v_MULTI( v, _, ret_type )                       \
  [&]{ auto& __v = v;                                           \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& val0 ) -> ret_type {                 \
        auto& __val = val0;                                     \
        if constexpr( false )

#define matcher_v_SINGLE( v )                                   \
  [&]{ auto& __v = v;                                           \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
       auto __f = [&]( auto&& val0 ) {                          \
         auto& __val = val0;                                    \
         if constexpr( false )

#define matcher_v( ... )                                        \
        PP_ONE_OR_MORE_ARGS( matcher_v, __VA_ARGS__ )

#define matcher_exhaustive                                      \
  } else static_assert(                                         \
        ::util::detail::parametrized_false_v<decltype( __val )>,\
        "non-exhaustive variant visitor type list" );           \
  }; { return std::visit( __f, __v ); } }(); {

#define default_matcher( ... )                                  \
  } else { __VA_ARGS__ }                                        \
  }; { return std::visit( __f, __v ); } }(); {

/*                 === variant_function ===                    */

#define variant_function_MULTI( val, _, ret_type )              \
  []( auto& v ){ auto& __v = v;                                 \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& __val ) -> ret_type {                \
        auto& val = __val;                                      \
        if constexpr( false )

#define variant_function_SINGLE( val )                          \
  []( auto& v ){ auto& __v = v;                                 \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& val ) {                              \
        auto& val = __val;                                      \
        if constexpr( false )

#define variant_function_c_MULTI( val, _, ret_type )            \
  [&]( auto& v ){ auto& __v = v;                                \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& __val ) -> ret_type {                \
        auto& val = __val;                                      \
        if constexpr( false )

#define variant_function_c_SINGLE( val )                        \
  [&]( auto& v ){ auto& __v = v;                                \
    constexpr bool __is_it_const =                              \
      std::is_const_v<std::remove_reference_t<decltype(__v)>>;  \
    auto __f = [&]( auto&& val ) {                              \
        auto& val = __val;                                      \
        if constexpr( false )

#define variant_function( ... )                                 \
        PP_ONE_OR_MORE_ARGS( variant_function, __VA_ARGS__ )

// Version that captures.
#define variant_function_c( ... )                               \
        PP_ONE_OR_MORE_ARGS( variant_function_c, __VA_ARGS__ )

#define variant_function_exhaustive                             \
  } else static_assert(                                         \
        ::util::detail::parametrized_false_v<decltype( __val )>,\
        "non-exhaustive variant visitor type list" );           \
  }; { return std::visit( __f, __v ); }

#define default_variant_function( def )                         \
  } else { def; }                                               \
  }; { return std::visit( __f, __v ); }

/*                       === case_v ===                        */

// Helpers to be used only in this header.
#define V_UNDERSCORES( a )     PP_JOIN( __, a )
#define V_USE_VARIABLE( a )    (void)a;
#define V_REF_STRUCT_ELEM( a ) auto& a = __val.a;

// The references in the structured binding should inherit const
// from `v`. Here inside the if we generate something like this:
//
//   // This checks number of vars.
//   auto& [__x,__y] = val;
//   (void)__x; (void)__y;
//
//   // These check the variable names and allow using them
//   // independent of order:
//   auto& x = val.x;
//   auto& y = val.y;
//
#define case_v_MULTI( t, ... )                                      \
  } else if constexpr(                                              \
          std::is_same_v<std::decay_t<decltype( __val )>, t> ) {    \
      (void)__is_it_const;                                          \
      auto& [EVAL( PP_MAP_COMMAS( V_UNDERSCORES, __VA_ARGS__ ) )]   \
          = __val;                                                  \
      EVAL( PP_MAP( V_USE_VARIABLE,                                 \
                    PP_MAP_COMMAS( V_UNDERSCORES, __VA_ARGS__ ) ) ) \
      EVAL( PP_MAP( V_REF_STRUCT_ELEM, __VA_ARGS__ ) )

#define case_v_SINGLE( t )                                          \
  } else if constexpr(                                              \
          std::is_same_v<std::decay_t<decltype( __val )>, t> ) {    \
      using val_t = mp::const_if_t<t, __is_it_const>;               \
      val_t& val = __val; (void)val;

#define case_v( ... ) PP_ONE_OR_MORE_ARGS( case_v, __VA_ARGS__ )

/*                     === break/return ===                        */

#define break_v  return ::util::detail::switch_ret_do_not_use_t{}
#define result_v return

/*                    === terse keywords ===                       */

// If these are conflicting with symbols in other headers then
// define this in the relevant translation unit before including
// this header.
#ifndef RN_NO_DEFINE_SHORT_V_KEYWORDS
#  define  switch_( ... )  switch_v( __VA_ARGS__ )
#  define matcher_( ... ) matcher_v( __VA_ARGS__ )
#  define    case_( ... )    case_v( __VA_ARGS__ )
#  define   break_          break_v
#  define   resu1t         result_v
#endif

namespace detail {

struct switch_ret_do_not_use_t {
  explicit constexpr switch_ret_do_not_use_t() {}
};
inline constexpr switch_ret_do_not_use_t VSRDNU{};

// We must use this in the above macro instead of just "false"
// because otherwise the static_assert will always trigger.
template<typename T>
inline bool constexpr parametrized_false_v = false;

}

} // namespace util
