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

// This is intended to lessen typing for the simplest of lambda
// functions, namely, those which have no captures, take one or
// two const ref parameters, and consist of either a single re-
// turn statement or just a single expression.
#define L( a )  [] ( auto const& _ ) { return a; }
#define L_( a ) [] ( auto const& _ ) { a; }

#define L0( a )  [] { return a; }
#define L0_( a ) [] { a; }

#define L2( a )  [] ( auto const& _1, auto const& _2 ) { return a; }
#define L2_( a ) [] ( auto const& _1, auto const& _2 ) { a; }

// One  for  lambdas  that  capture  all (usually for simplicity).
#define LC( a )  [&]( auto const& _ ) { return a; }
#define LC_( a ) [&]( auto const& _ ) { a; }

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

// Does the set contain the given key.
template<typename ContainerT, typename KeyT>
bool has_key( ContainerT const& s, KeyT const& k ) {
    return s.find( k ) != s.end();
}

// The idea of this function is that it will test the given key's
// membership (as a key) in the map and, if it is found, it  will
// return  a  reference  (in  an optional) to that key inside the
// container. Again, it guarantees to not only return a reference
// to  the  key, but it will be a reference to the one in the con-
// tainer, which will then only live as  long  as  the  map  does.
//
// The reason for the return semantics is that 1) if  the  caller
// eventually  wants  to  copy the returned key, they can do that
// regardless  of which reference we return, but 2) if the caller
// wants  to  hang on to a reference to the key (when found) then
// it is more likely they will want a reference to the one in the
// map,  for  reasons of managing lifetime of the object (key) re-
// ferred to.
template<typename ContainerT, typename KeyT>
OptCRef<KeyT> get_key_safe( ContainerT const& m,
                            KeyT       const& k ) {
    auto found = m.find( k );
    if( found == m.end() )
        return std::nullopt;
    // Must return the one in the container, not the  one  passed
    // in as an argument, that's the idea here.
    return found->first;
}

// Get  a reference to a value in a map. Since the key may not ex-
// ist, we return an optional. But  since  we want a reference to
// the  object,  we  return  an  optional  of a reference wrapper,
// since  containers can't hold references. I think the reference
// wrapper returned here should only allow const references to be
// extracted.
template<
    typename KeyT,
    typename ValT,
    // typename...  to  allow  for  maps that may have additional
    // template parameters (but which  we  don't care about here).
    template<typename KeyT_, typename ValT_, typename...>
    typename MapT
>
OptRef<ValT const> get_val_safe( MapT<KeyT,ValT> const& m,
                                 KeyT            const& k ) {
    auto found = m.find( k );
    if( found == m.end() )
        return std::nullopt;
    return found->second;
}

// Get value for key; if key does not exist it will throw. If  it
// does exist it will return  a  reference  to  the object in the
// container. ***NOTE that, for error reporting, we are  assuming
// that the key type can be output via ostream, which seems  like
// a reasonable assumption for most things that are normally used
// as keys.
template<
    typename KeyT,
    typename ValT,
    // typename...  to  allow  for  maps that may have additional
    // template parameters (but which  we  don't care about here).
    template<typename KeyT_, typename ValT_, typename...>
    typename MapT
>
ValT const& get_val( MapT<KeyT,ValT> const& m,
                     KeyT            const& k ) {
    auto found = m.find( k );
    ASSERT( found != m.end(), k << " not found in map" );
    return found->second;
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
