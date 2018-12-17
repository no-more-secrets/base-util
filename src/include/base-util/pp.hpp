/****************************************************************
* Preprocessor Metaprogramming
****************************************************************/
#pragma once

// Using EVAL<n>( ... ) will result in 3^n evals.
#define EVAL( ... ) EVAL9( __VA_ARGS__ )

#define EVAL9( ... ) EVAL8( EVAL8( EVAL8 ( __VA_ARGS__ ) ) )
#define EVAL8( ... ) EVAL7( EVAL7( EVAL7 ( __VA_ARGS__ ) ) )
#define EVAL7( ... ) EVAL6( EVAL6( EVAL6 ( __VA_ARGS__ ) ) )
#define EVAL6( ... ) EVAL5( EVAL5( EVAL5 ( __VA_ARGS__ ) ) )
#define EVAL5( ... ) EVAL4( EVAL4( EVAL4 ( __VA_ARGS__ ) ) )
#define EVAL4( ... ) EVAL3( EVAL3( EVAL3 ( __VA_ARGS__ ) ) )
#define EVAL3( ... ) EVAL2( EVAL2( EVAL2 ( __VA_ARGS__ ) ) )
#define EVAL2( ... ) EVAL1( EVAL1( EVAL1 ( __VA_ARGS__ ) ) )
#define EVAL1( ... ) EVAL0( EVAL0( EVAL0 ( __VA_ARGS__ ) ) )
#define EVAL0( ... ) __VA_ARGS__

#define EMPTY()

#define HEAD( a, ... ) a
#define TAIL( a, ... ) __VA_ARGS__

// This macro requires --std=c++2a and also to disable the a
// warning that gets triggered when we pass zero arguments to
// the __VA_ARGS__ of a variadic macro:
//
//   -std=c++2a  OR  -std=c++20
//   -Wno-gnu-zero-variadic-macro-arguments
//
#define PP_MAP( f, ... ) __VA_OPT__(                      \
    f( HEAD( __VA_ARGS__ ) );                             \
    PP_MAP_INDIRECT EMPTY() () ( f, TAIL( __VA_ARGS__ ) ) \
)

#define PP_MAP_INDIRECT() PP_MAP
