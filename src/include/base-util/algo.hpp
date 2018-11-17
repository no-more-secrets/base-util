/****************************************************************
* Algorithms
****************************************************************/
#pragma once

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

} // namespace util
