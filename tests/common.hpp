/****************************************************************
* Unit test infrastructure
****************************************************************/
#pragma once

#include "base-util/colors.hpp"
#include "base-util/macros.hpp"

#include <exception>
#include <functional>
#include <regex>
#include <string>
#include <vector>

namespace testing {

// These are updated on each  macro  call  and are printed out in
// the  event  that  an exception happens that does not come from
// one of the unit testing macros  in  order  to help the user to
// figure out which line threw the exception.
extern size_t      checkpoint_line;
extern char const* checkpoint_file;

// This  is the type of a unit test function. All that it does is
// to  throw an exception if the test fails. If it returns, it is
// assumed to have passed.
using TestType = void(void);

// Global test list: tests are automatically registered and added
// to this list at static initialization time.
std::vector<TestType*>& test_list();

// Run all unit tests in all modules.
void run_all_tests();

// Run a single unit test; no need to call this directly.
void run_single_test( size_t      line,
                      char const* file,
                      char const* name,
                      std::function<void(void)> func );

// Functions for printing to console
std::string fail();
std::string pass();
std::string skip();
std::string bar();

// This  exception is thrown by the unit testing macros to signal
// a failure so that we  can  distinguish  it from other kinds of
// generic std::exception's.
struct failed_exception : public std::exception {
    failed_exception( std::string const& what )
        : msg( what ) {}
    char const* what() const noexcept
        { return msg.c_str(); }
    std::string msg;
};

// This exception is thrown by a  test  to indicate that the test
// should be skipped.
struct skipped_exception : public std::exception {};

} // namespace testing

#define CHECKPOINT \
    testing::checkpoint_line = __LINE__;                 \
    testing::checkpoint_file = __FILE__;                 \

#define TRUE_( a ) {                                     \
    CHECKPOINT                                           \
    if( !(a) ) {                                         \
        using util::operator<<;                          \
        ostringstream ss;                                \
        ss << "On line " << TO_STRING( __LINE__ ) ": ";  \
        ss << "assert " << #a;                           \
        throw failed_exception( ss.str() );              \
    } }

// a  is an expression that must be true, and b is just something
// printable that will be displayed  in  the  event  a  is  false.
#define TRUE( a, b ) {                                   \
    CHECKPOINT                                           \
    if( !(a) ) {                                         \
        using util::operator<<;                          \
        ostringstream ss;                                \
        ss << "On line " << TO_STRING( __LINE__ ) ": ";  \
        ss << b;                                         \
        throw failed_exception( ss.str() );              \
    } }

#define EQUALS( a, b ) {                                      \
    CHECKPOINT                                                \
    if( !((a) == (b)) ) {                                     \
        using util::operator<<;                               \
        ostringstream ss;                                     \
        ss << "On line " << TO_STRING( __LINE__ ) ": " << #a  \
             << " != " << #b << "\n";                         \
        ss << "instead got: " << (a);                         \
        ss << " ?= " << #b;                                   \
        throw failed_exception( ss.str() );                   \
    } }

// Checks that the string a fully  matches the regex rx, where rx
// is just a regex string (not regex object).
#define MATCHES( a, rx ) {                               \
    smatch m;                                            \
    TRUE( regex_match( a, m, regex( rx ) ), "string " << \
          quoted( a ) << " did not match regex " <<      \
          quoted( rx ) );                                \
}

// This  is  a  bit messy because we want to throw just after the
// `a` gets executed, but we can't throw there because  otherwise
// it  would  get  caught  by the catch block, so instead we just
// record  if  the  code didn't throw and then throw an exception
// later.
#define THROWS( a ) {                                    \
    CHECKPOINT                                           \
    bool threw = false;                                  \
    try {                                                \
        a;                                               \
    } catch( std::exception const& ) {                   \
        threw = true;                                    \
    }                                                    \
    TRUE( threw, "expression: " #a " did not throw." );  \
}

// This  is  used  to  create a unit test function. The anonymous
// namespace is surrounding STARTUP() to  prevent  duplicate  sym-
// bols  from  appearing  publicly  due  to different translation
// units declaring TEST macros  on  the  same  source  file  line.
#define TEST( a )                                               \
    void STRING_JOIN( __test_, a )();                           \
    void STRING_JOIN( test_, a )() {                            \
        testing::run_single_test( __LINE__, __FILE__,           \
                                  TO_STRING( a ),               \
                                  STRING_JOIN( __test_, a ) );  \
    }                                                           \
    namespace {                                                 \
        STARTUP() {                                             \
            test_list().push_back( STRING_JOIN( test_, a ) );   \
        }                                                       \
    }                                                           \
    void STRING_JOIN( __test_, a )()
