/****************************************************************
* Unit tests
****************************************************************/
#include "common.hpp"

#include "base-util/algo.hpp"
#include "base-util/algo-par.hpp"
#include "base-util/string.hpp"

using namespace std;

namespace testing {

TEST( map )
{
    vector<int> v0{ 9, 1, 8, 2, 7, 3, 6, 4, 5 };

    auto v0_res = util::map( L( _ + 2 ), v0 );

    EQUALS( v0_res.size(), v0.size() );

    EQUALS( v0_res[0], 11 );
    EQUALS( v0_res[1], 3  );
    EQUALS( v0_res[2], 10 );
    EQUALS( v0_res[3], 4  );
    EQUALS( v0_res[4], 9  );
    EQUALS( v0_res[5], 5  );
    EQUALS( v0_res[6], 8  );
    EQUALS( v0_res[7], 6  );
    EQUALS( v0_res[8], 7  );

    struct S {
        int x;
        int y;
    };

    auto make_S = []( int n ){ return S{ n*2, n/2 }; };

    vector<int> v1{ 1, 2, 3, 4 };

    auto v1_res = util::map( make_S, v1 );

    EQUALS( v1_res.size(), v1.size() );

    EQUALS( v1_res[0].x, 2 );
    EQUALS( v1_res[0].y, 0 );
    EQUALS( v1_res[1].x, 4 );
    EQUALS( v1_res[1].y, 1 );
    EQUALS( v1_res[2].x, 6 );
    EQUALS( v1_res[2].y, 1 );
    EQUALS( v1_res[3].x, 8 );
    EQUALS( v1_res[3].y, 2 );

    vector<int> v2{ 2, 3, 4 };
    int count = 0;

    auto mod = [&]( int x ){ count += x; };

    util::map_( mod, v2 );

    EQUALS( count, 9 );
}

TEST( algo )
{
    auto find_n = []( int n, int test ) { return test < n; };

    vector<int> v1{};

    // Test with empty vector
    auto r1 = util::lower_bound( v1, LC( find_n( 5, _ ) ) );
    auto r2 = util::lower_bound( v1, LC( find_n( 0, _ ) ) );
    TRUE_( r1 == end( v1 ) );
    TRUE_( r2 == end( v1 ) );

    // Test with vector with one element.
    vector v2{ 4 };

    auto r3 = util::lower_bound( v2, LC( find_n( 5, _ ) ) );
    auto r4 = util::lower_bound( v2, LC( find_n( 3, _ ) ) );
    auto r5 = util::lower_bound( v2, LC( find_n( 4, _ ) ) );
    TRUE_( r3 == end( v2 ) );
    TRUE_( r4 != end( v2 ) ); EQUALS( *r4, 4 );
    TRUE_( r5 != end( v2 ) ); EQUALS( *r5, 4 );

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
    TRUE_( r6  != end( v3 ) ); EQUALS( *r6,  4  );
    TRUE_( r7  != end( v3 ) ); EQUALS( *r7,  4  );
    TRUE_( r8  != end( v3 ) ); EQUALS( *r8,  4  );
    TRUE_( r9  != end( v3 ) ); EQUALS( *r9,  10 );
    TRUE_( r10 != end( v3 ) ); EQUALS( *r10, 10 );
    TRUE_( r11 != end( v3 ) ); EQUALS( *r11, 10 );
    TRUE_( r12 == end( v3 ) );
    TRUE_( r13 == end( v3 ) );

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

    TRUE_( r14 != end( v4 ) ); EQUALS( *r14, 0   );
    TRUE_( r15 != end( v4 ) ); EQUALS( *r15, 0   );
    TRUE_( r16 != end( v4 ) ); EQUALS( *r16, 4   );
    TRUE_( r17 != end( v4 ) ); EQUALS( *r17, 7   );
    TRUE_( r18 != end( v4 ) ); EQUALS( *r18, 102 );
    TRUE_( r19 != end( v4 ) ); EQUALS( *r19, 102 );
    TRUE_( r20 != end( v4 ) ); EQUALS( *r20, 103 );
    TRUE_( r21 != end( v4 ) ); EQUALS( *r21, 104 );
    TRUE_( r22 != end( v4 ) ); EQUALS( *r22, 200 );
    TRUE_( r23 != end( v4 ) ); EQUALS( *r23, 200 );
    TRUE_( r24 != end( v4 ) ); EQUALS( *r24, 200 );
    TRUE_( r25 == end( v4 ) );
}

TEST( chunking )
{
    using PType = PairVec<size_t, size_t>;

    THROWS( util::chunks( 1, 0 ) );

    EQUALS( util::chunks( 0, 0 ),
            (PType{}) );

    EQUALS( util::chunks( 0, 1 ),
            (PType{}) );
    EQUALS( util::chunks( 0, 3 ),
            (PType{}) );

    EQUALS( util::chunks( 1, 1 ),
            (PType{ {0,1} }) );
    EQUALS( util::chunks( 2, 1 ),
            (PType{ {0,1},{1,2} }) );
    EQUALS( util::chunks( 3, 1 ),
            (PType{ {0,1},{1,2},{2,3} }) );

    EQUALS( util::chunks( 10, 1 ),
            (PType{ {0,1},{1,2},{2,3},{3,4},{4,5},
                    {5,6},{6,7},{7,8},{8,9},{9,10} }) );
    EQUALS( util::chunks( 10, 2 ),
            (PType{ {0,2},{2,4},{4,6},{6,8},{8,10} }) );
    EQUALS( util::chunks( 10, 3 ),
            (PType{ {0,3},{3,6},{6,9},{9,10} }) );
    EQUALS( util::chunks( 10, 4 ),
            (PType{ {0,4},{4,8},{8,10} }) );
    EQUALS( util::chunks( 10, 5 ),
            (PType{ {0,5},{5,10} }) );
    EQUALS( util::chunks( 10, 6 ),
            (PType{ {0,6},{6,10} }) );
    EQUALS( util::chunks( 10, 7 ),
            (PType{ {0,7},{7,10} }) );
    EQUALS( util::chunks( 10, 8 ),
            (PType{ {0,8},{8,10} }) );
    EQUALS( util::chunks( 10, 9 ),
            (PType{ {0,9},{9,10} }) );
    EQUALS( util::chunks( 10, 10 ),
            (PType{ {0,10} }) );
    EQUALS( util::chunks( 10, 11 ),
            (PType{ {0,10} }) );
    EQUALS( util::chunks( 10, 20 ),
            (PType{ {0,10} }) );
}

TEST( for_each_par )
{
    vector<int> outputs{ 1, 2, 3, 4 };
    auto inc = [&outputs]( int index ){
        outputs[index]++;
    };

    util::par::for_each( vector<int>{ 0, 1, 2, 3 }, inc );
    EQUALS( outputs, (vector<int>{ 2, 3, 4, 5 }) );

    util::par::for_each( vector<int>{ 1, 3 }, inc, 1 );
    EQUALS( outputs, (vector<int>{ 2, 4, 4, 6 }) );

    util::par::for_each( vector<int>{ 1, 3 }, inc, 0 );
    EQUALS( outputs, (vector<int>{ 2, 5, 4, 7 }) );

    util::par::for_each( vector<int>{ 1, 2, 3 }, inc );
    EQUALS( outputs, (vector<int>{ 2, 6, 5, 8 }) );
}

TEST( map_par )
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
    EQUALS( res_v1, vector<fs::path>{} );

    vector<int> v2{ 3 };
    auto res_v2 = util::par::map( inc, v2 );
    EQUALS( res_v2, vector<fs::path>{ "4" } );

    vector<int> v3{ 5, 4, 3, 2, 1 };
    auto res_v3 = util::par::map( inc, v3 );
    EQUALS( res_v3, (vector<fs::path>{ "6","5","4","3","2" }) );

    // First with one job, then with  two  jobs,  then  max  jobs.
    vector<int> v4;
    vector<fs::path> goal4;
    for( int i = 0; i < 1000; ++i ) {
        v4.push_back( i );
        goal4.emplace_back( to_string( i+1 ) );
    }
    auto res_v4 = util::par::map( inc, v4, 1 );
    EQUALS( res_v4, goal4 );
    auto res_v5 = util::par::map( inc, v4, 2 );
    EQUALS( res_v5, goal4 );
    auto res_v6 = util::par::map( inc, v4, 0 );
    EQUALS( res_v6, goal4 );

    // Now test error reporting.
    vector<int> v7{ 5, 4, 3, 2, 1 };
    auto inc_err = []( int x ){
        ASSERT_( x != 3 );
        return fs::path( to_string( x+1 ) );
    };
    THROWS( util::par::map( inc_err, v7, 0 ) );
}

TEST( map_par_safe )
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
    EQUALS( res_v1, goal1 );

    vector<int> v2{ 3 };
    auto res_v2 = util::par::map_safe( inc, v2 );
    vector<util::Result<fs::path>> goal2;
    goal2.emplace_back( fs::path( "4" ) );
    EQUALS( res_v2, goal2 );

    vector<int> v3{ 5, 4, 3, 2, 1 };
    auto res_v3 = util::par::map_safe( inc, v3 );
    vector<util::Result<fs::path>> goal3;
    for( auto p : { "6","5","4","3","2" } )
        goal3.emplace_back( fs::path( p ) );
    EQUALS( res_v3, goal3 );

    // First with one job, then with  two  jobs,  then  max  jobs.
    vector<int> v4;
    vector<util::Result<fs::path>> goal4;
    for( int i = 0; i < 1000; ++i ) {
        v4.push_back( i );
        goal4.emplace_back( fs::path( to_string( i+1 ) ) );
    }
    auto res_v4 = util::par::map_safe( inc, v4, 1 );
    EQUALS( res_v4, goal4 );
    auto res_v5 = util::par::map_safe( inc, v4, 2 );
    EQUALS( res_v5, goal4 );
    auto res_v6 = util::par::map_safe( inc, v4, 0 );
    EQUALS( res_v6, goal4 );

    // Now test error reporting.
    vector<int> v7{ 5, 4, 3, 2, 1 };
    auto inc_err = []( int x ){
        ASSERT_( x != 3 );
        return fs::path( to_string( x+1 ) );
    };
    auto res_v7 = util::par::map_safe( inc_err, v7, 0 );
    EQUALS( res_v7.size(), 5 );
    EQUALS( res_v7[0], util::Result<fs::path>( "6" ) );
    EQUALS( res_v7[1], util::Result<fs::path>( "5" ) );
    TRUE_( holds_alternative<util::Error>( res_v7[2] ) );
    TRUE_( util::contains(
                get<util::Error>( res_v7[2] ).msg, "error" ) );
    EQUALS( res_v7[3], util::Result<fs::path>( "3" ) );
    EQUALS( res_v7[4], util::Result<fs::path>( "2" ) );
}

} // namespace testing
