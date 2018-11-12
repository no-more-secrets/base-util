/****************************************************************
* Unit tests
****************************************************************/
#include "common.hpp"

#include "base-util/string-util.hpp"

using namespace std;

fs::path const data_common = "tests/data";

namespace testing {

TEST( from_string )
{
    THROWS( util::stoi( "" ) );

    EQUALS( util::stoi( "0"      ),  0   );
    EQUALS( util::stoi( "1"      ),  1   );
    EQUALS( util::stoi( "222"    ),  222 );
    EQUALS( util::stoi( "0",  16 ),  0   );
    EQUALS( util::stoi( "10", 16 ),  16  );
    EQUALS( util::stoi( "-10"    ), -10  );
    EQUALS( util::stoi( "-0"     ),  0   );
}

TEST( split_join )
{
    SVVec v_{ "" };
    EQUALS( util::join( v_, "," ), "" );

    SVVec v0{ "one" };
    EQUALS( util::join( v0, "," ), "one" );

    SVVec v{ "one", "two", "three" };
    EQUALS( util::join( v, "," ), "one,two,three" );
    EQUALS( util::join( v, "--" ), "one--two--three" );

    SVVec svv_{ "" };
    EQUALS( util::split( "", ',' ), svv_ );

    SVVec svv0{ "ab" };
    EQUALS( util::split( "ab", ',' ), svv0 );

    SVVec svv{ "ab", "cd", "ef" };
    EQUALS( util::split( "ab,cd,ef", ',' ), svv );

    SVVec svv2{ "ab", "cd", "ef" };
    EQUALS( util::split_on_any( "ab,cd-ef", ",-" ), svv );

    EQUALS( util::join( util::split( "ab,cd,ef", ',' ), "," ),
            "ab,cd,ef" );

    EQUALS( util::split_strip( " ab ,cd   ,ef   ", ',' ),
            (SVVec{"ab","cd","ef"}) );

    EQUALS( util::split_strip_any( " ab\n,\nx\ncd   ,ef   ", ",\n" ),
            (SVVec{"ab","x","cd","ef"}) );
}

TEST( wrap )
{
    EQUALS( util::wrap_text( "", 0 ), vector<string>() );
    EQUALS( util::wrap_text( "", 2 ), vector<string>() );

    EQUALS( util::wrap_text( "a", 0 ), vector<string>{"a"} );
    EQUALS( util::wrap_text( "a", 1 ), vector<string>{"a"} );
    EQUALS( util::wrap_text( "a", 2 ), vector<string>{"a"} );

    EQUALS( util::wrap_text( "abc", 0 ), vector<string>{"abc"} );
    EQUALS( util::wrap_text( "abc", 1 ), vector<string>{"abc"} );
    EQUALS( util::wrap_text( "abc", 5 ), vector<string>{"abc"} );

    EQUALS( util::wrap_text( "abc def", 0 ), (vector<string>{"abc","def"}) );
    EQUALS( util::wrap_text( "abc def", 2 ), (vector<string>{"abc","def"}) );
    EQUALS( util::wrap_text( "abc def", 5 ), (vector<string>{"abc","def"}) );
    EQUALS( util::wrap_text( "abc def", 6 ), (vector<string>{"abc","def"}) );
    EQUALS( util::wrap_text( "abc def", 7 ), (vector<string>{"abc def"}) );
    EQUALS( util::wrap_text( "abc def", 8 ), (vector<string>{"abc def"}) );

    EQUALS( util::wrap_text( "abc\ndef", 8 ), (vector<string>{"abc def"}) );
    EQUALS( util::wrap_text( "abc\n\n  def", 8 ), (vector<string>{"abc def"}) );
    EQUALS( util::wrap_text( "  abc  def   ", 8 ), (vector<string>{"abc def"}) );

    string text1 = "Ask not what your country can do for you "
        "but instead ask what you can do for your country.";

    vector<string> res012345 =
        {"Ask","not","what","your","country","can","do","for",
         "you","but","instead","ask","what","you","can","do",
         "for","your","country."};
    EQUALS( util::wrap_text( text1, 0 ), res012345 );
    EQUALS( util::wrap_text( text1, 1 ), res012345 );
    EQUALS( util::wrap_text( text1, 2 ), res012345 );
    EQUALS( util::wrap_text( text1, 3 ), res012345 );
    EQUALS( util::wrap_text( text1, 4 ), res012345 );
    EQUALS( util::wrap_text( text1, 5 ), res012345 );

    vector<string> res6 =
        {"Ask","not","what","your","country","can do","for",
         "you","but","instead","ask","what","you","can do",
         "for","your","country."};
    EQUALS( util::wrap_text( text1, 6 ), res6 );

    vector<string> res7 =
        {"Ask not","what","your","country","can do","for you",
         "but","instead","ask","what","you can", "do for",
         "your","country."};
    EQUALS( util::wrap_text( text1, 7 ), res7 );

    vector<string> res8 =
        {"Ask not","what","your","country","can do","for you",
         "but","instead","ask what","you can", "do for",
         "your","country."};
    EQUALS( util::wrap_text( text1, 8 ), res8 );

    vector<string> res9 =
        {"Ask not","what your","country","can do","for you",
         "but","instead","ask what","you can", "do for",
         "your","country."};
    EQUALS( util::wrap_text( text1, 9 ), res9 );

    vector<string> res10 =
        {"Ask not","what your","country","can do for", "you but",
         "instead","ask what","you can do", "for your", "country."};
    EQUALS( util::wrap_text( text1, 10 ), res10 );

    vector<string> res11 =
        {"Ask not","what your","country can","do for you",
         "but instead","ask what","you can do", "for your", "country."};
    EQUALS( util::wrap_text( text1, 11 ), res11 );

    vector<string> res12 =
        {"Ask not what","your country","can do for","you but",
         "instead ask","what you can","do for your", "country."};
    EQUALS( util::wrap_text( text1, 12 ), res12 );

    vector<string> res13 =
        {"Ask not what","your country","can do for","you but",
         "instead ask","what you can","do for your", "country."};
    EQUALS( util::wrap_text( text1, 13 ), res13 );

    vector<string> res14 =
        {"Ask not what","your country","can do for you","but instead",
         "ask what you","can do for","your country."};
    EQUALS( util::wrap_text( text1, 14 ), res14 );

    vector<string> res15 =
        {"Ask not what","your country","can do for you","but instead ask",
         "what you can do","for your","country."};
    EQUALS( util::wrap_text( text1, 15 ), res15 );

    vector<string> res16 =
        {"Ask not what","your country can","do for you but",
         "instead ask what","you can do for","your country."};
    EQUALS( util::wrap_text( text1, 16 ), res16 );

    vector<string> res17 =
        {"Ask not what your","country can do","for you but",
         "instead ask what","you can do for","your country."};
    EQUALS( util::wrap_text( text1, 17 ), res17 );

    vector<string> res18 =
        {"Ask not what your","country can do for","you but instead",
         "ask what you can","do for your","country."};
    EQUALS( util::wrap_text( text1, 18 ), res18 );

    vector<string> res19 =
        {"Ask not what your","country can do for","you but instead ask",
         "what you can do for","your country."};
    EQUALS( util::wrap_text( text1, 19 ), res19 );

    vector<string> res20 =
        {"Ask not what your","country can do for","you but instead ask",
         "what you can do for","your country."};
    EQUALS( util::wrap_text( text1, 20 ), res20 );

    vector<string> res21 =
        //---------------------
        {"Ask not what your",
         "country can do for",
         "you but instead ask",
         "what you can do for",
         "your country."};
    EQUALS( util::wrap_text( text1, 21 ), res21 );

    vector<string> res40 =
        //----------------------------------------
        {"Ask not what your country can do for you",
         "but instead ask what you can do for your",
         "country."};
    EQUALS( util::wrap_text( text1, 40 ), res40 );

    vector<string> res80 =
        //--------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for",
         "your country."};
    EQUALS( util::wrap_text( text1, 80 ), res80 );

    vector<string> res87 =
        //---------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    EQUALS( util::wrap_text( text1, 87 ), res87 );

    vector<string> res88 =
        //----------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    EQUALS( util::wrap_text( text1, 88 ), res88 );

    vector<string> res89 =
        //-----------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    EQUALS( util::wrap_text( text1, 89 ), res89 );

    vector<string> res90 =
        //------------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your country."};
    EQUALS( util::wrap_text( text1, 90 ), res90 );
}

TEST( to_string )
{
    EQUALS( util::to_string( 5    ), "5"        );
    EQUALS( util::to_string( 5.5  ), "5.500000" );
    EQUALS( util::to_string( true ), "1"        );
    EQUALS( util::to_string( 'a'  ), "'a'"      );
    EQUALS( util::to_string( "a"  ), "\"a\""    );

    string s = "xyz";
    EQUALS( util::to_string( s ), "\"xyz\"" );

    tuple<int, string, double> tp{ 5, "david", 67.9 };
    vector<int> v1{ 3, 4, 5 };
    vector<string> v2{ "A", "B", "C" };

    EQUALS( util::to_string( tp ), "(5,\"david\",67.900000)" );
    EQUALS( util::to_string( v1 ), "[3,4,5]"                 );
    EQUALS( util::to_string( v2 ), "[\"A\",\"B\",\"C\"]"     );

    char c = 'c'; auto rw = cref( c );
    EQUALS( util::to_string( rw ), "'c'" );

    OptStr opt;
    EQUALS( util::to_string( opt ), "nullopt" );
    opt = "something";
    EQUALS( util::to_string( opt ), "\"something\"" );

    variant<int, string> var = 5;
    EQUALS( util::to_string( var ), "5" );
    var = "hello";
    EQUALS( util::to_string( var ), "\"hello\"" );

    vector<tuple<int, string>> v3{ {5,"a"}, {6,"b"} };
    EQUALS( util::to_string( v3 ), "[(5,\"a\"),(6,\"b\")]" );

    fs::path p = "A/B/C";
    EQUALS( util::to_string( p ), "\"A/B/C\"" );

    auto now = chrono::system_clock::now();
    auto now_str = util::to_string( now );
    EQUALS( now_str.size(), 29 );
    auto now_zoned = ZonedTimePoint( now, util::tz_utc() );
    auto now_zoned_str = util::to_string( now_zoned );
    EQUALS( now_zoned_str.size(), 34 );
}

TEST( string_util )
{
    bool b;

    /*************************************************************
    * starts_with / ends_with
    *************************************************************/
    b = util::starts_with( ""      , ""      ); EQUALS( b, true  );
    b = util::starts_with( "x"     , ""      ); EQUALS( b, true  );
    b = util::starts_with( ""      , "x"     ); EQUALS( b, false );
    b = util::starts_with( "xxx"   , ""      ); EQUALS( b, true  );
    b = util::starts_with( ""      , "xxx"   ); EQUALS( b, false );
    b = util::starts_with( "abcde" , "abcde" ); EQUALS( b, true  );
    b = util::starts_with( "abcde" , "a"     ); EQUALS( b, true  );
    b = util::starts_with( "abcde" , "ab"    ); EQUALS( b, true  );
    b = util::starts_with( "abcde" , "abcd"  ); EQUALS( b, true  );
    b = util::starts_with( "abcde" , "abfd"  ); EQUALS( b, false );
    b = util::starts_with( "abcde" , "abfdx" ); EQUALS( b, false );
    b = util::starts_with( "abcde ", "abcd"  ); EQUALS( b, true  );

    b = util::ends_with( ""      , ""      ); EQUALS( b, true  );
    b = util::ends_with( "x"     , ""      ); EQUALS( b, true  );
    b = util::ends_with( ""      , "x"     ); EQUALS( b, false );
    b = util::ends_with( "xxx"   , ""      ); EQUALS( b, true  );
    b = util::ends_with( ""      , "xxx"   ); EQUALS( b, false );
    b = util::ends_with( "abcde" , "abcde" ); EQUALS( b, true  );
    b = util::ends_with( "abcde" , "e"     ); EQUALS( b, true  );
    b = util::ends_with( "abcde" , "de"    ); EQUALS( b, true  );
    b = util::ends_with( "abcde" , "bcde"  ); EQUALS( b, true  );
    b = util::ends_with( "abcde" , "bcfe"  ); EQUALS( b, false );
    b = util::ends_with( "abcde" , "xbcfe" ); EQUALS( b, false );
    b = util::ends_with( " abcde", "bcde"  ); EQUALS( b, true  );

    /*************************************************************
    * string comparison
    *************************************************************/
    b = util::iequals<string>( ""      , ""       ); EQUALS( b, true  );
    b = util::iequals<string>( "x"     , ""       ); EQUALS( b, false );
    b = util::iequals<string>( ""      , "x"      ); EQUALS( b, false );
    b = util::iequals<string>( "x"     , "x"      ); EQUALS( b, true  );
    b = util::iequals<string>( "X"     , "x"      ); EQUALS( b, true  );
    b = util::iequals<string>( "x"     , "X"      ); EQUALS( b, true  );
    b = util::iequals<string>( "abcde" , "abcde"  ); EQUALS( b, true  );
    b = util::iequals<string>( "aBCde" , "abcde"  ); EQUALS( b, true  );
    b = util::iequals<string>( "abcde" , "abcdex" ); EQUALS( b, false );
    b = util::iequals<string>( "abcdex", "abcde"  ); EQUALS( b, false );
    b = util::iequals<string>( "abcde" , "xabcde" ); EQUALS( b, false );
    b = util::iequals<string>( "xabcde", "abcde"  ); EQUALS( b, false );
    b = util::iequals<string>( "ABCDE",  "abcde"  ); EQUALS( b, true  );
}

} // namespace testing
