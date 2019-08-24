/****************************************************************
* Unit tests
****************************************************************/
#include "catch2/catch.hpp"

#include "base-util/graph.hpp"
#include "base-util/io.hpp"
#include "base-util/logger.hpp"
#include "base-util/optional.hpp"
#include "base-util/string.hpp"
#include "base-util/type-map.hpp"
#include "base-util/variant.hpp"

#include <optional>
#include <type_traits>

using namespace std;
using namespace std::string_literals;

using ::Catch::Matches; // regex matcher

// Compile-time test that we can log IO manipulators such as endl.
using io_manip_valid = decltype( operator<<( util::log, endl ) );

// This test is purely compile time so doesn't need to be in a
// test case, but we put it in one anyway.
TEST_CASE( "type-map" )
{
  using M = TypeMap<
    Pair<int,  float>,
    Pair<void, const double>,
    Pair<char, NoConstruct>
  >;

  static_assert( std::is_same_v<Get<M, void>, const double> );
  static_assert( std::is_same_v<Get<M, char>, NoConstruct> );
  static_assert( std::is_same_v<Get<M, int>,  float> );

  // With default value
  static_assert( std::is_same_v<Get<M, void,  int>, const double> );
  static_assert( std::is_same_v<Get<M, char*, int>, int> );
}

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

TEST_CASE( "variant" )
{
    variant<int, string> v1{5};
    variant<int, string> v2{"hello"};

    REQUIRE( util::holds( v1, 5 ) );
    REQUIRE( !util::holds( v1, 6 ) );
    REQUIRE( !util::holds( v1, string( "world" ) ) );

    bool is_int = false;
    GET_IF( v2, int, p ) {
        is_int = true;
    }
    bool is_string = false;
    GET_IF( v2, string, p ) {
        REQUIRE( *p == "hello" );
        is_string = true;
    }

    REQUIRE( !is_int );
    REQUIRE( is_string );

    using V = std::variant<int, double, std::string>;

    V v3 = 5.5;
    bool found3 = false;
    switch_v( v3 ) {
      case_v( int ) {
        found3 = false;
      }
      case_v( double ) {
        found3 = true;
      }
      case_v( std::string ) {
        found3 = false;
      }
      switch_exhaustive;
    }
    REQUIRE( found3 );

    V v4 = "hello";
    bool found4 = false;
    switch_v( v4 ) {
      case_v( int ) {
        found4= false;
      }
      case_v( double ) {
        found4= false;
      }
      case_v( std::string ) {
        found4= true;
      }
      switch_exhaustive;
    }
    REQUIRE( found4 );

    auto ret = matcher_v( v4 ) {
      case_v( int ) {
        result_v 6;
      }
      case_v( double ) {
        result_v 6;
      }
      case_v( std::string ) {
        result_v 7;
      }
      matcher_exhaustive;
    }
    REQUIRE( ret == 7 );

    // Test return type hints.
    auto ret2 = matcher_v( v4, ->, std::optional<int> ) {
      case_v( int ) {
        result_v nullopt;
      }
      case_v( double ) {
        result_v {};
      }
      case_v( std::string ) {
        result_v 7;
      }
      matcher_exhaustive;
    }
    REQUIRE( ret2 == 7 );

    // Test the structured bindings version.
    using A = pair<int, double>;
    using V2 = std::variant<int, A>;

    V2 v5 = A{3, 4.5};
    bool found5 = false;
    switch_v( v5 ) {
      case_v( int ) {
        found5 = false;
      }
      case_v( A, first, second ) {
        if( first == 3 && second == 4.5 )
            found5 = true;
      }
      switch_exhaustive;
    }
    REQUIRE( found5 );

    struct AA {
      int    x;
      string y;
    };

    struct BB {
      double d;
    };

    struct CC {
      int u;
      int v;
    };

    variant<AA, BB, CC, double> v6 = CC{3,4};

    // Test pattern matching with out-of-order params.
    auto ret3 = matcher_v( v6, ->, int ) {
      case_v( AA ) {
        //
        result_v 1;
      }
      case_v( BB, d ) {
        //
        result_v d;
      }
      case_v( CC, v, u ) {
        //
        (void)u;
        result_v v;
      }
      case_v( double ) {
        //
        result_v 10;
      }
      matcher_exhaustive;
    };
    REQUIRE( ret3 == 4 );

    struct Z {};

    auto tostring = variant_function( item, ->, std::string ) {
      case_v( int )    return std::to_string( item );
      case_v( string ) return item;
      case_v( Z )      return "Z";
      variant_function_exhaustive;
    };

    variant<int, string, Z> v7 = Z{};
    REQUIRE( tostring( v7 ) == "Z" );
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

    optional<int> o;
    auto to_str = []( int ) { return string("n"); };

    o = nullopt;
    REQUIRE( !util::fmap( to_str, o ).has_value() );

    o = 5;
    auto new_o = util::fmap( to_str, o );
    static_assert( is_same_v<decay_t<decltype(*new_o)>, string> );

    REQUIRE( new_o.has_value() );
    REQUIRE( new_o.value() == "n" );

    int x = 5;

    auto to_int_ref_nonconst = [&]( int ) -> int& { return x; };
    o = nullopt;
    auto new_o2 = util::fmap( to_int_ref_nonconst, o );
    REQUIRE( !new_o2.has_value() );
    static_assert( is_same_v<decltype(new_o2),
            std::optional<std::reference_wrapper<int>>> );
    o = 6;
    new_o2 = util::fmap( to_int_ref_nonconst, o );
    REQUIRE( new_o2.has_value() );
    REQUIRE( new_o2->get() == 5 );
    auto new_o3 = o | util::infix::fmap( to_int_ref_nonconst );
    static_assert( is_same_v<decltype(new_o3),
            std::optional<std::reference_wrapper<int>>> );
    REQUIRE( new_o3.has_value() );
    REQUIRE( new_o3->get() == 5 );

    auto to_int_ref_const = [&]( int ) -> int const& { return x; };
    o = nullopt;
    auto new_o4 = util::fmap( to_int_ref_const, o );
    REQUIRE( !new_o4.has_value() );
    static_assert( is_same_v<decltype(new_o4),
            std::optional<std::reference_wrapper<int const>>> );
    o = 6;
    new_o4 = util::fmap( to_int_ref_const, o );
    REQUIRE( new_o4.has_value() );
    REQUIRE( new_o4->get() == 5 );
    auto new_o5 = o | util::infix::fmap( to_int_ref_const );
    static_assert( is_same_v<decltype(new_o5),
            std::optional<std::reference_wrapper<int const>>> );
    REQUIRE( new_o5.has_value() );
    REQUIRE( new_o5->get() == 5 );

    auto to_maybe_int = []( int n ) -> std::optional<int> {
        if( n == 1 )
            return std::nullopt;
        else
            return std::optional<int>( 4 );
    };
    o = nullopt;
    auto maybe_new_o = util::fmap_join( to_maybe_int, o );
    static_assert( is_same_v<decltype(maybe_new_o), std::optional<int>> );
    REQUIRE( !maybe_new_o.has_value() );
    o = 1;
    maybe_new_o = util::fmap_join( to_maybe_int, o );
    REQUIRE( !maybe_new_o.has_value() );
    o = 2;
    maybe_new_o = util::fmap_join( to_maybe_int, o );
    REQUIRE( maybe_new_o.has_value() );
    REQUIRE( maybe_new_o == 4 );
}

TEST_CASE( "directed_graph" )
{
    // Test the `cyclic` function.
    unordered_map<fs::path, vector<fs::path>> c;

    c = {};
    REQUIRE( !util::make_graph( c ).cyclic() );

    c = {
      { "B", {} }
    };
    REQUIRE( !util::make_graph( c ).cyclic() );

    c = {
      { "B", { "B" } }
    };
    REQUIRE( util::make_graph( c ).cyclic() );

    c = {
      { "B", { "C" } },
      { "C", { "C" } }
    };
    REQUIRE( util::make_graph( c ).cyclic() );

    c = {
      { "B", { "C" } },
      { "C", { "B" } }
    };
    REQUIRE( util::make_graph( c ).cyclic() );

    c = {
      { "B", { "C" } },
      { "C", { "D" } },
      { "D", { "C","B" } }
    };
    REQUIRE( util::make_graph( c ).cyclic() );

    c = {
      { "B", { "D" } },
      { "C", { "D" } },
      { "D", {} }
    };
    REQUIRE( !util::make_graph( c ).cyclic() );

    c = {
      { "B", { "C" } },
      { "C", { "D" } },
      { "D", {} }
    };
    REQUIRE( !util::make_graph( c ).cyclic() );

    // Test the `accessible` function.
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

    REQUIRE( g.cyclic() );

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

    unordered_map<fs::path, vector<fs::path>> m2{
        { "B", { } },
        { "I", { } },
        { "F", { "B", "I" } },
        { "C", { "D", "E" } },
        { "D", { "E"      } },
        { "E", { "F", "I" } },
        { "A", { "G"      } },
        { "G", { "C"      } },
        { "H", { "C", "D" } }
    };

    auto g2 = util::DAG<fs::path>::make_dag( m2 );

    REQUIRE( !g2.cyclic() );

    // Test sorting
    v = g2.sorted();
    REQUIRE( v == (vector<fs::path>{"B","I","F","E","D","C","G","H","A"}) );

    unordered_map<fs::path, vector<fs::path>> m3{
        { "A", { "C", "D" } },
        { "B", { "C", "D" } },
        { "C", { "D", "E" } },
        { "D", { "E", "F" } },
        { "E", { "F", "G" } },
        { "F", { "G"      } },
        { "G", {} }
    };

    auto g3 = util::DAG<fs::path>::make_dag( m3 );

    REQUIRE( !g3.cyclic() );

    // Test sorting
    v = g3.sorted();
    REQUIRE( v == (vector<fs::path>{"G","F","E","D","C","B","A"}) );
}

TEST_CASE( "directed graph sort test 1" )
{
    vector<string> names{
        "configs",
        "rng",
        "sdl",
        "fonts",
        "app_window",
        "midiseq",
        "midiplayer",
        "conductor",
        "tunes",
        "screen",
        "renderer",
        "sprites",
        "planes",
        "sound",
        "images",
        "menus",
        "terrain"
    };

    unordered_map<string, vector<string>> m{
        {"configs", {}},
        {"rng",
        {
            "configs"
        }},
        {"sdl",
        {
            "configs" //
        }},
        {"fonts",
        {
            "configs", //
            "sdl"      //
        }},
        {"app_window",
        {
            "configs", //
            "sdl"      //
        }},
        {"screen",
        {
            "configs", //
            "sdl"      //
        }},
        {"renderer",
        {
            "configs",    //
            "app_window", //
            "screen"      //
        }},
        {"sprites",
        {
            "configs", //
            "sdl",     //
            "renderer" //
        }},
        {"planes",
        {
            "configs", //
            "sdl",     //
            "screen",  //
            "renderer" //
        }},
        {"sound",
        {
            "configs", //
            "sdl"      //
        }},
        {"images",
        {
            "configs", //
            "sdl"      //
        }},
        {"menus",
        {
            "configs",  //
            "sdl",      //
            "screen",   //
            "renderer", //
            "sprites",  //
            "fonts"     //
        }},
        {"terrain",
        {
            "configs",  //
            "renderer", //
            "sprites",  //
            "sdl"       //
        }},
        {"tunes",
        {
            "configs", //
            "rng",     //
        }},
        {"midiseq",
        {
            "configs", //
        }},
        {"midiplayer",
        {
            "midiseq", //
            "configs", //
        }},
        {"conductor",
        {
            "tunes",      //
            "midiplayer", //
            "configs",    //
        }}};

    vector<string> sorted_target{
        "configs",
        "midiseq",
        "sdl",
        "fonts",
        "images",
        "screen",
        "midiplayer",
        "rng",
        "app_window",
        "renderer",
        "tunes",
        "planes",
        "sprites",
        "sound",
        "menus",
        "terrain",
        "conductor",
    };
    auto g = util::DAG<string>::make_dag( m );

    REQUIRE( !g.cyclic() );

    // Test sorting
    auto v = g.sorted();
    REQUIRE( v == sorted_target );
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
