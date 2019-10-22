/****************************************************************
** Utilities for getting keys/values from map-like objects.
*****************************************************************/
#pragma once

#include <optional>
#include <type_traits>

namespace bu {

// Does the set contain the given key. If not, returns nullopt.
// If so, returns the iterator to the location.
template<typename ContainerT, typename KeyT>
[[nodiscard]] auto has_key( ContainerT&& s, KeyT const& k ) {
  std::optional<decltype(
      std::forward<ContainerT>( s ).find( k ) )>
      res;
  if( auto it = std::forward<ContainerT>( s ).find( k );
      it != std::forward<ContainerT>( s ).end() )
    res = it;
  return res;
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
auto key_safe( ContainerT const& m, KeyT const& k ) {
  std::optional<std::reference_wrapper<KeyT const>> res;
  if( auto found = m.find( k ); found != m.end() )
    res = found->first;
  // Must return the one in the container, not the one passed in
  // as an argument, that's the idea here.
  return res;
}

// Get a reference to a value in a map. Since the key may not ex-
// ist, we return an optional. But since we want a reference to
// the object, we return an optional of a reference wrapper,
// since containers can't hold references.
template<typename MapT, typename KeyT>
[[nodiscard]] auto val_safe( MapT&& m, KeyT const& k ) {
  std::optional<
      std::reference_wrapper<const std::remove_reference_t<
          decltype( std::forward<MapT>( m ).at( k ) )>>>
      res;
  if( auto found = std::forward<MapT>( m ).find( k );
      found != std::forward<MapT>( m ).end() )
    res.emplace( found->second );
  return res;
}

} // namespace bu
