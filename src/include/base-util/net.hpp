/****************************************************************
* Network Utilities
****************************************************************/
#pragma once

#include "string.hpp"

#include <string>
#include <string_view>

namespace net {

// URL-encode a string.
std::string url_encode( std::string_view in );

// This function will accept some kind  of range / container that
// will yield either pairs or 2-tuples of strings. It  will  then
// url-encode  each  key/value pair, join each pair with an equal
// sign, then join the pairs with &.  E.g., result is of the form
// A=B&C=D&E=F.
template<typename KeyValT>
std::string url_encode_kv( KeyValT const& kv ) {

    std::vector<std::string> terms( kv.size() );

    auto encode_pair = []( auto const& p ) {
        return url_encode( std::get<0>( p ) ) + '=' +
               url_encode( std::get<1>( p ) );
    };

    // Encode each key/value pair.
    std::transform( std::begin( kv ),    std::end( kv ),
                    std::begin( terms ), encode_pair );

    return util::join( terms, "&" );
}

} // namespace net
