/****************************************************************
* Algorithms
****************************************************************/
#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

namespace util {

// This is a version  of  lower_bound  adapted from an implementa-
// tion of the standard library  version  found  on  cppreference
// which does not require passing a value as a parameter; the  bi-
// nary search is conducted  only  from  the  input  of the unary
// function. The unary function should have the signature:
//
//   bool( T const& )
//
// The lower_bound function will return an iterator to  the  posi-
// tion of the first element in  the  list for which the function
// returns false, or end() if there is none. It is  assumed  that
// the list is partitioned according  to  the return value of the
// unary function and furthermore that  the  values for which the
// function  returns  true  come  first. If this condition is not
// met, then it is not certain what will be returned.
//
// As an example, if we  are  performing  a binary search to find
// the  number  5  in  a list, then our unary function (with para-
// meter x) would return (x<5). This way,  the  lower_bound  func-
// tion will return an iterator to the first position whose value
// is not less than 5 (which will be equal to 5 if  that  element
// exists in the list, otherwise to the  smallest  value  greater
// han 5 in the list, or end() if none exist).
template<class Container, class UnaryFunc>
auto lower_bound( Container const& c, UnaryFunc func )
    -> typename Container::const_iterator {

    using const_it_t = typename Container::const_iterator;
    const_it_t first = std::cbegin( c );
    const_it_t last  = std::cend  ( c );

    using diff_t = typename
        std::iterator_traits<const_it_t>::difference_type;
    diff_t count = std::distance( first, last );
    diff_t step;

    const_it_t it;

    while( count > 0 ) {
        it = first;
        step = count / 2;
        std::advance( it, step );
        if( func( *it ) ) {
            first = ++it;
            count -= step + 1;
        }
        else
            count = step;
    }
    return first;
}

// Applies a function to each element of a vector, yielding a new
// vector with the results. Function will be applied serially and
// in order of the elements. Vector returned  will  be  pre  allo-
// cated  and  should  be  NRVO'd so this function should be effi-
// cient.
template<typename Func, typename In>
auto map( Func const& f, std::vector<In> const& in )
       -> std::vector<decltype( f( in[0] ) )> {

    using RetType = decltype( f( in[0] ) );

    std::vector<RetType> res; res.reserve( in.size() );

    for( auto const& e : in )
        res.emplace_back( f( e ) );

    return res;
}

// Applies  function to each element in vector, disgarding return
// value.
template<typename Func, typename In>
void map_( Func const& f, std::vector<In> const& in ) {

    for( auto const& e : in ) f( e );
}

// This  will  do  the remove/erase idiom automatically for conve-
// nience.
template<typename Container, typename Func>
void remove_if( Container& c, Func f ) {
    auto new_end = std::remove_if(
            std::begin( c ), std::end  ( c ), f );
    c.erase( new_end, end( c ) );
}

// Will do an in-place sort.
template<typename T>
void sort( std::vector<T>& v ) {
    std::sort( begin( v ), end( v ) );
}

// Will do an in-place sort and unique.
template<typename T>
void uniq_sort( std::vector<T>& v ) {
    std::sort( begin( v ), end( v ) );
    auto i = std::unique( begin( v ), end( v ) );
    v.erase( i, end( v ) );
}

// Will sort the vector as if the elements of the vector were
// replaced with the results of calling key_func on each.
template<typename T, typename Func>
void sort_by_key( std::vector<T>& v, Func key_func ) {
  auto cmp = [&key_func]( T const& l, T const& r ) {
    return key_func( l ) < key_func( r );
  };
  std::sort( v.begin(), v.end(), cmp );
}

// Same as above, but stable.
template<typename T, typename Func>
void stable_sort_by_key( std::vector<T>& v, Func key_func ) {
  auto cmp = [&key_func]( T const& l, T const& r ) {
    return key_func( l ) < key_func( r );
  };
  std::stable_sort( v.begin(), v.end(), cmp );
}

// When Ranges come there may be functions that do this better
// (such as various `group` or `group_by` functions).
//
// This function will scan the vector v and compute a `key` for
// each element using the provided function. Then it will return
// a vector of indexes at the start of each new segment, where
// segment is defined as a contiguous sequence of elements having
// the same key.  It will _not_ include an index for the first
// element (this is implied).
//
// Note that no sorting is performed or required as a
// precondition of this function, though in many cases the caller
// will want to pre-sort the vector for useful results.
template<typename T, typename Func>
std::vector<size_t> group_by_key( std::vector<T> const& v,
                                  Func key_func ) {
    std::vector<size_t> res;
    if( v.empty() )
        return res;
    auto current_key = key_func( v[0] );
    for( size_t idx = 1; idx < v.size(); ++idx ) {
        auto key = key_func( v[idx] );
        if( key != current_key ) {
            current_key = key;
            res.push_back( idx );
        }
    }
    return res;
}

// Given a list of indexes this function will split the vector on
// those indexes. Each index will be the start of a new segment.
//
// E.g.:
//
//   split_on_idxs( {1,2,3,4,5,6}, {1,4} ) == {{1},{2,3,4},{5,6}}
//
//   split_on_idxs( {1,2,3},       {0,2} ) == {{},{1,2},{3}}
//
// NOTE: it is expected that the idxs vector will be sorted and
//       not contain any duplicate elements.
template<typename T>
std::vector<std::vector<T>>
split_on_idxs( std::vector<T>      const& v,
               std::vector<size_t> const& idxs ) {

    std::vector<std::vector<T>> res;
    if( v.empty() )
        return res;
    size_t current = 0;
    for( auto idx : idxs ) {
        res.emplace_back();
        auto& back = res.back();
        for( ; current < idx; ++current )
            back.push_back( v[current] );
    }
    if( res.size() == idxs.size() )
        res.emplace_back();
    while( current < v.size() ) {
        res.back().push_back( v[current] );
        ++current;
    }
    return res;
}

} // namespace util
