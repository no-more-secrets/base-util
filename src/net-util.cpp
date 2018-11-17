/****************************************************************
* Network Utilities
****************************************************************/
#include "base-util/net-util.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;

namespace net {

// URL-encode a string.
//
// Implementation is adapted from code found here:
//
//   stackoverflow.com/questions/154536/encode-decode-urls-in-c
string url_encode( string_view in ) {

    ostringstream escaped;
    escaped.fill( '0' );
    escaped << hex;

    for( auto c : in ) {

        // Keep alpha-num and other  accepted  characters  intact.
        if( isalnum( c ) || c == '-' || c == '_' ||
                            c == '.' || c == '~' ) {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded.
        escaped << uppercase;
        escaped << '%' << setw( 2 ) << int( (unsigned char)c );
        escaped << nouppercase;
    }

    return escaped.str();
}

} // net
