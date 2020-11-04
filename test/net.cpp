/****************************************************************
* Unit tests for network functionality
****************************************************************/
#include "infra/common.hpp"

#include "base-util/bimap.hpp"
#include "base-util/io.hpp"
#include "base-util/net.hpp"
#include "base-util/types.hpp"

#include "catch2/catch.hpp"

using namespace std;

TEST_CASE( "url_encode" )
{
    auto s_inp  = util::read_file_as_string( data_common/"encode-inp.txt"  );
    auto s_base = util::read_file_as_string( data_common/"encode-base.txt" );
    REQUIRE( s_inp.has_value() );
    REQUIRE( s_base.has_value() );

    auto s_enc = net::url_encode( *s_inp );

    REQUIRE( s_enc.size() == s_base->size() );
    REQUIRE( s_enc == *s_base );

    // Test encoding of key/value pairs from vector.
    vector<tuple<string, string>> kv1{
        { "hello",       "world"                  },
        { "func",        "{ cout << \"hello\"; }" },
        { "with spaces", "with&amp"               },
        { "empty-val",   ""                       }
    };

    string target =
        "hello=world&"
        "func=%7B%20cout%20%3C%3C%20%22hello%22%3B%20%7D&"
        "with%20spaces=with%26amp&"
        "empty-val=";

    REQUIRE( net::url_encode_kv( kv1 ) == target );

    // Test another, map-like container.
    util::BiMapFixed<string, string> bm( move( kv1 ) );

    // This container will yield elements sorted by key.
    target = "empty-val=&"
             "func=%7B%20cout%20%3C%3C%20%22hello%22%3B%20%7D&"
             "hello=world&"
             "with%20spaces=with%26amp";

    REQUIRE( net::url_encode_kv( bm ) == target );
}
