/****************************************************************
* Unit tests
****************************************************************/
#include "catch2/catch.hpp"

#include "base-util/string.hpp"

using namespace std;

TEST_CASE( "from_string" )
{
    REQUIRE( !util::stoi( "" ).has_value() );

    REQUIRE( util::stoi( "0"      ) ==  0   );
    REQUIRE( util::stoi( "1"      ) ==  1   );
    REQUIRE( util::stoi( "222"    ) ==  222 );
    REQUIRE( util::stoi( "0",  16 ) ==  0   );
    REQUIRE( util::stoi( "10", 16 ) ==  16  );
    REQUIRE( util::stoi( "-10"    ) == -10  );
    REQUIRE( util::stoi( "-0"     ) ==  0   );
}

TEST_CASE( "common_prefix" )
{
    using util::common_prefix;
    vector<string> v;

    v = {};
    REQUIRE_FALSE( common_prefix( v ).has_value() );

    v = {""};
    REQUIRE( common_prefix( v ) == "" );
    v = {"", ""};
    REQUIRE( common_prefix( v ) == "" );
    v = {"a", ""};
    REQUIRE( common_prefix( v ) == "" );
    v = {"", "a"};
    REQUIRE( common_prefix( v ) == "" );
    v = {"", "ab"};
    REQUIRE( common_prefix( v ) == "" );
    v = {"ab", ""};
    REQUIRE( common_prefix( v ) == "" );
    v = {"a", "b"};
    REQUIRE( common_prefix( v ) == "" );
    v = {"ab", "bb"};
    REQUIRE( common_prefix( v ) == "" );
    v = {"a", "a"};
    REQUIRE( common_prefix( v ) == "a" );
    v = {"aa", "ab"};
    REQUIRE( common_prefix( v ) == "a" );
    v = {"aa", "aa"};
    REQUIRE( common_prefix( v ) == "aa" );
    v = {"ab", "ab"};
    REQUIRE( common_prefix( v ) == "ab" );
    v = {"ab", "ab", "a"};
    REQUIRE( common_prefix( v ) == "a" );
    v = {"ab", "", "ab"};
    REQUIRE( common_prefix( v ) == "" );
    v = {"abcd", "abvd", "abcd"};
    REQUIRE( common_prefix( v ) == "ab" );
    v = {"abcd", "abcd", "abcd"};
    REQUIRE( common_prefix( v ) == "abcd" );
    v = {"abcd.efg", "abcd.efh", "abcd.ehf"};
    REQUIRE( common_prefix( v ) == "abcd.e" );
    v = {"abcd.efg", "abc", "abcd.efghi"};
    REQUIRE( common_prefix( v ) == "abc" );
}

TEST_CASE( "split_join" )
{
    SVVec v_{ "" };
    REQUIRE( util::join( v_, "," ) == "" );

    SVVec v0{ "one" };
    REQUIRE( util::join( v0, "," ) == "one" );

    SVVec v{ "one", "two", "three" };
    REQUIRE( util::join( v, "," ) == "one,two,three" );
    REQUIRE( util::join( v, "--" ) == "one--two--three" );

    SVVec svv_{ "" };
    REQUIRE( util::split( "", ',' ) == svv_ );

    SVVec svv0{ "ab" };
    REQUIRE( util::split( "ab", ',' ) == svv0 );

    SVVec svv{ "ab", "cd", "ef" };
    REQUIRE( util::split( "ab,cd,ef", ',' ) == svv );

    SVVec svv2{ "ab", "cd", "ef" };
    REQUIRE( util::split_on_any( "ab,cd-ef", ",-" ) == svv );

    REQUIRE( util::join( util::split( "ab,cd,ef", ',' ), "," ) ==
            "ab,cd,ef" );

    REQUIRE( util::split_strip( " ab ,cd   ,ef   ", ',' ) ==
            (SVVec{"ab","cd","ef"}) );

    REQUIRE( util::split_strip_any( " ab\n,\nx\ncd   ,ef   ", ",\n" ) ==
            (SVVec{"ab","x","cd","ef"}) );
}

TEST_CASE( "wrap" )
{
    REQUIRE( util::wrap_text( "", 0 ) == vector<string>() );
    REQUIRE( util::wrap_text( "", 2 ) == vector<string>() );

    REQUIRE( util::wrap_text( "a", 0 ) == vector<string>{"a"} );
    REQUIRE( util::wrap_text( "a", 1 ) == vector<string>{"a"} );
    REQUIRE( util::wrap_text( "a", 2 ) == vector<string>{"a"} );

    REQUIRE( util::wrap_text( "abc", 0 ) == vector<string>{"abc"} );
    REQUIRE( util::wrap_text( "abc", 1 ) == vector<string>{"abc"} );
    REQUIRE( util::wrap_text( "abc", 5 ) == vector<string>{"abc"} );

    REQUIRE( util::wrap_text( "abc def", 0 ) == (vector<string>{"abc","def"}) );
    REQUIRE( util::wrap_text( "abc def", 2 ) == (vector<string>{"abc","def"}) );
    REQUIRE( util::wrap_text( "abc def", 5 ) == (vector<string>{"abc","def"}) );
    REQUIRE( util::wrap_text( "abc def", 6 ) == (vector<string>{"abc","def"}) );
    REQUIRE( util::wrap_text( "abc def", 7 ) == (vector<string>{"abc def"}) );
    REQUIRE( util::wrap_text( "abc def", 8 ) == (vector<string>{"abc def"}) );

    REQUIRE( util::wrap_text( "abc\ndef", 8 ) == (vector<string>{"abc def"}) );
    REQUIRE( util::wrap_text( "abc\n\n  def", 8 ) == (vector<string>{"abc def"}) );
    REQUIRE( util::wrap_text( "  abc  def   ", 8 ) == (vector<string>{"abc def"}) );

    string text1 = "Ask not what your country can do for you "
        "but instead ask what you can do for your country.";

    vector<string> res012345 =
        {"Ask","not","what","your","country","can","do","for",
         "you","but","instead","ask","what","you","can","do",
         "for","your","country."};
    REQUIRE( util::wrap_text( text1, 0 ) == res012345 );
    REQUIRE( util::wrap_text( text1, 1 ) == res012345 );
    REQUIRE( util::wrap_text( text1, 2 ) == res012345 );
    REQUIRE( util::wrap_text( text1, 3 ) == res012345 );
    REQUIRE( util::wrap_text( text1, 4 ) == res012345 );
    REQUIRE( util::wrap_text( text1, 5 ) == res012345 );

    vector<string> res6 =
        {"Ask","not","what","your","country","can do","for",
         "you","but","instead","ask","what","you","can do",
         "for","your","country."};
    REQUIRE( util::wrap_text( text1, 6 ) == res6 );

    vector<string> res7 =
        {"Ask not","what","your","country","can do","for you",
         "but","instead","ask","what","you can", "do for",
         "your","country."};
    REQUIRE( util::wrap_text( text1, 7 ) == res7 );

    vector<string> res8 =
        {"Ask not","what","your","country","can do","for you",
         "but","instead","ask what","you can", "do for",
         "your","country."};
    REQUIRE( util::wrap_text( text1, 8 ) == res8 );

    vector<string> res9 =
        {"Ask not","what your","country","can do","for you",
         "but","instead","ask what","you can", "do for",
         "your","country."};
    REQUIRE( util::wrap_text( text1, 9 ) == res9 );

    vector<string> res10 =
        {"Ask not","what your","country","can do for", "you but",
         "instead","ask what","you can do", "for your", "country."};
    REQUIRE( util::wrap_text( text1, 10 ) == res10 );

    vector<string> res11 =
        {"Ask not","what your","country can","do for you",
         "but instead","ask what","you can do", "for your", "country."};
    REQUIRE( util::wrap_text( text1, 11 ) == res11 );

    vector<string> res12 =
        {"Ask not what","your country","can do for","you but",
         "instead ask","what you can","do for your", "country."};
    REQUIRE( util::wrap_text( text1, 12 ) == res12 );

    vector<string> res13 =
        {"Ask not what","your country","can do for","you but",
         "instead ask","what you can","do for your", "country."};
    REQUIRE( util::wrap_text( text1, 13 ) == res13 );

    vector<string> res14 =
        {"Ask not what","your country","can do for you","but instead",
         "ask what you","can do for","your country."};
    REQUIRE( util::wrap_text( text1, 14 ) == res14 );

    vector<string> res15 =
        {"Ask not what","your country","can do for you","but instead ask",
         "what you can do","for your","country."};
    REQUIRE( util::wrap_text( text1, 15 ) == res15 );

    vector<string> res16 =
        {"Ask not what","your country can","do for you but",
         "instead ask what","you can do for","your country."};
    REQUIRE( util::wrap_text( text1, 16 ) == res16 );

    vector<string> res17 =
        {"Ask not what your","country can do","for you but",
         "instead ask what","you can do for","your country."};
    REQUIRE( util::wrap_text( text1, 17 ) == res17 );

    vector<string> res18 =
        {"Ask not what your","country can do for","you but instead",
         "ask what you can","do for your","country."};
    REQUIRE( util::wrap_text( text1, 18 ) == res18 );

    vector<string> res19 =
        {"Ask not what your","country can do for","you but instead ask",
         "what you can do for","your country."};
    REQUIRE( util::wrap_text( text1, 19 ) == res19 );

    vector<string> res20 =
        {"Ask not what your","country can do for","you but instead ask",
         "what you can do for","your country."};
    REQUIRE( util::wrap_text( text1, 20 ) == res20 );

    vector<string> res21 =
        //---------------------
        {"Ask not what your",
         "country can do for",
         "you but instead ask",
         "what you can do for",
         "your country."};
    REQUIRE( util::wrap_text( text1, 21 ) == res21 );

    vector<string> res40 =
        //----------------------------------------
        {"Ask not what your country can do for you",
         "but instead ask what you can do for your",
         "country."};
    REQUIRE( util::wrap_text( text1, 40 ) == res40 );

    vector<string> res80 =
        //--------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for",
         "your country."};
    REQUIRE( util::wrap_text( text1, 80 ) == res80 );

    vector<string> res87 =
        //---------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    REQUIRE( util::wrap_text( text1, 87 ) == res87 );

    vector<string> res88 =
        //----------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    REQUIRE( util::wrap_text( text1, 88 ) == res88 );

    vector<string> res89 =
        //-----------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your",
         "country."};
    REQUIRE( util::wrap_text( text1, 89 ) == res89 );

    vector<string> res90 =
        //------------------------------------------------------------------------------------------
        {"Ask not what your country can do for you but instead ask what you can do for your country."};
    REQUIRE( util::wrap_text( text1, 90 ) == res90 );
}

TEST_CASE( "to_string" )
{
    REQUIRE( util::to_string( 5    ) == "5"        );
    REQUIRE( util::to_string( 5.5  ) == "5.500000" );
    REQUIRE( util::to_string( true ) == "1"        );
    REQUIRE( util::to_string( 'a'  ) == "'a'"      );
    REQUIRE( util::to_string( "a"  ) == "\"a\""    );

    string s = "xyz";
    REQUIRE( util::to_string( s ) == "\"xyz\"" );

    tuple<int, string, double> tp{ 5, "david", 67.9 };
    vector<int> v1{ 3, 4, 5 };
    vector<string> v2{ "A", "B", "C" };

    REQUIRE( util::to_string( tp ) == "(5,\"david\",67.900000)" );
    REQUIRE( util::to_string( v1 ) == "[3,4,5]"                 );
    REQUIRE( util::to_string( v2 ) == "[\"A\",\"B\",\"C\"]"     );

    char c = 'c'; auto rw = cref( c );
    REQUIRE( util::to_string( rw ) == "'c'" );

    OptStr opt;
    REQUIRE( util::to_string( opt ) == "nullopt" );
    opt = "something";
    REQUIRE( util::to_string( opt ) == "\"something\"" );

    variant<int, string> var = 5;
    REQUIRE( util::to_string( var ) == "5" );
    var = "hello";
    REQUIRE( util::to_string( var ) == "\"hello\"" );

    vector<tuple<int, string>> v3{ {5,"a"}, {6,"b"} };
    REQUIRE( util::to_string( v3 ) == "[(5,\"a\"),(6,\"b\")]" );

    fs::path p = "A/B/C";
    REQUIRE( util::to_string( p ) == "\"A/B/C\"" );

    auto now = chrono::system_clock::now();
    auto now_str = util::to_string( now );
    REQUIRE( now_str.size() == 29 );
    auto now_zoned = ZonedTimePoint( now, util::tz_utc() );
    auto now_zoned_str = util::to_string( now_zoned );
    REQUIRE( now_zoned_str.size() == 34 );
}

TEST_CASE( "string_util" )
{
    bool b;

    /*************************************************************
    * starts_with / ends_with
    *************************************************************/
    b = util::starts_with( ""      , ""      ); REQUIRE( b == true  );
    b = util::starts_with( "x"     , ""      ); REQUIRE( b == true  );
    b = util::starts_with( ""      , "x"     ); REQUIRE( b == false );
    b = util::starts_with( "xxx"   , ""      ); REQUIRE( b == true  );
    b = util::starts_with( ""      , "xxx"   ); REQUIRE( b == false );
    b = util::starts_with( "abcde" , "abcde" ); REQUIRE( b == true  );
    b = util::starts_with( "abcde" , "a"     ); REQUIRE( b == true  );
    b = util::starts_with( "abcde" , "ab"    ); REQUIRE( b == true  );
    b = util::starts_with( "abcde" , "abcd"  ); REQUIRE( b == true  );
    b = util::starts_with( "abcde" , "abfd"  ); REQUIRE( b == false );
    b = util::starts_with( "abcde" , "abfdx" ); REQUIRE( b == false );
    b = util::starts_with( "abcde ", "abcd"  ); REQUIRE( b == true  );

    b = util::ends_with( ""      , ""      ); REQUIRE( b == true  );
    b = util::ends_with( "x"     , ""      ); REQUIRE( b == true  );
    b = util::ends_with( ""      , "x"     ); REQUIRE( b == false );
    b = util::ends_with( "xxx"   , ""      ); REQUIRE( b == true  );
    b = util::ends_with( ""      , "xxx"   ); REQUIRE( b == false );
    b = util::ends_with( "abcde" , "abcde" ); REQUIRE( b == true  );
    b = util::ends_with( "abcde" , "e"     ); REQUIRE( b == true  );
    b = util::ends_with( "abcde" , "de"    ); REQUIRE( b == true  );
    b = util::ends_with( "abcde" , "bcde"  ); REQUIRE( b == true  );
    b = util::ends_with( "abcde" , "bcfe"  ); REQUIRE( b == false );
    b = util::ends_with( "abcde" , "xbcfe" ); REQUIRE( b == false );
    b = util::ends_with( " abcde", "bcde"  ); REQUIRE( b == true  );

    /*************************************************************
    * string comparison
    *************************************************************/
    b = util::iequals<string>( ""      , ""       ); REQUIRE( b == true  );
    b = util::iequals<string>( "x"     , ""       ); REQUIRE( b == false );
    b = util::iequals<string>( ""      , "x"      ); REQUIRE( b == false );
    b = util::iequals<string>( "x"     , "x"      ); REQUIRE( b == true  );
    b = util::iequals<string>( "X"     , "x"      ); REQUIRE( b == true  );
    b = util::iequals<string>( "x"     , "X"      ); REQUIRE( b == true  );
    b = util::iequals<string>( "abcde" , "abcde"  ); REQUIRE( b == true  );
    b = util::iequals<string>( "aBCde" , "abcde"  ); REQUIRE( b == true  );
    b = util::iequals<string>( "abcde" , "abcdex" ); REQUIRE( b == false );
    b = util::iequals<string>( "abcdex", "abcde"  ); REQUIRE( b == false );
    b = util::iequals<string>( "abcde" , "xabcde" ); REQUIRE( b == false );
    b = util::iequals<string>( "xabcde", "abcde"  ); REQUIRE( b == false );
    b = util::iequals<string>( "ABCDE",  "abcde"  ); REQUIRE( b == true  );
}
