/****************************************************************
* Unit tests for encoding conversions
****************************************************************/
#include "common.hpp"

#include "base-util/conv.hpp"
#include "base-util/io.hpp"
#include "base-util/types.hpp"
#include "base-util/string.hpp"
#include "base-util/misc.hpp"

using namespace std;

namespace testing {

TEST( ascii_2_utf16le )
{
    auto f = []( vector<char> const& v, bool bom = false ) {
        return conv::ascii_2_utf16le( v, bom );
    };

    auto b0 = (char)0xFF, b1 = (char)0xFE, _0 = (char)0x00;

    vector<char> empty;
    EQUALS( f( empty, false ), (vector<char>{}) );
    EQUALS( f( empty, true  ), (vector<char>{}) );

    vector<char> one{ 'A' };
    EQUALS( f( one, false ), (vector<char>{ 'A', _0 }) );
    EQUALS( f( one, true  ), (vector<char>{ b0, b1, 'A', _0 }) );

    vector<char> two{ 'A', 'B' };
    EQUALS( f( two, false ), (vector<char>{ 'A', _0, 'B', _0 }) );
    EQUALS( f( two, true  ), (vector<char>{ b0, b1, 'A', _0, 'B', _0 }) );

    vector<char> lines{ 'A', 0x0A, 'B', 0x0A };
    EQUALS( f( lines, false ),
             (vector<char>{ 'A', _0, 0x0A, _0, 'B', _0, 0x0A, _0 }) );
    EQUALS( f( lines, true  ),
             (vector<char>{ b0, b1, 'A', _0, 0x0A, _0, 'B', _0, 0x0A, _0 }) );

    vector<char> invalid1{ 'A', (char)0x80, 'B' };
    THROWS( f( invalid1 ) );
    vector<char> invalid2{ 'A', (char)0xFF, 'B' };
    THROWS( f( invalid2 ) );

    auto tmp = fs::temp_directory_path()/"3-lines.txt";
    util::copy_file( data_common/"3-lines.txt", tmp );
    conv::ascii_2_utf16le( tmp );
    auto v = util::read_file( tmp );
    EQUALS( v.size(), 44 );

    EQUALS( v[0],  b0  );
    EQUALS( v[1],  b1  );
    EQUALS( v[4],  'i' );
    EQUALS( v[5],  _0  );
    EQUALS( v[16], 'l' );
    EQUALS( v[17], _0  );
}

} // namespace testing
