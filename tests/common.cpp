/****************************************************************
* Helpers for unit tests
****************************************************************/
#include "common.hpp"
#include "base-util/main.hpp"

#include "base-util/logger.hpp"
#include "base-util/string-util.hpp"
#include "base-util/types.hpp"

using namespace std;
using namespace std::string_literals;

namespace testing {

// These are updated on each  macro  call  and are printed out in
// the  event  that  an exception happens that does not come from
// one of the unit testing macros  in  order  to help the user to
// figure out which line threw the exception.
size_t      checkpoint_line = 0;
char const* checkpoint_file = "none";

string fail() { return string( util::c_red )    + "fail" + string( util::c_norm ); }
string pass() { return string( util::c_green )  + "pass" + string( util::c_norm ); }
string skip() { return string( util::c_yellow ) + "skip" + string( util::c_norm ); }

string bar() {
    return "---------------------------------------------------";
}

// Global test list: tests are automatically registered and added
// to this list at static initialization time.
vector<TestType*>& test_list() {
    static vector<TestType*> g_tests;
    return g_tests;
}

// Run all unit tests in all modules.
void run_all_tests() {
    for( auto f : test_list() ) f();
}

// Run a single unit test; no need to call this directly.
void run_single_test( size_t      line,
                      char const* file,
                      char const* name,
                      function<void(void)> func ) {

    using util::operator<<;
    checkpoint_line = line;
    checkpoint_file = file;
    cout << string( name ) + " ";
    enum class Res { PASSED, SKIPPED, FAILED };
    Res result = Res::FAILED;
    string err;
    try {
        func();
        result = Res::PASSED;
    } catch( skipped_exception const& ) {
        result = Res::SKIPPED;
    } catch( failed_exception const& e ) {
        err = e.what();
        result = Res::FAILED;
    //} catch( sqlite::sqlite_exception const& e ) {
    //    std::cerr << sqlite::exception_msg( e ) << "\n";
    } catch( exception const& e ) {
        err = e.what();
        err += "\nLast checkpoint: ";
        err += string( checkpoint_file );
        err += ", line ";
        err += util::to_string( checkpoint_line );
        result = Res::FAILED;
    } catch( ... ) {
        err = "unknown exception";
        err += "\nLast checkpoint: ";
        err += string( checkpoint_file );
        err += ", line ";
        err += util::to_string( checkpoint_line );
        result = Res::FAILED;
    }
    bool failed = false;
    switch( result ) {
    case Res::FAILED:
        failed = true;
        cerr << fail() << "\n" << bar() << "\n";
        cerr << err << "\n" << bar() << "\n";
        break;
    case Res::SKIPPED:
        cout << skip();
        break;
    case Res::PASSED:
        cout << pass();
        break;
    }
    cout << util::c_norm << "\n";
    if( failed ) throw runtime_error( "test failed" );
}

} // namespace testing

// This is the entrypoint for the unit test executable.
int main_( int, char** )
{
    util::Logger::enabled = false;

    testing::run_all_tests();

    return 0;
}
