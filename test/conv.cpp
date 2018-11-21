/****************************************************************
* Unit tests for encoding conversions
****************************************************************/
#include "infra/common.hpp"

#include "base-util/conv.hpp"
#include "base-util/io.hpp"
#include "base-util/types.hpp"
#include "base-util/string.hpp"
#include "base-util/misc.hpp"

#include "catch2/catch.hpp"

using namespace std;

TEST_CASE( "ascii_2_utf16le" )
{
    auto f = []( vector<char> const& v, bool bom = false ) {
        return conv::ascii_2_utf16le( v, bom );
    };

    auto b0 = (char)0xFF, b1 = (char)0xFE, _0 = (char)0x00;

    vector<char> empty;
    REQUIRE( f( empty, false ) == vector<char>{} );
    REQUIRE( f( empty, true  ) == vector<char>{} );

    vector<char> one{ 'A' };
    REQUIRE( f( one, false ) == vector<char>{ 'A', _0 } );
    REQUIRE( f( one, true  ) == vector<char>{ b0, b1, 'A', _0 } );

    vector<char> two{ 'A', 'B' };
    REQUIRE( f( two, false ) == vector<char>{ 'A', _0, 'B', _0 } );
    REQUIRE( f( two, true  ) == vector<char>{ b0, b1, 'A', _0, 'B', _0 } );

    vector<char> lines{ 'A', 0x0A, 'B', 0x0A };
    REQUIRE( f( lines, false ) ==
             vector<char>{ 'A', _0, 0x0A, _0, 'B', _0, 0x0A, _0 } );
    REQUIRE( f( lines, true  ) ==
             vector<char>{ b0, b1, 'A', _0, 0x0A, _0, 'B', _0, 0x0A, _0 } );

    vector<char> invalid1{ 'A', (char)0x80, 'B' };
    REQUIRE_THROWS( f( invalid1 ) );
    vector<char> invalid2{ 'A', (char)0xFF, 'B' };
    REQUIRE_THROWS( f( invalid2 ) );

    auto tmp = fs::temp_directory_path()/"3-lines.txt";
    util::copy_file( ::data_common/"3-lines.txt", tmp );
    conv::ascii_2_utf16le( tmp );
    auto v = util::read_file( tmp );
    REQUIRE( v.size() == 44 );

    REQUIRE( v[0]  == b0  );
    REQUIRE( v[1]  == b1  );
    REQUIRE( v[4]  == 'i' );
    REQUIRE( v[5]  == _0  );
    REQUIRE( v[16] == 'l' );
    REQUIRE( v[17] == _0  );
}
