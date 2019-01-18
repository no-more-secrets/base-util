/****************************************************************
**Preprocessor Metaprogramming
*
*   NOTE: -Wno-gnu-zero-variadic-macro-arguments may be needed
*         for this code to compile without warnings.
*****************************************************************/
#pragma once

// Using EVAL<n>( ... ) will result in 3^n evals.
#define EVAL( ... ) EVAL9( __VA_ARGS__ )

#define EVAL9( ... ) EVAL8( EVAL8( EVAL8( __VA_ARGS__ ) ) )
#define EVAL8( ... ) EVAL7( EVAL7( EVAL7( __VA_ARGS__ ) ) )
#define EVAL7( ... ) EVAL6( EVAL6( EVAL6( __VA_ARGS__ ) ) )
#define EVAL6( ... ) EVAL5( EVAL5( EVAL5( __VA_ARGS__ ) ) )
#define EVAL5( ... ) EVAL4( EVAL4( EVAL4( __VA_ARGS__ ) ) )
#define EVAL4( ... ) EVAL3( EVAL3( EVAL3( __VA_ARGS__ ) ) )
#define EVAL3( ... ) EVAL2( EVAL2( EVAL2( __VA_ARGS__ ) ) )
#define EVAL2( ... ) EVAL1( EVAL1( EVAL1( __VA_ARGS__ ) ) )
#define EVAL1( ... ) EVAL0( EVAL0( EVAL0( __VA_ARGS__ ) ) )
#define EVAL0( ... ) __VA_ARGS__

#define ID( ... ) __VA_ARGS__
#define EMPTY()

#define DEFER( ... ) __VA_ARGS__ EMPTY()

#define HEAD( a, ... ) a

#define TAIL( a, ... ) __VA_ARGS__

// JOIN_SEMIS will join the parameters but with a semicolon after
// each one (including the last).
#define JOIN_SEMIS( ... ) \
  __VA_OPT__( JOIN_SEMIS1( __VA_ARGS__ ) )

#define JOIN_SEMIS1( a, ... ) \
  a;                          \
  __VA_OPT__( JOIN_SEMIS1_INDIRECT EMPTY()()( __VA_ARGS__ ) )

#define JOIN_SEMIS1_INDIRECT() JOIN_SEMIS1

// PP_MAP_SEMI will map the function over the list and put a
// semicolon after each result value (including the last one).
//
// This one uses a head/tail approach to dealing with the edge
// cases (zeroand one-size lists).
#define PP_MAP_SEMI( ... ) \
  EVAL( PP_MAP_SEMI_RECURSE( __VA_ARGS__ ) )

#define PP_MAP_SEMI_RECURSE( f, ... )                 \
  __VA_OPT__( f( HEAD( __VA_ARGS__ ) );               \
              PP_MAP_SEMI_RECURSE_INDIRECT EMPTY()()( \
                  f, TAIL( __VA_ARGS__ ) ) )

#define PP_MAP_SEMI_RECURSE_INDIRECT() PP_MAP_SEMI_RECURSE

// PP_MAP_COMMAS will map the function over the list and put
// commas between the result values (but not after the last).
//
// This one uses a two-macro approach to dealing with the edge
// cases (zeroand one-size lists).
#define PP_MAP_COMMAS( ... ) \
  EVAL( PP_MAP_COMMAS_RECURSE( __VA_ARGS__ ) )

#define PP_MAP_COMMAS_RECURSE( f, ... ) \
  __VA_OPT__( PP_MAP_COMMAS1_RECURSE( f, __VA_ARGS__ ) )

#define PP_MAP_COMMAS1_RECURSE( f, a, ... )                   \
  f( a )                                                      \
      __VA_OPT__(, PP_MAP_COMMAS1_RECURSE_INDIRECT EMPTY()()( \
                       f, __VA_ARGS__ ) )

#define PP_MAP_COMMAS1_RECURSE_INDIRECT() PP_MAP_COMMAS1_RECURSE
