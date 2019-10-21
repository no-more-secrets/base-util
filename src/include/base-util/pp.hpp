/****************************************************************
**Preprocessor Metaprogramming
*
*   NOTE: -Wno-gnu-zero-variadic-macro-arguments may be needed
*         for this code to compile without warnings.
*****************************************************************/
#pragma once

/****************************************************************
** EVAL
*****************************************************************/
// Note about EVAL usage: by default, we should not put EVAL
// macros inside a macro definition that might be embedded in a
// parent macro as a building block. Instead, we should fully
// compose the macros (without EVAL) and then put one EVAL at the
// top level. This is why we do not put EVAL in the macros in
// this file. Another reason is that the macros in this file use
// recursion and this may not play nicely with the nested EVALs
// that would result. Instead, we compose the macros, then put an
// EVAL at the top level.
//
// However, that said, there are a few cases that seem to arise
// where it is necessary to put EVAL inside a macro, perhaps when
// embedding a complicated macro that is itself composed of sub
// macros. However, this should only be a last resort.

// Using EVAL<n>( ... ) will result in 3^n evals.
//
// WARNING: adding more stages to this will exponentially
// increase preprocessing time.
#define EVAL( ... ) EVAL4( __VA_ARGS__ )

#define EVAL4( ... ) EVAL3( EVAL3( EVAL3( __VA_ARGS__ ) ) )
#define EVAL3( ... ) EVAL2( EVAL2( EVAL2( __VA_ARGS__ ) ) )
#define EVAL2( ... ) EVAL1( EVAL1( EVAL1( __VA_ARGS__ ) ) )
#define EVAL1( ... ) EVAL0( EVAL0( EVAL0( __VA_ARGS__ ) ) )
#define EVAL0( ... ) __VA_ARGS__

/****************************************************************
** Miscellaneous
*****************************************************************/
#define PP_CONST_ONE( ... ) 1
#define PP_ADD_TYPENAME( ... ) typename __VA_ARGS__

/****************************************************************
** Evalutation Control
*****************************************************************/
#define EMPTY()
#define DEFER( ... ) __VA_ARGS__ EMPTY()
#define OBSTRUCT( ... ) __VA_ARGS__ DEFER( EMPTY )()

// This one can be useful when you would normally use a "##"
// joining operator. Sometimes that can cause evaluation order
// issues when inside a complicated macro, and this can solve it.
#define PP_JOIN( a, b ) a##b

/****************************************************************
** List Operations
*****************************************************************/
#define ID( ... ) __VA_ARGS__
#define EAT( ... ) __VA_ARGS__
#define NO_EAT( ... )

#define KEEP( ... ) __VA_ARGS__
#define NO_KEEP( ... )

// Take a list and determine if it is empty; use the first arg if
// it is not, and the second one if it is.
#define SWITCH_EMPTY( non_empty_case, empty_case, ... ) \
  __VA_OPT__( non_empty_case )                          \
  __VA_OPT__( NO_ )##KEEP( empty_case )

#define HEAD( a, ... ) a
#define HEAD_TUPLE( a ) HEAD a
#define TAIL( a, ... ) __VA_ARGS__

/****************************************************************
** Variadic Macro Utility
*****************************************************************/
// The purpose of these macros is to allow defining a macro that
// can take one or more arguments and where the first argument
// must be extracted. Macros actually don't allow this, since if
// one defines:
//
//   #define FOO( ... )
//
// Then one cannot extract the first argument; if one defines:
//
//   #define FOO( a, ... )
//
// Then it will give a compiler error when only a single argument
// is passed, because, for some unknown reason, there must always
// be at least one parameter passed for a "...".
//
// This is solved by the following macros.  First define these:
//
//   #define FOO_MULTI( s, ... ) printf( s, __VA_ARGS__ )
//   #define FOO_SINGLE( s )     printf( s )
//
// Now use `PP_ONE_OR_MORE_ARGS` to define a macro that will
// dispatch based on whether the macro is given one or
// more-than-one argument:
//
//   #define FOO( ... ) PP_ONE_OR_MORE_ARGS( FOO, __VA_ARGS__ )
//
// Now it can be used like this:
//
//   FOO( "test 1\n" );
//   FOO( "test 2: %d\n", 5 );
//   FOO( "test 3: %d, %f\n", 5, 6.6 );
//
// Note that FOO must still be given at least one argument.

// If the macro may be given more than ~10 parameters then this
// will have to be increased, along with `PP_HAS_MULTI_ARGS*`
// below.
#define PP_CHOOSE_TENTH_ARG( _1, _2, _3, _4, _5, _6, _7, _8, \
                             _9, _10, ... )                  \
  _10

#define PP_DISAMBIGUATE_MULTI_ARGS( f, has_args, ... ) \
  PP_JOIN( f##_, has_args )( __VA_ARGS__ )

#define PP_HAS_MULTI_ARGS_1( ... )                              \
  PP_CHOOSE_TENTH_ARG( __VA_ARGS__, MULTI, MULTI, MULTI, MULTI, \
                       MULTI, MULTI, MULTI, MULTI, SINGLE,      \
                       ERROR )

#define PP_HAS_MULTI_ARGS_2( a, ... )                           \
  PP_CHOOSE_TENTH_ARG( __VA_ARGS__, MULTI, MULTI, MULTI, MULTI, \
                       MULTI, MULTI, MULTI, MULTI, SINGLE,      \
                       ERROR )

#define PP_HAS_MULTI_ARGS_3( a, b, ... )                        \
  PP_CHOOSE_TENTH_ARG( __VA_ARGS__, MULTI, MULTI, MULTI, MULTI, \
                       MULTI, MULTI, MULTI, MULTI, SINGLE,      \
                       ERROR )

#define PP_ONE_OR_MORE_ARGS( f, ... ) \
  PP_DISAMBIGUATE_MULTI_ARGS(         \
      f, PP_HAS_MULTI_ARGS_1( __VA_ARGS__ ), __VA_ARGS__ )

#define PP_N_OR_MORE_ARGS_2( f, ... ) \
  PP_DISAMBIGUATE_MULTI_ARGS(         \
      f, PP_HAS_MULTI_ARGS_2( __VA_ARGS__ ), __VA_ARGS__ )

#define PP_N_OR_MORE_ARGS_3( f, ... ) \
  PP_DISAMBIGUATE_MULTI_ARGS(         \
      f, PP_HAS_MULTI_ARGS_3( __VA_ARGS__ ), __VA_ARGS__ )

/****************************************************************
** Tuple Operations
*****************************************************************/
#define EXPAND( ... ) __VA_ARGS__

#define PREPEND_TUPLE( what, tuple ) ( what, EXPAND tuple )
#define PREPEND_TUPLE2( what1, what2, tuple ) \
  ( what1, what2, EXPAND tuple )
#define PREPEND_TUPLE3( what1, what2, what3, tuple ) \
  ( what1, what2, what3, EXPAND tuple )

#define PAIR_TAKE_FIRST( a, b ) a
#define PAIR_TAKE_SECOND( a, b ) b

#define TUPLE_TAKE_FIRST( a, ... ) a

#define TUPLE_TAKE_SECOND_SINGLE( a, b ) b
#define TUPLE_TAKE_SECOND_MULTI( a, b, ... ) b
#define TUPLE_TAKE_SECOND( ... ) \
  PP_N_OR_MORE_ARGS_2( __VA_ARGS__ )
/****************************************************************
** PP_MAP_PREPEND_NS
*****************************************************************/
// PP_MAP_PREPEND_NS will prepend the given string to each
// element as a namespace.
#define PP_MAP_PREPEND_NS( name, ... ) \
  __VA_OPT__( PP_MAP_PREPEND_NS1( name, __VA_ARGS__ ) )

#define PP_MAP_PREPEND_NS1( name, a, ... )                    \
  name::a __VA_OPT__(, PP_MAP_PREPEND_NS1_INDIRECT EMPTY()()( \
                           name, __VA_ARGS__ ) )

#define PP_MAP_PREPEND_NS1_INDIRECT() PP_MAP_PREPEND_NS1

/****************************************************************
** PP_MAP_PREPEND_TUPLE
*****************************************************************/
// PP_MAP_PREPEND_TUPLE will prepend `what` to each element,
// which themselves must be tuples.
#define PP_MAP_PREPEND_TUPLE( what, ... ) \
  __VA_OPT__( PP_MAP_PREPEND_TUPLE1( what, __VA_ARGS__ ) )

#define PP_MAP_PREPEND_TUPLE1( what, a, ... )                \
  ( what, EXPAND a )                                         \
      __VA_OPT__(, PP_MAP_PREPEND_TUPLE1_INDIRECT EMPTY()()( \
                       what, __VA_ARGS__ ) )

#define PP_MAP_PREPEND_TUPLE1_INDIRECT() PP_MAP_PREPEND_TUPLE1

/****************************************************************
** PP_MAP_PREPEND2_TUPLE
*****************************************************************/
// PP_MAP_PREPEND2_TUPLE will prepend `what1` and `what2` to each
// element, which themselves must be tuples.
#define PP_MAP_PREPEND2_TUPLE( what1, what2, ... ) \
  __VA_OPT__(                                      \
      PP_MAP_PREPEND2_TUPLE1( what1, what2, __VA_ARGS__ ) )

#define PP_MAP_PREPEND2_TUPLE1( what1, what2, a, ... )        \
  ( what1, what2, EXPAND a )                                  \
      __VA_OPT__(, PP_MAP_PREPEND2_TUPLE1_INDIRECT EMPTY()()( \
                       what1, what2, __VA_ARGS__ ) )

#define PP_MAP_PREPEND2_TUPLE1_INDIRECT() PP_MAP_PREPEND2_TUPLE1

/****************************************************************
** PP_MAP_PREPEND3_TUPLE
*****************************************************************/
// PP_MAP_PREPEND3_TUPLE will prepend the "whats" to each ele-
// ment, which themselves must be tuples.
#define PP_MAP_PREPEND3_TUPLE( what1, what2, what3, ... )  \
  __VA_OPT__( PP_MAP_PREPEND3_TUPLE1( what1, what2, what3, \
                                      __VA_ARGS__ ) )

#define PP_MAP_PREPEND3_TUPLE1( what1, what2, what3, a, ... ) \
  ( what1, what2, what3, EXPAND a )                           \
      __VA_OPT__(, PP_MAP_PREPEND3_TUPLE1_INDIRECT EMPTY()()( \
                       what1, what2, what3, __VA_ARGS__ ) )

#define PP_MAP_PREPEND3_TUPLE1_INDIRECT() PP_MAP_PREPEND3_TUPLE1

/****************************************************************
** PP_MAP_PREPEND4_TUPLE
*****************************************************************/
// PP_MAP_PREPEND4_TUPLE will prepend the "whats" to each ele-
// ment, which themselves must be tuples.
#define PP_MAP_PREPEND4_TUPLE( what1, what2, what3, what4, \
                               ... )                       \
  __VA_OPT__( PP_MAP_PREPEND4_TUPLE1( what1, what2, what3, \
                                      what4, __VA_ARGS__ ) )

#define PP_MAP_PREPEND4_TUPLE1( what1, what2, what3, what4, a, \
                                ... )                          \
  ( what1, what2, what3, what4, EXPAND a ) __VA_OPT__(         \
      , PP_MAP_PREPEND4_TUPLE1_INDIRECT EMPTY()()(             \
            what1, what2, what3, what4, __VA_ARGS__ ) )

#define PP_MAP_PREPEND4_TUPLE1_INDIRECT() PP_MAP_PREPEND4_TUPLE1

/****************************************************************
** JOIN_SEMIS
*****************************************************************/
// JOIN_SEMIS will join the parameters but with a semicolon after
// each one (including the last).
#define JOIN_SEMIS( ... ) \
  __VA_OPT__( JOIN_SEMIS1( __VA_ARGS__ ) )

#define JOIN_SEMIS1( a, ... ) \
  a;                          \
  __VA_OPT__( JOIN_SEMIS1_INDIRECT EMPTY()()( __VA_ARGS__ ) )

#define JOIN_SEMIS1_INDIRECT() JOIN_SEMIS1

/****************************************************************
** JOIN_WITH
*****************************************************************/
// JOIN_WITH will join the parameters but with a the given thing,
// but not after the last.
#define JOIN_WITH( what, ... ) \
  __VA_OPT__( JOIN_WITH1( what, __VA_ARGS__ ) )

#define JOIN_WITH1( what, a, ... ) \
  a __VA_OPT__(                    \
      what JOIN_WITH1_INDIRECT EMPTY()()( what, __VA_ARGS__ ) )

#define JOIN_WITH1_INDIRECT() JOIN_WITH1

/****************************************************************
** JOIN_WITH_TUPLE_EXPAND
*****************************************************************/
// Will take a tuple, remove the parenthesis, and append the
// contents to each array element (including the last), joining
// the results with commas.
#define JOIN_WITH_TUPLE_EXPAND( what, ... ) \
  __VA_OPT__( JOIN_WITH_TUPLE_EXPAND1( what, __VA_ARGS__ ) )

#define JOIN_WITH_TUPLE_EXPAND1( what, a, ... )     \
  a EXPAND what __VA_OPT__(                         \
      , JOIN_WITH_TUPLE_EXPAND1_INDIRECT EMPTY()()( \
            what, __VA_ARGS__ ) )

#define JOIN_WITH_TUPLE_EXPAND1_INDIRECT() \
  JOIN_WITH_TUPLE_EXPAND1

/****************************************************************
** PP_MAP
*****************************************************************/
// PP_MAP will map the function over the list and emit the
// results without any separators.
#define PP_MAP( ... ) PP_MAP_RECURSE( __VA_ARGS__ )

#define PP_MAP_RECURSE( f, ... )                     \
  __VA_OPT__( f( HEAD( __VA_ARGS__ ) )               \
                  PP_MAP_RECURSE_INDIRECT EMPTY()()( \
                      f, TAIL( __VA_ARGS__ ) ) )

#define PP_MAP_RECURSE_INDIRECT() PP_MAP_RECURSE

/****************************************************************
** PP_MAP_TUPLE
*****************************************************************/
// PP_MAP_TUPLE will map the function over the list of tuples,
// expanding the tuple to be the args of the function.
#define PP_MAP_TUPLE( ... ) PP_MAP_TUPLE_RECURSE( __VA_ARGS__ )

#define PP_MAP_TUPLE_RECURSE( f, ... )                     \
  __VA_OPT__( f HEAD( __VA_ARGS__ )                        \
                  PP_MAP_TUPLE_RECURSE_INDIRECT EMPTY()()( \
                      f, TAIL( __VA_ARGS__ ) ) )

#define PP_MAP_TUPLE_RECURSE_INDIRECT() PP_MAP_TUPLE_RECURSE

/****************************************************************
** PP_MAP_TUPLE_COMMAS
*****************************************************************/
// PP_MAP_TUPLE_COMMAS will map the function over the list of tu-
// ples, expanding the tuple to be the args of the function, sep-
// arating results with commas.
#define PP_MAP_TUPLE_COMMAS( ... ) \
  PP_MAP_TUPLE_COMMAS_RECURSE( __VA_ARGS__ )

#define PP_MAP_TUPLE_COMMAS_RECURSE( f, ... ) \
  __VA_OPT__( PP_MAP_TUPLE_COMMAS1_RECURSE( f, __VA_ARGS__ ) )

#define PP_MAP_TUPLE_COMMAS1_RECURSE( f, a, ... )        \
  f a __VA_OPT__(                                        \
      , PP_MAP_TUPLE_COMMAS1_RECURSE_INDIRECT EMPTY()()( \
            f, __VA_ARGS__ ) )

#define PP_MAP_TUPLE_COMMAS1_RECURSE_INDIRECT() \
  PP_MAP_TUPLE_COMMAS1_RECURSE

/****************************************************************
** PP_MAP_SEMI
*****************************************************************/
// PP_MAP_SEMI will map the function over the list and put a
// semicolon after each result value (including the last one).
#define PP_MAP_SEMI( ... ) PP_MAP_SEMI_RECURSE( __VA_ARGS__ )

#define PP_MAP_SEMI_RECURSE( f, ... )                 \
  __VA_OPT__( f( HEAD( __VA_ARGS__ ) );               \
              PP_MAP_SEMI_RECURSE_INDIRECT EMPTY()()( \
                  f, TAIL( __VA_ARGS__ ) ) )

#define PP_MAP_SEMI_RECURSE_INDIRECT() PP_MAP_SEMI_RECURSE

/****************************************************************
** PP_MAP_AMP
*****************************************************************/
// PP_MAP_AMP will map the function over the list and put
// && between the result values (but not after the last).
#define PP_MAP_AMP( ... ) PP_MAP_AMP_RECURSE( __VA_ARGS__ )

#define PP_MAP_AMP_RECURSE( f, ... ) \
  __VA_OPT__( PP_MAP_AMP1_RECURSE( f, __VA_ARGS__ ) )

#define PP_MAP_AMP1_RECURSE( f, a, ... )                       \
  f( a ) __VA_OPT__( &&PP_MAP_AMP1_RECURSE_INDIRECT EMPTY()()( \
      f, __VA_ARGS__ ) )

#define PP_MAP_AMP1_RECURSE_INDIRECT() PP_MAP_AMP1_RECURSE

/****************************************************************
** PP_MAP_COMMAS
*****************************************************************/
// PP_MAP_COMMAS will map the function over the list and put
// commas between the result values (but not after the last).
#define PP_MAP_COMMAS( ... ) PP_MAP_COMMAS_RECURSE( __VA_ARGS__ )

#define PP_MAP_COMMAS_RECURSE( f, ... ) \
  __VA_OPT__( PP_MAP_COMMAS1_RECURSE( f, __VA_ARGS__ ) )

#define PP_MAP_COMMAS1_RECURSE( f, a, ... )                   \
  f( a )                                                      \
      __VA_OPT__(, PP_MAP_COMMAS1_RECURSE_INDIRECT EMPTY()()( \
                       f, __VA_ARGS__ ) )

#define PP_MAP_COMMAS1_RECURSE_INDIRECT() PP_MAP_COMMAS1_RECURSE

/****************************************************************
** PP_MAP_PLUS
*****************************************************************/
// PP_MAP_PLUS will map the function over the list and put +
// between the result values (but not after the last).
#define PP_MAP_PLUS( ... ) PP_MAP_PLUS_RECURSE( __VA_ARGS__ )

#define PP_MAP_PLUS_RECURSE( f, ... ) \
  __VA_OPT__( PP_MAP_PLUS1_RECURSE( f, __VA_ARGS__ ) )

#define PP_MAP_PLUS1_RECURSE( f, a, ... )                      \
  f( a ) __VA_OPT__( +PP_MAP_PLUS1_RECURSE_INDIRECT EMPTY()()( \
      f, __VA_ARGS__ ) )

#define PP_MAP_PLUS1_RECURSE_INDIRECT() PP_MAP_PLUS1_RECURSE
