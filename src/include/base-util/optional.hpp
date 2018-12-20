/****************************************************************
* std::optional utilities
****************************************************************/
#pragma once

#include "base-util/misc.hpp"

#include <iostream>
#include <optional>

namespace util {

// In global namespace.
template<typename T>
std::ostream& operator<<( std::ostream&           out,
                          std::optional<T> const& opt ) {
    // In the implementation of  this  function we don't delegate
    // to  the  to_string method of optional that we have because
    // we  don't  want to assume that the type T can be converted
    // to a string necessarily  (all  we  need  is  for  it to be
    // streamable).
    if( opt )
        return (out << *opt);
    return (out << "nullopt");
}

// This will take the vectors of optionals and will gather all of
// them that are not nullopt and move their values into a  vector
// and return it.
template<typename T>
std::vector<T> cat_opts( std::vector<std::optional<T>>&& opts ) {
    // We might need up to this size.
    std::vector<T> res; res.reserve( opts.size() );
    for( auto& o : opts )
        if( o )
            res.emplace_back( std::move( *o ) );

    return res;
}

// This will take the vectors of optionals and will gather all of
// them that are not nullopt and move their values into a  vector
// and return it.
template<typename T>
std::vector<T> cat_opts( std::vector<std::optional<T>> const& opts ) {
    // We might need up to this size.
    std::vector<T> res; res.reserve( opts.size() );
    for( auto const& o : opts )
        if( o )
            res.push_back( *o );

    return res;
}

} // namespace util
