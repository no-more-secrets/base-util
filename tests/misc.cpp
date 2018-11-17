/****************************************************************
* Unit tests
****************************************************************/
#include "common.hpp"

#include "base-util/algo.hpp"
#include "base-util/graph.hpp"
#include "base-util/io.hpp"
#include "base-util/logger.hpp"
#include "base-util/opt-util.hpp"
#include "base-util/string-util.hpp"

#include <type_traits>

using namespace std;
using namespace std::string_literals;

namespace testing {

// Compile-time test that we can log IO manipulators such as endl.
using io_manip_valid = decltype( operator<<( util::log, endl ) );

TEST( datetime )
{
    // Parenthesis in regex's are for raw string, not capture.

    // time_t overload
    auto s1 = util::fmt_time( chrono::seconds( time( NULL ) ) );
    MATCHES( s1, R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})" );

    // chrono::system_clock::time_point overload
    auto l = chrono::system_clock::now();
    auto t = util::fmt_time( l );
    MATCHES( t, R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9})" );

    // ZonedTimePoint overload
    auto z = ZonedTimePoint( l, util::tz_utc() );
    t = util::fmt_time( z );
    MATCHES( t,
        R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9}[-+]\d{4})" );
    t = util::fmt_time( z, util::tz_utc() );
    MATCHES( t,
        R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{9}\+0000)" );

    auto hhmm = util::tz_hhmm();
    EQUALS( hhmm.size(), 5 );
    auto hhmm_utc = util::tz_hhmm( util::tz_utc() );
    EQUALS( hhmm_utc, "+0000" );
}

TEST( opt_util )
{
    vector<optional<int>> v{
        { 5 }, nullopt, { 7 }, { 9 }, nullopt, { 0 }, { 1 } };

    auto res = util::cat_opts( v );
    EQUALS( res, (vector<int>{ 5, 7, 9, 0, 1 }) );
}

TEST( directed_graph )
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
    EQUALS( v, (vector<fs::path>{ "A","B","C","D","E","F","G" }) );

    v = g.accessible( "E" );
    sort( begin( v ), end( v ) );
    EQUALS( v, (vector<fs::path>{ "B", "C", "D", "E", "F" }) );

    v = g.accessible( "H" );
    sort( begin( v ), end( v ) );
    EQUALS( v, (vector<fs::path>{ "B","C","D","E","F","H" }) );

    v = g.accessible( "G" );
    sort( begin( v ), end( v ) );
    EQUALS( v, (vector<fs::path>{ "B", "C", "D", "E", "F", "G" }) );
}

TEST( bimap )
{
    using BM = util::BDIndexMap<fs::path>;

    BM bm0( {} );
    EQUALS( bm0.size(), size_t( 0 ) );

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

    EQUALS( bm.size(), size_t( 8 ) );

    fs::path s;

    for( size_t i = 0; i < bm.size(); ++i )
        { TRUE_( bm.val_safe( i ) ); }

    s = *bm.val_safe( 0 ); EQUALS( s, ""          );
    s = *bm.val_safe( 1 ); EQUALS( s, "A"         );
    s = *bm.val_safe( 2 ); EQUALS( s, "A/B"       );
    s = *bm.val_safe( 3 ); EQUALS( s, "A/B/C"     );
    s = *bm.val_safe( 4 ); EQUALS( s, "A/B/C/D"   );
    s = *bm.val_safe( 5 ); EQUALS( s, "A/B/C/D/E" );
    s = *bm.val_safe( 6 ); EQUALS( s, "AAAA"      );
    s = *bm.val_safe( 7 ); EQUALS( s, "ABBB"      );

    TRUE_( !(bm.val_safe( 8 )) );
    TRUE_( !(bm.val_safe( 8000 )) );

    EQUALS( bm.key_safe( ""          ), 0 );
    EQUALS( bm.key_safe( "A"         ), 1 );
    EQUALS( bm.key_safe( "A/B"       ), 2 );
    EQUALS( bm.key_safe( "A/B/C"     ), 3 );
    EQUALS( bm.key_safe( "A/B/C/D"   ), 4 );
    EQUALS( bm.key_safe( "A/B/C/D/E" ), 5 );
    EQUALS( bm.key_safe( "AAAA"      ), 6 );
    EQUALS( bm.key_safe( "ABBB"      ), 7 );

    EQUALS( bm.key_safe( "XXXX" ), nullopt );
    EQUALS( bm.key_safe( "AAA"  ), nullopt );

    /******************************************************/
    // Now test BiMapFixed.

    // First empty map.
    vector<tuple<string, int>> v0{};
    util::BiMapFixed bmf0( move( v0 ) );
    EQUALS( bmf0.size(), 0 );
    TRUE_( !bmf0.val_safe( "xxx" ) );
    TRUE_( !bmf0.key_safe( 1     ) );

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

    EQUALS( bmf1.size(), 13 );
    auto r1 = bmf1.val_safe( "xxx" );
    TRUE_( r1 ); EQUALS( *r1, 2000 );
    auto r2 = bmf1.val_safe( "three" );
    TRUE_( r2 ); EQUALS( *r2, 5 );
    auto r3 = bmf1.val_safe( "aaa" );
    TRUE_( !r3 );
    auto r4 = bmf1.val_safe( "" );
    TRUE_( r4 ); EQUALS( *r4, 101 );

    auto r5 = bmf1.key_safe( 101 );
    TRUE_( r5 ); EQUALS( (*r5).get(), ""s );
    auto r6 = bmf1.key_safe( 3000 );
    TRUE_( r6 ); EQUALS( (*r6).get(), "yyy"s );
    auto r7 = bmf1.key_safe( 6 );
    TRUE_( r7 ); EQUALS( (*r7).get(), "two"s );
    auto r8 = bmf1.key_safe( 3001 );
    TRUE_( !r8 );

    THROWS( bmf1.key(  102  ) );
    THROWS( bmf1.val( "988" ) );

    EQUALS( bmf1.key(  98   ), "98"s );
    EQUALS( bmf1.key(  3    ), "d"s  );
    EQUALS( bmf1.val( "98"  ), 98 );
    EQUALS( bmf1.val( "d"   ), 3  );

    // test iterator interface.
    vector<tuple<string, int>> v2;
    for( auto const& [k,v] : bmf1 )
        v2.emplace_back( k, v );

    EQUALS( v2.size(), 13 );
    EQUALS( get<0>( v2[0]  ), ""s    );
    EQUALS( get<1>( v2[0]  ), 101             );
    EQUALS( get<0>( v2[2]  ), "98"s  );
    EQUALS( get<1>( v2[2]  ), 98              );
    EQUALS( get<0>( v2[12] ), "yyy"s );
    EQUALS( get<1>( v2[12] ), 3000            );
}

} // namespace testing
