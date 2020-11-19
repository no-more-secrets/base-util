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

} // namespace util
