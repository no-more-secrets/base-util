/****************************************************************
* Unit tests for network functionality
****************************************************************/
#include "common.hpp"

#include "base-util/bimap.hpp"
#include "base-util/io.hpp"
#include "base-util/net-util.hpp"
#include "base-util/types.hpp"

using namespace std;

namespace testing {

TEST( url_encode )
{
    auto s_inp  = util::read_file_str( data_common/"encode-inp.txt"  );
    auto s_base = util::read_file_str( data_common/"encode-base.txt" );

    auto s_enc = net::url_encode( s_inp );

    EQUALS( s_enc.size(), s_base.size() );
    EQUALS( s_enc, s_base );

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

    EQUALS( net::url_encode_kv( kv1 ), target );

    // Test another, map-like container.
    util::BiMapFixed<string, string> bm( move( kv1 ) );

    // This container will yield elements sorted by key.
    target = "empty-val=&"
             "func=%7B%20cout%20%3C%3C%20%22hello%22%3B%20%7D&"
             "hello=world&"
             "with%20spaces=with%26amp";

    EQUALS( net::url_encode_kv( bm ), target );
}

} // namespace testing
