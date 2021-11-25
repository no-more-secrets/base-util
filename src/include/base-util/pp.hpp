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
#define SINGLETON_TUPLE( a ) ( a )

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
#define PP_REMOVE_PARENS( ... ) __VA_ARGS__
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
#define TAIL_TUPLE( a ) TAIL a

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

#define PP_PAIR_TAKE_FIRST( a, b ) a
#define PP_PAIR_TAKE_SECOND( a, b ) b

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

/****************************************************************
** PP_ZIP
*****************************************************************/
// PP_ZIP will zip two lists.
//
//   PP_ZIP( (0, 1, 2), (a, b, c) ) --> ((0,a), (1,b), (2,c))
//
// The zipping will stop at whichever list is shorter.

// For some reason we can't use the regular TAIL macro for this
// since it has issues expanding because we are using it deeper
// inside the macro.
#define PP_ZIP_TAIL( a, ... ) __VA_ARGS__

#define PP_ZIP( l, r ) \
  DEFER( PP_ZIP_TAIL )( PP_ZIP_IMPL( l, r ) )

#define PP_ZIP_IMPL( l, r )                                   \
  SWITCH_EMPTY(                                               \
      SWITCH_EMPTY(                                           \
          PP_REMOVE_PARENS(                                   \
              , ( HEAD_TUPLE( l ), HEAD_TUPLE( r ) ) )        \
              PP_ZIP_RECURSE_INDIRECT OBSTRUCT( EMPTY )()()(  \
                  ( TAIL_TUPLE( l ) ), ( TAIL_TUPLE( r ) ) ), \
          , PP_REMOVE_PARENS r ),                             \
      , PP_REMOVE_PARENS l )

#define PP_ZIP_RECURSE_INDIRECT() PP_ZIP_IMPL

/****************************************************************
** PP_ENUMERATE
*****************************************************************/
// PP_ENUMERATE will map a list to a list of pairs with in-
// creasing numbers as the first elements. E.g.:
//
//   (a, b, c) --> ((0,a), (1,b), (2,c))
//
#define PP_NUM_LIST                                             \
  ( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,   \
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, \
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, \
    47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, \
    62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, \
    77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, \
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104,    \
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, \
    117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, \
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, \
    141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, \
    153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, \
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, \
    177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, \
    189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, \
    201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, \
    213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, \
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, \
    237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, \
    249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, \
    261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, \
    273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, \
    285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, \
    297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, \
    309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, \
    321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, \
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, \
    345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, \
    357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, \
    369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, \
    381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, \
    393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, \
    405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, \
    417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, \
    429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, \
    441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, \
    453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, \
    465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, \
    477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, \
    489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, \
    501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, \
    513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, \
    525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, \
    537, 538, 539, 540, 541, 542, 543, 544, 545, 546, 547, 548, \
    549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, \
    561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, \
    573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, \
    585, 586, 587 )

#define PP_ENUMERATE( ... ) \
  PP_ZIP( PP_NUM_LIST, ( __VA_ARGS__ ) )
