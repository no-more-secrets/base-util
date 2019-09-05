/****************************************************************
** Type Map
*****************************************************************/
// Map from type to type, possibly with default.

#include <type_traits>

template<typename V>
struct Ident {
  using type = V;
};

template<typename K, typename V>
struct KV {
  using first_type  = K;
  using second_type = V;
};

template<typename... Pairs>
struct TypeMap : Pairs... {};

template<typename K, typename..., typename V>
Ident<V> GetImpl( KV<K, V> );

template<typename K, typename Default>
Default GetImpl( ... );

template<typename M, typename T, typename... Default>
using Get = typename decltype(
    GetImpl<T, Ident<Default>...>( M{} ) )::type;

/****************************************************************
** Examples / Tests
*****************************************************************/

class NoConstruct {
  NoConstruct() = delete;
};

using M = TypeMap<          //
    KV<int, float>,         // typical example
    KV<void, const double>, // works with void, const
    KV<char, NoConstruct>   // and non-default-constructible
    >;

// With no default value; will give a compile error if the key
// type is not found in the map.
static_assert( std::is_same_v<Get<M, void>, const double> );
static_assert( std::is_same_v<Get<M, char>, NoConstruct> );
static_assert( std::is_same_v<Get<M, int>, float> );

// With default value; will never give a compile error.
static_assert( std::is_same_v<Get<M, void, int>, const double> );
static_assert( std::is_same_v<Get<M, char*, int>, int> );
