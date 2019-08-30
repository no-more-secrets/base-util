/****************************************************************
** Utilities for getting keys/values from map-like objects.
*****************************************************************/
#pragma once

#include <optional>

namespace bu {

// Does the set contain the given key.
template<typename ContainerT, typename KeyT>
bool has_key( ContainerT const& s, KeyT const& k ) {
  return s.find( k ) != s.end();
}

// The idea of this function is that it will test the given key's
// membership (as a key) in the map and, if it is found, it will
// return a reference (in an optional) to that key inside the
// container. Again, it guarantees to not only return a reference
// to the key, but it will be a reference to the one in the con-
// tainer, which will then only live as long as the map does.
//
// The reason for the return semantics is that 1) if the caller
// eventually wants to copy the returned key, they can do that
// regardless of which reference we return, but 2) if the caller
// wants to hang on to a reference to the key (when found) then
// it is more likely they will want a reference to the one in the
// map, for reasons of managing lifetime of the object (key) re-
// ferred to.
template<typename ContainerT, typename KeyT>
std::optional<std::reference_wrapper<KeyT const>> key_safe(
    ContainerT const& m, KeyT const& k ) {
  auto found = m.find( k );
  if( found == m.end() ) return std::nullopt;
  // Must return the one in the container, not the one passed in
  // as an argument, that's the idea here.
  return found->first;
}

// Get a reference to a value in a map. Since the key may not ex-
// ist, we return an optional. But since we want a reference to
// the object, we return an optional of a reference wrapper,
// since containers can't hold references. I think the reference
// wrapper returned here should only allow const references to be
// extracted.
template<typename KeyT, typename ValT,
         // typename... to allow for maps that may have addi-
         // tional template parameters (but which we don't care
         // about here).
         template<typename KeyT_, typename ValT_, typename...>
         typename MapT>
std::optional<std::reference_wrapper<ValT const>> val_safe(
    MapT<KeyT, ValT> const& m, KeyT const& k ) {
  auto found = m.find( k );
  if( found == m.end() ) return std::nullopt;
  return found->second;
}

} // namespace bu
