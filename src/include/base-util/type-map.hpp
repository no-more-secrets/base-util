/****************************************************************
** Type Map
*****************************************************************/
// Map from type to type, possibly with default.

#include <type_traits>

template<typename V>
struct Ident { using type = V; };

template<typename K, typename V>
struct Pair {
  using first_type  = K;
  using second_type = V;
};

template<typename... Pairs>
struct TypeMap : Pairs... {};

template<typename K, typename..., typename V>
Ident<V> GetImpl( Pair<K, V> );

template<typename K, typename Default>
Default  GetImpl( ... );

template<typename M, typename T, typename... Default>
using Get = typename decltype( GetImpl<T, Ident<Default>...>( M{} ) )::type;

class NoConstruct { NoConstruct() = delete; };

/****************************************************************
** Examples / Tests
*****************************************************************/

using M = TypeMap<
  Pair<int,  float>,
  Pair<void, const double>,
  Pair<char, NoConstruct>
>;

static_assert( std::is_same_v<Get<M, void>, const double> );
static_assert( std::is_same_v<Get<M, char>, NoConstruct> );
static_assert( std::is_same_v<Get<M, int>,  float> );

// With default value
static_assert( std::is_same_v<Get<M, void,  int>, const double> );
static_assert( std::is_same_v<Get<M, char*, int>, int> );
