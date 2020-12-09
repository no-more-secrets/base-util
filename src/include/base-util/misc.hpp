/****************************************************************
* Utilities
****************************************************************/
#pragma once

#include "base-util/macros.hpp"
#include "base-util/types.hpp"

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace util {

// Stream here to send to nowhere.  This is kind of like /dev/null
// except that it does not actually use the OS's /dev/null, it's
// just an application-level black hole.
struct cnull_t {};

template<typename T>
inline cnull_t& operator<<( cnull_t& cnull, T const& /*unused*/ ) {
  return cnull;
}
// E.g.:  cnull << "sending number " << 1 << " to nowhere.";
extern cnull_t cnull;

// Given a range size and chunk  size  this  will  partition  the
// range into equal sized chunks of size chunk_size with the last
// chunk possibly being smaller than  chunk_size. The result will
// be a list of pairs, each pair  describing a chunk as a pair of
// offsets  each  representing the distance from the start of the
// range to the (beginning, end) of the chunk, and where the  end
// is a one-passed-the-end marker.
//
//   E.g.: chunks( 7, 2 ) == [(0,2),(2,4),(4,6),(6,7)]
//
// The resulting offsets can be added to iterators to index  into
// containers.
PairVec<size_t, size_t> chunks( size_t size,
                                size_t chunk_size );

// Loop through the elements in a vector and output them.
template<typename T>
void print_vec( std::vector<T> const& v,
                std::ostream&         out,
                bool                  indent = false,
                std::string_view      name = std::string_view() ) {
    if( !name.empty() )
        out << name << "\n";
    std::string padding = indent ? "    " : "";
    for( auto const& e : v )
        out << padding << e << "\n";
}

}

// Here  we  open up the std namespace to add a hash function spe-
// cialization for a reference_wrapper. This  is  straightforward
// because we can just delegate to the specialization of the type
// inside the wrapper, if there is one. If there is not one, then
// this  specialization  will not be used (which is what we want).
namespace std {

    // Specializing hash<>, but then adding template for contents
    // of reference_wrapper
    template<typename T>
    struct hash<reference_wrapper<T>> {
        auto operator()(
                reference_wrapper<T> const& p ) const noexcept {
            return hash<remove_const_t<T>>{}( p.get() );
        }
    };

} // namespace std

namespace gsl {

// owner<T*> is an alias for T*.  It just marks a pointer as owning,
// used by tools such as clang-tidy.
template <class T, class = std::enable_if_t<std::is_pointer<T>::value>>
using owner = T;

} // namespace gsl
