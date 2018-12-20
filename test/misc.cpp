/****************************************************************
* Unit tests
****************************************************************/
#include "catch2/catch.hpp"

#include "base-util/graph.hpp"
#include "base-util/io.hpp"
#include "base-util/logger.hpp"
#include "base-util/optional.hpp"
#include "base-util/string.hpp"

#include <type_traits>

using namespace std;
using namespace std::string_literals;

using ::Catch::Matches; // regex matcher

// Compile-time test that we can log IO manipulators such as endl.
using io_manip_valid = decltype( operator<<( util::log, endl ) );

TEST_CASE( "datetime" )
{
    // Parenthesis in regex's are for raw string, not capture.

    // time_t overload
    auto s1 = util::fmt_time( chrono::seconds( time( NULL ) ) );
    REQUIRE_THAT( s1, Matches( R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})" ) );

    // chrono::system_clock::time_point overload
    auto l = chrono::system_clock::now();
    auto t = util::fmt_time( l );
    REQUIRE_THAT( t, Matches( R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9})" ) );

    // ZonedTimePoint overload
    auto z = ZonedTimePoint( l, util::tz_utc() );
    t = util::fmt_time( z );
    REQUIRE_THAT( t, Matches(
        R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9}[-+]\d{4})" ) );
    t = util::fmt_time( z, util::tz_utc() );
    REQUIRE_THAT( t, Matches(
        R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9}\+0000)" ) );

    auto hhmm = util::tz_hhmm();
    REQUIRE( hhmm.size() == 5 );
    auto hhmm_utc = util::tz_hhmm( util::tz_utc() );
    REQUIRE( hhmm_utc == "+0000" );
}

TEST_CASE( "opt_util" )
{
    vector<optional<string>> v{{"5"}, nullopt, {"7"}, {"9"}, nullopt, {"0"}, {"1"}};

    auto res = util::cat_opts( v );
    REQUIRE( res == vector<string>{ "5", "7", "9", "0", "1" } );

    // Do it again to make sure v wasn't moved from.
    auto res2 = util::cat_opts( v );
    REQUIRE( res2 == vector<string>{ "5", "7", "9", "0", "1" } );

    // Now do the moved from version.
    auto res3 = util::cat_opts( move( v ) );
    REQUIRE( res3 == vector<string>{ "5", "7", "9", "0", "1" } );

    // Make sure it was moved from.
    REQUIRE( v == vector<optional<string>>{
                    {""}, nullopt, {""}, {""}, nullopt, {""}, {""}});
}

TEST_CASE( "directed_graph" )
{
    using DG = util::DirectedGraph<fs::path>;

    unordered_map<fs::path, vector<fs::path>> m{
        { "B", { "B"      } },
        { "F", { "C", "B" } },
        { "C", { "D", "E" } },
        { "D", { "E"      } },
        { "E", { "F"      } },
        { "A", { "G"      } },
        { "G", { "C"      } },
        { "H", { "C", "D" } }
    };

    DG g = util::make_graph<fs::path>( m );

    vector<fs::path> v;

    v = g.accessible( "A" );
    sort( begin( v ), end( v ) );
    REQUIRE( v == (vector<fs::path>{ "A","B","C","D","E","F","G" }) );

    v = g.accessible( "E" );
    sort( begin( v ), end( v ) );
    REQUIRE( v == (vector<fs::path>{ "B", "C", "D", "E", "F" }) );

    v = g.accessible( "H" );
    sort( begin( v ), end( v ) );
    REQUIRE( v == (vector<fs::path>{ "B","C","D","E","F","H" }) );

    v = g.accessible( "G" );
    sort( begin( v ), end( v ) );
    REQUIRE( v == (vector<fs::path>{ "B", "C", "D", "E", "F", "G" }) );
}

TEST_CASE( "bimap" )
{
    using BM = util::BDIndexMap<fs::path>;

    BM bm0( {} );
    REQUIRE( bm0.size() == size_t( 0 ) );

    auto data = vector<fs::path>{
        "A/B/C/D/E",
        "A",
        "A",
        "A",
        "A/B/C",
        "A/B",
        "A",
        "A/B/C/D",
        "A",
        "",
        "ABBB",
        "AAAA",
    };

    BM bm( move( data ) );

    REQUIRE( bm.size() == size_t( 8 ) );

    fs::path s;

    for( size_t i = 0; i < bm.size(); ++i )
        { REQUIRE( bm.val_safe( i ) ); }

    s = *bm.val_safe( 0 ); REQUIRE( s == ""          );
    s = *bm.val_safe( 1 ); REQUIRE( s == "A"         );
    s = *bm.val_safe( 2 ); REQUIRE( s == "A/B"       );
    s = *bm.val_safe( 3 ); REQUIRE( s == "A/B/C"     );
    s = *bm.val_safe( 4 ); REQUIRE( s == "A/B/C/D"   );
    s = *bm.val_safe( 5 ); REQUIRE( s == "A/B/C/D/E" );
    s = *bm.val_safe( 6 ); REQUIRE( s == "AAAA"      );
    s = *bm.val_safe( 7 ); REQUIRE( s == "ABBB"      );

    REQUIRE( !(bm.val_safe( 8 )) );
    REQUIRE( !(bm.val_safe( 8000 )) );

    REQUIRE( bm.key_safe( ""          ) == 0 );
    REQUIRE( bm.key_safe( "A"         ) == 1 );
    REQUIRE( bm.key_safe( "A/B"       ) == 2 );
    REQUIRE( bm.key_safe( "A/B/C"     ) == 3 );
    REQUIRE( bm.key_safe( "A/B/C/D"   ) == 4 );
    REQUIRE( bm.key_safe( "A/B/C/D/E" ) == 5 );
    REQUIRE( bm.key_safe( "AAAA"      ) == 6 );
    REQUIRE( bm.key_safe( "ABBB"      ) == 7 );

    REQUIRE( bm.key_safe( "XXXX" ) == nullopt );
    REQUIRE( bm.key_safe( "AAA"  ) == nullopt );

    /******************************************************/
    // Now test BiMapFixed.

    // First empty map.
    vector<tuple<string, int>> v0{};
    util::BiMapFixed bmf0( move( v0 ) );
    REQUIRE( bmf0.size() == 0 );
    REQUIRE( !bmf0.val_safe( "xxx" ) );
    REQUIRE( !bmf0.key_safe( 1     ) );

    // Now a large map.
    vector<tuple<string, int>> v1{
        { "abc",     9    },
        { "def",     2    },
        { "yyy",     3000 },
        { "ab",      8    },
        { "xxx",     2000 },
        { "d",       3    },
        { "hello",   7    },
        { "one",     4    },
        { "two",     6    },
        { "three",   5    },
        { "33",      33   },
        { "98",      98   },
        { "",        101  }
    };

    // This will move from the vector!
    util::BiMapFixed bmf1( move( v1 ) );

    REQUIRE( bmf1.size() == 13 );
    auto r1 = bmf1.val_safe( "xxx" );
    REQUIRE( r1 ); REQUIRE( *r1 == 2000 );
    auto r2 = bmf1.val_safe( "three" );
    REQUIRE( r2 ); REQUIRE( *r2 == 5 );
    auto r3 = bmf1.val_safe( "aaa" );
    REQUIRE( !r3 );
    auto r4 = bmf1.val_safe( "" );
    REQUIRE( r4 ); REQUIRE( *r4 == 101 );

    auto r5 = bmf1.key_safe( 101 );
    REQUIRE( r5 ); REQUIRE( (*r5).get() == ""s );
    auto r6 = bmf1.key_safe( 3000 );
    REQUIRE( r6 ); REQUIRE( (*r6).get() == "yyy"s );
    auto r7 = bmf1.key_safe( 6 );
    REQUIRE( r7 ); REQUIRE( (*r7).get() == "two"s );
    auto r8 = bmf1.key_safe( 3001 );
    REQUIRE( !r8 );

    REQUIRE_THROWS( bmf1.key(  102  ) );
    REQUIRE_THROWS( bmf1.val( "988" ) );

    REQUIRE( bmf1.key(  98   ) == "98"s );
    REQUIRE( bmf1.key(  3    ) == "d"s  );
    REQUIRE( bmf1.val( "98"  ) == 98 );
    REQUIRE( bmf1.val( "d"   ) == 3  );

    // test iterator interface.
    vector<tuple<string, int>> v2;
    for( auto const& [k,v] : bmf1 )
        v2.emplace_back( k, v );

    REQUIRE( v2.size() == 13 );
    REQUIRE( get<0>( v2[0]  ) == ""s    );
    REQUIRE( get<1>( v2[0]  ) == 101    );
    REQUIRE( get<0>( v2[2]  ) == "98"s  );
    REQUIRE( get<1>( v2[2]  ) == 98     );
    REQUIRE( get<0>( v2[12] ) == "yyy"s );
    REQUIRE( get<1>( v2[12] ) == 3000   );
}
