/****************************************************************
* Unit tests
****************************************************************/
#include "catch2/catch.hpp"

#include "base-util/algo.hpp"
#include "base-util/algo-par.hpp"
#include "base-util/string.hpp"

using namespace std;

TEST_CASE( "group_by_key" )
{
    using util::group_by_key;

    auto mult_of_3 = []( auto n ) { return n % 3 == 0; };

    auto v1 = vector<int>{};
    auto r1 = group_by_key( v1, mult_of_3 );
    REQUIRE( r1 == vector<size_t>{} );

    auto v2 = vector{ 2 };
    auto r2 = group_by_key( v2, mult_of_3 );
    REQUIRE( r2 == vector<size_t>{0} );

    auto v3 = vector{ 3 };
    auto r3 = group_by_key( v3, mult_of_3 );
    REQUIRE( r3 == vector<size_t>{0} );

    auto v4 = vector{ 9, 9 };
    auto r4 = group_by_key( v4, mult_of_3 );
    REQUIRE( r4 == vector<size_t>{0} );

    auto v5 = vector{ 4, 4 };
    auto r5 = group_by_key( v5, mult_of_3 );
    REQUIRE( r5 == vector<size_t>{0} );

    auto v6 = vector{ 4, 6 };
    auto r6 = group_by_key( v6, mult_of_3 );
    REQUIRE( r6 == vector<size_t>{0, 1} );

    auto v7 = vector{ 6, 4 };
    auto r7 = group_by_key( v7, mult_of_3 );
    REQUIRE( r7 == vector<size_t>{0, 1} );

    auto v8 = vector{
        7, 5, 2, 9, 12, 3, 3, 3, 1, 6, 7, 9, 8, 8, 8, 3, 3 };
    auto r8 = group_by_key( v8, mult_of_3 );
    REQUIRE( r8 == vector<size_t>{0, 3, 8, 9, 10, 11, 12, 15 } );

    auto is_short = []( auto s ){ return s.size() < 4; };
    auto v9 = vector<string>{ "hello", "world", "yes", "no" };
    auto r9 = group_by_key( v9, is_short );
    REQUIRE( r9 == vector<size_t>{0, 2} );
}

TEST_CASE( "remove_if" )
{
    auto v = vector{ 7, 6, 5, 4, 3, 2, 1 };
    auto even = L( _ % 2 == 0 );
    util::remove_if( v, even );
    REQUIRE( v == vector{ 7, 5, 3, 1 } );
}

TEST_CASE( "sorting" )
{
    vector<int> v{ 4, 7, 3, 6, 4, 7, 5, 2, 4, 5, 2 };
    auto v2 = v;

    util::sort( v );
    REQUIRE( v == vector{ 2, 2, 3, 4, 4, 4, 5, 5, 6, 7, 7 } );

    util::uniq_sort( v2 );
    REQUIRE( v2 == vector{ 2, 3, 4, 5, 6, 7 } );
}

TEST_CASE( "map" )
{
    vector<int> v0{ 9, 1, 8, 2, 7, 3, 6, 4, 5 };

    auto v0_res = util::map( L( _ + 2 ), v0 );

    REQUIRE( v0_res.size() == v0.size() );

    REQUIRE( v0_res[0] == 11 );
    REQUIRE( v0_res[1] == 3  );
    REQUIRE( v0_res[2] == 10 );
    REQUIRE( v0_res[3] == 4  );
    REQUIRE( v0_res[4] == 9  );
    REQUIRE( v0_res[5] == 5  );
    REQUIRE( v0_res[6] == 8  );
    REQUIRE( v0_res[7] == 6  );
    REQUIRE( v0_res[8] == 7  );

    struct S {
        int x;
        int y;
    };

    auto make_S = []( int n ){ return S{ n*2, n/2 }; };

    vector<int> v1{ 1, 2, 3, 4 };

    auto v1_res = util::map( make_S, v1 );

    REQUIRE( v1_res.size() == v1.size() );

    REQUIRE( v1_res[0].x == 2 );
    REQUIRE( v1_res[0].y == 0 );
    REQUIRE( v1_res[1].x == 4 );
    REQUIRE( v1_res[1].y == 1 );
    REQUIRE( v1_res[2].x == 6 );
    REQUIRE( v1_res[2].y == 1 );
    REQUIRE( v1_res[3].x == 8 );
    REQUIRE( v1_res[3].y == 2 );

    vector<int> v2{ 2, 3, 4 };
    int count = 0;

    auto mod = [&]( int x ){ count += x; };

    util::map_( mod, v2 );

    REQUIRE( count == 9 );
}

TEST_CASE( "algo" )
{
    auto find_n = []( int n, int test ) { return test < n; };

    vector<int> v1{};

    // Test with empty vector
    auto r1 = util::lower_bound( v1, LC( find_n( 5, _ ) ) );
    auto r2 = util::lower_bound( v1, LC( find_n( 0, _ ) ) );
    REQUIRE( r1 == end( v1 ) );
    REQUIRE( r2 == end( v1 ) );

    // Test with vector with one element.
    vector v2{ 4 };

    auto r3 = util::lower_bound( v2, LC( find_n( 5, _ ) ) );
    auto r4 = util::lower_bound( v2, LC( find_n( 3, _ ) ) );
    auto r5 = util::lower_bound( v2, LC( find_n( 4, _ ) ) );
    REQUIRE( r3 == end( v2 ) );
    REQUIRE( r4 != end( v2 ) ); REQUIRE( *r4 == 4 );
    REQUIRE( r5 != end( v2 ) ); REQUIRE( *r5 == 4 );

    // Test with vector with two elements.
    vector v3{ 4, 10 };
    auto r6  = util::lower_bound( v3, LC( find_n( 0,  _ ) ) );
    auto r7  = util::lower_bound( v3, LC( find_n( 3,  _ ) ) );
    auto r8  = util::lower_bound( v3, LC( find_n( 4,  _ ) ) );
    auto r9  = util::lower_bound( v3, LC( find_n( 5,  _ ) ) );
    auto r10 = util::lower_bound( v3, LC( find_n( 9,  _ ) ) );
    auto r11 = util::lower_bound( v3, LC( find_n( 10, _ ) ) );
    auto r12 = util::lower_bound( v3, LC( find_n( 11, _ ) ) );
    auto r13 = util::lower_bound( v3, LC( find_n( 12, _ ) ) );
    REQUIRE( r6  != end( v3 ) ); REQUIRE( *r6 ==  4  );
    REQUIRE( r7  != end( v3 ) ); REQUIRE( *r7 ==  4  );
    REQUIRE( r8  != end( v3 ) ); REQUIRE( *r8 ==  4  );
    REQUIRE( r9  != end( v3 ) ); REQUIRE( *r9 ==  10 );
    REQUIRE( r10 != end( v3 ) ); REQUIRE( *r10 == 10 );
    REQUIRE( r11 != end( v3 ) ); REQUIRE( *r11 == 10 );
    REQUIRE( r12 == end( v3 ) );
    REQUIRE( r13 == end( v3 ) );

    // Test with vector with many elements.
    vector v4{ 0, 4, 7, 9, 55, 102, 103, 104, 200 };

    auto r14 = util::lower_bound( v4, LC( find_n( -1,    _ ) ) );
    auto r15 = util::lower_bound( v4, LC( find_n(  0,    _ ) ) );
    auto r16 = util::lower_bound( v4, LC( find_n(  4,    _ ) ) );
    auto r17 = util::lower_bound( v4, LC( find_n(  5,    _ ) ) );
    auto r18 = util::lower_bound( v4, LC( find_n(  101,  _ ) ) );
    auto r19 = util::lower_bound( v4, LC( find_n(  102,  _ ) ) );
    auto r20 = util::lower_bound( v4, LC( find_n(  103,  _ ) ) );
    auto r21 = util::lower_bound( v4, LC( find_n(  104,  _ ) ) );
    auto r22 = util::lower_bound( v4, LC( find_n(  105,  _ ) ) );
    auto r23 = util::lower_bound( v4, LC( find_n(  106,  _ ) ) );
    auto r24 = util::lower_bound( v4, LC( find_n(  200,  _ ) ) );
    auto r25 = util::lower_bound( v4, LC( find_n(  220,  _ ) ) );

    REQUIRE( r14 != end( v4 ) ); REQUIRE( *r14 == 0   );
    REQUIRE( r15 != end( v4 ) ); REQUIRE( *r15 == 0   );
    REQUIRE( r16 != end( v4 ) ); REQUIRE( *r16 == 4   );
    REQUIRE( r17 != end( v4 ) ); REQUIRE( *r17 == 7   );
    REQUIRE( r18 != end( v4 ) ); REQUIRE( *r18 == 102 );
    REQUIRE( r19 != end( v4 ) ); REQUIRE( *r19 == 102 );
    REQUIRE( r20 != end( v4 ) ); REQUIRE( *r20 == 103 );
    REQUIRE( r21 != end( v4 ) ); REQUIRE( *r21 == 104 );
    REQUIRE( r22 != end( v4 ) ); REQUIRE( *r22 == 200 );
    REQUIRE( r23 != end( v4 ) ); REQUIRE( *r23 == 200 );
    REQUIRE( r24 != end( v4 ) ); REQUIRE( *r24 == 200 );
    REQUIRE( r25 == end( v4 ) );
}

TEST_CASE( "chunking" )
{
    using PType = PairVec<size_t, size_t>;

    REQUIRE_THROWS( util::chunks( 1, 0 ) );

    REQUIRE( util::chunks( 0, 0 ) ==
             (PType{}) );

    REQUIRE( util::chunks( 0, 1 ) ==
             (PType{}) );
    REQUIRE( util::chunks( 0, 3 ) ==
             (PType{}) );

    REQUIRE( util::chunks( 1, 1 ) ==
             (PType{ {0,1} }) );
    REQUIRE( util::chunks( 2, 1 ) ==
             (PType{ {0,1},{1,2} }) );
    REQUIRE( util::chunks( 3, 1 ) ==
             (PType{ {0,1},{1,2},{2,3} }) );

    REQUIRE( util::chunks( 10, 1 ) ==
             (PType{ {0,1},{1,2},{2,3},{3,4},{4,5},
                     {5,6},{6,7},{7,8},{8,9},{9,10} }) );
    REQUIRE( util::chunks( 10, 2 ) ==
             (PType{ {0,2},{2,4},{4,6},{6,8},{8,10} }) );
    REQUIRE( util::chunks( 10, 3 ) ==
             (PType{ {0,3},{3,6},{6,9},{9,10} }) );
    REQUIRE( util::chunks( 10, 4 ) ==
             (PType{ {0,4},{4,8},{8,10} }) );
    REQUIRE( util::chunks( 10, 5 ) ==
             (PType{ {0,5},{5,10} }) );
    REQUIRE( util::chunks( 10, 6 ) ==
             (PType{ {0,6},{6,10} }) );
    REQUIRE( util::chunks( 10, 7 ) ==
             (PType{ {0,7},{7,10} }) );
    REQUIRE( util::chunks( 10, 8 ) ==
             (PType{ {0,8},{8,10} }) );
    REQUIRE( util::chunks( 10, 9 ) ==
             (PType{ {0,9},{9,10} }) );
    REQUIRE( util::chunks( 10, 10 ) ==
             (PType{ {0,10} }) );
    REQUIRE( util::chunks( 10, 11 ) ==
             (PType{ {0,10} }) );
    REQUIRE( util::chunks( 10, 20 ) ==
             (PType{ {0,10} }) );
}

TEST_CASE( "for_each_par" )
{
    vector<int> outputs{ 1, 2, 3, 4 };
    auto inc = [&outputs]( int index ){
        outputs[index]++;
    };

    util::par::for_each( vector<int>{ 0, 1, 2, 3 }, inc );
    REQUIRE( outputs == (vector<int>{ 2, 3, 4, 5 }) );

    util::par::for_each( vector<int>{ 1, 3 }, inc, 1 );
    REQUIRE( outputs == (vector<int>{ 2, 4, 4, 6 }) );

    util::par::for_each( vector<int>{ 1, 3 }, inc, 0 );
    REQUIRE( outputs == (vector<int>{ 2, 5, 4, 7 }) );

    util::par::for_each( vector<int>{ 1, 2, 3 }, inc );
    REQUIRE( outputs == (vector<int>{ 2, 6, 5, 8 }) );
}

TEST_CASE( "map_par" )
{
    // In this test, when creating vectors of Result's, can't use
    // initializer list directly because  for  some  reason  that
    // will require the Result variant to have  a  copy  construc-
    // tor, which  it  won't  because  the  Error  type  does not.

    auto inc = []( int x ){
        return fs::path( std::to_string( x+1 ) );
    };

    vector<int> v1;
    auto res_v1 = util::par::map( inc, v1 );
    REQUIRE( res_v1 == vector<fs::path>{} );

    vector<int> v2{ 3 };
    auto res_v2 = util::par::map( inc, v2 );
    REQUIRE( res_v2 == vector<fs::path>{ "4" } );

    vector<int> v3{ 5, 4, 3, 2, 1 };
    auto res_v3 = util::par::map( inc, v3 );
    REQUIRE( res_v3 == (vector<fs::path>{ "6","5","4","3","2" }) );

    // First with one job, then with  two  jobs,  then  max  jobs.
    vector<int> v4;
    vector<fs::path> goal4;
    for( int i = 0; i < 1000; ++i ) {
        v4.push_back( i );
        goal4.emplace_back( to_string( i+1 ) );
    }
    auto res_v4 = util::par::map( inc, v4, 1 );
    REQUIRE( res_v4 == goal4 );
    auto res_v5 = util::par::map( inc, v4, 2 );
    REQUIRE( res_v5 == goal4 );
    auto res_v6 = util::par::map( inc, v4, 0 );
    REQUIRE( res_v6 == goal4 );

    // Now test error reporting.
    vector<int> v7{ 5, 4, 3, 2, 1 };
    auto inc_err = []( int x ){
        ASSERT_( x != 3 );
        return fs::path( to_string( x+1 ) );
    };
    REQUIRE_THROWS( util::par::map( inc_err, v7, 0 ) );
}

TEST_CASE( "map_par_safe" )
{
    // In this test, when creating vectors of Result's, can't use
    // initializer list directly because  for  some  reason  that
    // will require the Result variant to have  a  copy  construc-
    // tor, which  it  won't  because  the  Error  type  does not.

    auto inc = []( int x ){
        return fs::path( std::to_string( x+1 ) );
    };

    vector<int> v1;
    auto res_v1 = util::par::map_safe( inc, v1 );
    vector<util::Result<fs::path>> goal1;
    REQUIRE( res_v1 == goal1 );

    vector<int> v2{ 3 };
    auto res_v2 = util::par::map_safe( inc, v2 );
    vector<util::Result<fs::path>> goal2;
    goal2.emplace_back( fs::path( "4" ) );
    REQUIRE( res_v2 == goal2 );

    vector<int> v3{ 5, 4, 3, 2, 1 };
    auto res_v3 = util::par::map_safe( inc, v3 );
    vector<util::Result<fs::path>> goal3;
    for( auto p : { "6","5","4","3","2" } )
        goal3.emplace_back( fs::path( p ) );
    REQUIRE( res_v3 == goal3 );

    // First with one job, then with  two  jobs,  then  max  jobs.
    vector<int> v4;
    vector<util::Result<fs::path>> goal4;
    for( int i = 0; i < 1000; ++i ) {
        v4.push_back( i );
        goal4.emplace_back( fs::path( to_string( i+1 ) ) );
    }
    auto res_v4 = util::par::map_safe( inc, v4, 1 );
    REQUIRE( res_v4 == goal4 );
    auto res_v5 = util::par::map_safe( inc, v4, 2 );
    REQUIRE( res_v5 == goal4 );
    auto res_v6 = util::par::map_safe( inc, v4, 0 );
    REQUIRE( res_v6 == goal4 );

    // Now test error reporting.
    vector<int> v7{ 5, 4, 3, 2, 1 };
    auto inc_err = []( int x ){
        ASSERT_( x != 3 );
        return fs::path( to_string( x+1 ) );
    };
    auto res_v7 = util::par::map_safe( inc_err, v7, 0 );
    REQUIRE( res_v7.size() == 5 );
    REQUIRE( res_v7[0] == util::Result<fs::path>( "6" ) );
    REQUIRE( res_v7[1] == util::Result<fs::path>( "5" ) );
    REQUIRE( holds_alternative<util::Error>( res_v7[2] ) );
    REQUIRE( util::contains(
                get<util::Error>( res_v7[2] ).msg, "error" ) );
    REQUIRE( res_v7[3] == util::Result<fs::path>( "3" ) );
    REQUIRE( res_v7[4] == util::Result<fs::path>( "2" ) );
}
