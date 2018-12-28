/****************************************************************
* std::variant utilities
****************************************************************/
#pragma once

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

// Macro for visiting/dispatching on variants using a switch-like
// syntax. Must only be used on variants without repeating types.
// Use like so:
//
//   auto v = std::variant<int, pair<int,int>, string>{ ... };
//
//   switch_v( v ) {
//     case_v( int ) {
//       cout << "int value: " << val << "\n";
//     }
//     case_v_( pair<int, int>, x, y ) {
//       cout << "x, y = " << x << ", " << y << "\n";
//     }
//     case_v( string ) {
//       cout << "string value: " << val << "\n";
//     }
//     default_v;
//   }
//
// The curly braces are required as is the default_v.
//
// The default case is required and will throw a compile error if
// the type list is not exhaustive. However, it will not trigger
// an error when a type is extraneous.
//
// The case_v_() takes additional parameters which will be bound
// to the value using structured bindings (pattern matching).
//
// The bodies of the case_v's have access to all variables in the
// surrounding scope (they capture them by reference).
//
// Note that, unlike a standard `switch` statement, the concept
// of fallthrough does not work here (indeed it wouldn't make
// sense since the `val` variable, which is available in the body
// of the case_v, can only have one type).
//
// The structure of curly braces is a bit strange in these
// macros, but that is to allow the user to write curly braces as
// in the example above.
#define switch_v( v )                                    \
  { auto const& __v = v;                                 \
    auto __f = [&]( auto&& val ) { if constexpr( false )

#define case_v( t )                                            \
  } else if constexpr(                                         \
          std::is_same_v<std::decay_t<decltype( val )>, t> ) {

#define case_v_( t, ... )                                      \
  } else if constexpr(                                         \
          std::is_same_v<std::decay_t<decltype( val )>, t> ) { \
      auto const& [__VA_ARGS__] = val;

#define default_v                                                \
  } else static_assert(                                          \
          ::util::detail::parametrized_false_v<decltype( val )>, \
          "non-exhaustive variant visitor type list" );          \
  }; { std::visit( __f, __v ); } (void)__v

namespace detail {

// We must use this in the above macro instead of just "false"
// because otherwise the static_assert will always trigger.
template<typename T>
inline bool constexpr parametrized_false_v = false;

}

} // namespace util
