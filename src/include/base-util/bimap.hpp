/****************************************************************
* Bi-directional Map Classes
****************************************************************/
#pragma once

#include "base-util/algo.hpp"
#include "base-util/misc.hpp"

#include <algorithm>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

namespace util {

/****************************************************************
* BiMapFixed ("Immutable Bi-directional Map")
*
* This class will map a collection  of  unique keys (of the speci-
* fied  type)  to a unique collection of values (of the specified
* type) in a 1-to-1 mapping.  Mapping  from  key to value or from
* value to key happens in O(ln(N)) time.
*
* The  key  characteristic  of this class is that it is immutable
* after construction. This allows for  significant  optimizations
* and simplifications in the implementation,  and  so  should  be
* used when a mutability is  not  needed.  Also, neither the keys
* nor  the  values  ever  need to be copied; only one instance of
* each key/value is held in the container.
*
* When constructing the class no validation  is done of the input.
* It  it  assumed  that the input contains a list of pairs (or tu-
* ples) such that all the keys are unique (first element) and all
* the values are unique (second  element), although the data does
* not need to be sorted in any way.
****************************************************************/
template<typename KeyT, typename ValT>
class BiMapFixed {

public:
    BiMapFixed( BiMapFixed const& )            = delete;
    BiMapFixed& operator=( BiMapFixed const& ) = delete;
    BiMapFixed( BiMapFixed&& )                 = default;
    BiMapFixed& operator=( BiMapFixed&& )      = default;

    using value_type = std::tuple<KeyT, ValT>;

    using const_iterator =
            typename std::vector<value_type>::const_iterator;

    // If  sorted is false then the data will be sorted according
    // to the first element in the pair.
    explicit BiMapFixed( std::vector<value_type>&& data,
                         bool sorted = false );

    // Data will be sorted according to the first element in pair.
    BiMapFixed( std::initializer_list<value_type> data );

    // Returns #keys (== #values)
    size_t size() const { return m_data.size(); }

    // Returns an optional  of  reference,  so  no copying/moving
    // should happen here.
    bu::OptRef<ValT const> val_safe( KeyT const& key ) const;
    bu::OptRef<KeyT const> key_safe( ValT const& val ) const;

    // These variants will throw exceptions when key/val  is  not
    // found.
    ValT const& val( KeyT const& key ) const;
    KeyT const& key( ValT const& val ) const;

    // Will yield tuples as values like STL map containers.
    const_iterator begin() const { return m_data.begin(); }
    const_iterator end()   const { return m_data.end();   }

private:

    using ref_type = std::reference_wrapper<value_type const>;

    // Helper to facilitate sharing code between constructors.
    void initialize( bool sorted );

    // These are references to the data  in m_data; they exist so
    // that we can have the data sorted  both by key and by value
    // for quick lookup in either direction.
    std::vector<ref_type>   m_by_key;
    std::vector<ref_type>   m_by_val;

    // Only one copy of each key/value is held here.
    std::vector<value_type> m_data;
};

template<typename KeyT, typename ValT>
typename BiMapFixed<KeyT, ValT>::const_iterator begin(
        BiMapFixed<KeyT, ValT> const& bmf )
    { return bmf.begin(); }

template<typename KeyT, typename ValT>
typename BiMapFixed<KeyT, ValT>::const_iterator end(
        BiMapFixed<KeyT, ValT> const& bmf )
    { return bmf.end(); }

// Helper function to  facilitate  sharing  code between construc-
// tors. This function assumes as a precondition that the
// m_by_key and m_by_val are empty and that m_data has been  popu-
// lated with values.
template<typename KeyT, typename ValT>
void BiMapFixed<KeyT, ValT>::initialize( bool sorted ) {

    m_by_key.reserve( m_data.size() );
    m_by_val.reserve( m_data.size() );

    // First we sort m_data by key. This  way,  when  we  iterate
    // through m_data it will  appear  in  order  sorted  by  key,
    // which is nice.
    auto lt_fst = []( value_type const& r1, value_type const& r2 )
        { return std::get<0>( r1 ) < std::get<0>( r2 ); };

    // Don't  sort  data  if the user claims it is already sorted.
    if( !sorted )
        std::sort( std::begin( m_data ), std::end( m_data ),
                   lt_fst );

    // Now we just populate the keys list from m_data and it will
    // already be sorted in the right  way since we sorted m_data
    // by key.
    for( auto const& e : m_data )
        m_by_key.emplace_back( e );

    // The value list should have all the same items as  the  key
    // list, just sorted in a  different  order.  So  first  copy,
    // then sort by value.
    m_by_val = m_by_key;

    auto lt_snd = []( ref_type const& r1, ref_type const& r2 )
        { return std::get<1>( r1.get() )  <
                 std::get<1>( r2.get() ); };

    std::sort( std::begin( m_by_val ), std::end( m_by_val ),
               lt_snd );
}

template<typename KeyT, typename ValT>
BiMapFixed<KeyT, ValT>::BiMapFixed(
    std::vector<value_type>&& data,
    bool sorted ) : m_by_key(), m_by_val(), m_data( std::move( data ) )
{
    initialize( sorted );
}

// Data will be sorted according to the first element in pair.
template<typename KeyT, typename ValT>
BiMapFixed<KeyT, ValT>::BiMapFixed(
        std::initializer_list<value_type> data )
      : m_by_key(), m_by_val(), m_data() {

    m_data.reserve( data.size() );
    for( auto const& e : data )
        m_data.emplace_back( e );

    // Finish initialization; `false` means that we do not assume
    // the contents of the initializer list are sorted.
    initialize( false );
}

// Returns  an optional of reference, so no copying/moving should
// happen here.
template<typename KeyT, typename ValT>
bu::OptRef<ValT const>
BiMapFixed<KeyT, ValT>::val_safe( KeyT const& key ) const {

    auto i = util::lower_bound(
                m_by_key,
                [&]( auto const& _) {
                  return std::get<0>( _.get() ) < key;
                } );

    // Make sure that the key was found.
    if( i != std::end( m_by_key ) ) {
        // Get a reference to the (key, val) pair found.
        auto const& p = (*i).get();
        // Make sure it is the  one  we  are looking for (the low-
        // er_bound) function won't guarantee this.
        if( std::get<0>( p ) == key )
            return std::get<1>( p );
    }

    return std::nullopt;
}

template<typename KeyT, typename ValT>
bu::OptRef<KeyT const>
BiMapFixed<KeyT, ValT>::key_safe( ValT const& val ) const {

    auto i = util::lower_bound(
                m_by_val,
                [&]( auto const& _ ) {
                  return std::get<1>( _.get() ) < val;
                } );

    // Make sure that the value was found.
    if( i != std::end( m_by_val ) ) {
        // Get a reference to the (key, val) pair found.
        auto const& p = (*i).get();
        // Make sure it is the  one  we  are looking for (the low-
        // er_bound) function won't guarantee this.
        if( std::get<1>( p ) == val )
            return std::get<0>( p );
    }

    return std::nullopt;
}

// These variants will  throw  exceptions  when  key/val  is  not
// found.
template<typename KeyT, typename ValT>
ValT const& BiMapFixed<KeyT, ValT>::val( KeyT const& key ) const {
    auto const& v = val_safe( key );
    ASSERT( v, "key not found in BiMapFixed" );
    return *v;
}

template<typename KeyT, typename ValT>
KeyT const& BiMapFixed<KeyT, ValT>::key( ValT const& val ) const {
    auto const& k = key_safe( val );
    ASSERT( k, "value not found in BiMapFixed" );
    return *k;
}

/****************************************************************
* BDIndexMap ("Bi-directional map with increasing ints as keys")
*
* This class will map a collection of unique items of  the  speci-
* fied  type  to  a unique list of integers. Mapping from key (al-
* ways integer) to value  (specified  type)  will  happen in O(1)
* time. Mapping from value to key will be O(ln(N)) time.
*
* The key characteristics of this class are that:
*
*   a) The class is immutable after construction
*   b) The keys must always be integers
*   c) The keys are always the integers 0..(N-1), where N
*      is the number of pairs in the map.  In other words,
*      one cannot specify an arbitrary list of integers
*      for the keys.
*
* Values are returned as optional references.
****************************************************************/
template<typename T> class BDIndexMap {

public:
    BDIndexMap( BDIndexMap const& )            = delete;
    BDIndexMap& operator=( BDIndexMap const& ) = delete;
    BDIndexMap( BDIndexMap&& )                 = default;
    BDIndexMap& operator=( BDIndexMap&& )      = default;

    // NOTE: the contained data must be a vector of unique items;
    // if what you are passing in does not meet that  requirement
    // then  set  the is_uniq_sorted flag to false and it will be
    // done for you. If you don't do this then this class may not
    // function properly.
    explicit BDIndexMap( std::vector<T>&& data,
                         bool             is_uniq_sorted = false );

    // Returns #keys (== #values)
    size_t size() const { return m_data.size(); }

    // Returns an optional  of  reference,  so  no copying/moving
    // should happen here.
    bu::OptRef<T const>   val_safe( size_t   n   ) const;
    std::optional<size_t> key_safe( T const& val ) const;

    // These variants will throw exceptions when key/val  is  not
    // found.
    T const& val( size_t   n   ) const;
    size_t   key( T const& val ) const;

private:

    std::vector<T> m_data;
};

template<typename T>
BDIndexMap<T>::BDIndexMap( std::vector<T>&& data,
                           bool is_uniq_sorted )
    : m_data( std::move( data ) ) {

    if( !is_uniq_sorted )
        util::uniq_sort( m_data );
}

template<typename T>
std::optional<size_t>
BDIndexMap<T>::key_safe( T const& val ) const {

    auto i = std::lower_bound(
                std::begin( m_data ), std::end( m_data ), val );

    if( i != std::end( m_data ) && *i == val )
        return i - std::begin( m_data );

    return std::nullopt;
}

template<typename T>
size_t BDIndexMap<T>::key( T const& val ) const {

    auto k = key_safe( val );
    ASSERT( k, "value not found in bimap" );
    return *k;
}

template<typename T>
bu::OptRef<T const> BDIndexMap<T>::val_safe( size_t n ) const {

    if( n >= m_data.size() )
        return std::nullopt;

    return m_data[n];
}

template<typename T>
T const& BDIndexMap<T>::val( size_t n ) const {

    ASSERT( n < m_data.size(),
           "index " << n << " not found in bimap" );
    return m_data[n];
}

} // namespace util
