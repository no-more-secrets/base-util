/****************************************************************
* Unit tests for filesystem-related code
****************************************************************/
#include "catch2/catch.hpp"

#include "common.hpp"

#include "base-util/fs.hpp"
#include "base-util/line-endings.hpp"
#include "base-util/logger.hpp"
#include "base-util/io.hpp"
#include "base-util/string.hpp"
#include "base-util/misc.hpp"

#include <thread>

using namespace std;

// Utility macro used to wrap string literals containing absolute
// paths; on windows it will attach a root name  to  them  (since
// our path manipulation functions  do  not  support paths with a
// root directory but not a root  name) and does nothing on Linux.
#ifdef _WIN32
#    define A( s ) "C:" s
#else
#    define A( s )      s
#endif

namespace testing {

TEST_CASE( "wildcard" )
{
    // Files only
    REQUIRE( util::wildcard( "",     false ) == (PathVec{}) );
    REQUIRE( util::wildcard( ".",    false ) == (PathVec{}) );
    REQUIRE( util::wildcard( "..",   false ) == (PathVec{}) );
    REQUIRE( util::wildcard( "../",  false ) == (PathVec{}) );
    REQUIRE( util::wildcard( "../a", false ) == (PathVec{}) );
    REQUIRE( util::wildcard( "test/?s.?pp", false ) == (PathVec{"test/fs.cpp"}) );

    // Files and Folders
    REQUIRE( util::wildcard( "",     true ) == (PathVec{})     );
    REQUIRE( util::wildcard( ".",    true ) == (PathVec{"."})  );
    REQUIRE( util::wildcard( "..",   true ) == (PathVec{".."}) );
    REQUIRE( util::wildcard( "../",  true ) == (PathVec{".."}) );
    REQUIRE( util::wildcard( "../a", true ) == (PathVec{})     );
    REQUIRE( util::wildcard( "test/?s.?pp", true ) == (PathVec{"test/fs.cpp"}) );

    auto abs1 = util::lexically_absolute( "." );
    auto abs2 = util::lexically_absolute( "test" );
    REQUIRE( util::wildcard( abs1, true ) == (PathVec{abs1}) );
    REQUIRE( util::wildcard( abs2/"?s.?pp", true ) ==
             (PathVec{abs2/"fs.cpp"}) );

    REQUIRE_THROWS( util::wildcard( "x/y/z/*" ) );

    // Test that wildcard must match full filename.
    REQUIRE( util::wildcard( "test/e",  true ) == (PathVec{}) );
    REQUIRE( util::wildcard( "test/e*", true ) == (PathVec{}) );
    REQUIRE( util::wildcard( "test/fs.cp", true ) == (PathVec{}) );
    REQUIRE( util::wildcard( "test/fs.cpp", true ) == (PathVec{"test/fs.cpp"}) );

#if CASE_INSENSITIVE_FS()
    // Test that on non-linux  platforms  (which  we are assuming
    // have case-insensitive filesystems) that  the wildcard func-
    // tion   enables  case-insensitivity  when  matching   files.
    REQUIRE( util::wildcard( "test/?s.?pp", true ) == (PathVec{"test/fs.cpp"}) );
    REQUIRE( util::wildcard( "test/?s.?PP", true ) == (PathVec{"test/fs.cpp"}) );
#else
    // Otherwise it should be case-sensitive.
    REQUIRE( util::wildcard( "test/?s.?pp", true ) == (PathVec{"test/fs.cpp"}) );
    REQUIRE( util::wildcard( "test/?s.?PP", true ) == (PathVec{}) );
#endif
}

TEST_CASE( "slashes" )
{
    REQUIRE( util::fwd_slashes( "" ) == "" );
    REQUIRE( util::back_slashes( "" ) == "" );

    REQUIRE( util::fwd_slashes( "/") == "/" );
    REQUIRE( util::back_slashes( "\\" ) == "\\" );

    REQUIRE( util::fwd_slashes( "////" ) == "////" );
    REQUIRE( util::back_slashes( "\\\\\\\\" ) == "\\\\\\\\" );

    REQUIRE( util::back_slashes( "////" ) == "\\\\\\\\" );
    REQUIRE( util::fwd_slashes( "\\\\\\\\" ) == "////" );

    REQUIRE( util::back_slashes( "1/a/b/c/d" ) == "1\\a\\b\\c\\d" );
    REQUIRE( util::fwd_slashes( "1\\2\\3\\4\\5" ) == "1/2/3/4/5" );
}

TEST_CASE( "dos_to_from_unix" )
{
    using namespace std::chrono_literals;

    // Ideally, this must be  a  time  delta  that  is small, but
    // large enough to appear in file time stamps. And  on  Linux
    // that's what it is.  But  on  Windows,  it appears that the
    // time points we get from file time stamps don't contain any
    // sub-second  information,  so we need to make it at least a
    // second on that platform.
#ifdef __linux__
    auto const delta = 10ms;
#else
    // OSX too?
    auto const delta = 1001ms;
#endif

    // First test some edge cases with dos2unix.
    vector<char> v1{}, v2{ 0x0A }, v3{ 0x0D }, v4{ 0x0D, 0x0A };
    vector<char> v5{ 0x0A, 0x0D };
    util::dos2unix( v1 ); util::dos2unix( v2 );
    util::dos2unix( v3 ); util::dos2unix( v4 );
    util::dos2unix( v5 );
    REQUIRE( v1 == (vector<char>{ }) );
    REQUIRE( v2 == (vector<char>{ 0x0A }) );
    REQUIRE( v3 == (vector<char>{ }) );
    REQUIRE( v4 == (vector<char>{ 0x0A }) );
    REQUIRE( v5 == (vector<char>{ 0x0A }) );

    // Now test some edge cases with unix2dos.
    vector<char> v6{}, v7{ 0x0A }, v8{ 0x0D }, v9{ 0x0D, 0x0A };
    vector<char> v10{ 0x0A, 0x0D };
    util::unix2dos( v6 ); util::unix2dos( v7 );
    util::unix2dos( v8 ); util::unix2dos( v9 );
    util::unix2dos( v10 );
    REQUIRE( v6 ==  (vector<char>{ }) );
    REQUIRE( v7 ==  (vector<char>{ 0x0D, 0x0A }) );
    REQUIRE( v8 ==  (vector<char>{ 0x0D }) );
    REQUIRE( v9 ==  (vector<char>{ 0x0D, 0x0A }) );
    REQUIRE( v10 == (vector<char>{ 0x0D, 0x0A, 0x0D }) );

    // Test dos2unix( vector )
    auto unix = util::read_file( data_common / "lines-unix.txt" );
    auto win  = util::read_file( data_common / "lines-win.txt"  );

    // Sanity check to make  sure  the  files have different line
    // endings  to  begin  with (e.g., that the repository hasn't
    // auto converted them for us which we don't want).
    REQUIRE( win.size() > unix.size() );

    auto unix_to_unix = unix; util::dos2unix( unix_to_unix );
    auto win_to_unix  = unix; util::dos2unix( win_to_unix  );

    REQUIRE( unix_to_unix == unix );
    REQUIRE( win_to_unix ==  unix );

    // Test dos2unix( string )
    string win_str( &win[0], win.size() );
    auto win_str_tmp = win_str;
    REQUIRE( win_str.size() == 64 );
    util::dos2unix( win_str );
    REQUIRE( win_str.size() == 53 );
    util::unix2dos( win_str );
    REQUIRE( win_str.size() == 64 );
    REQUIRE( win_str == win_str_tmp );

    // Test dos2unix( path )
    REQUIRE( unix.size() == 53 );
    fs::path win_inp  = "lines-win.txt";
    fs::path unix_inp = "lines-unix.txt";
    auto win_tmp  = fs::temp_directory_path()/win_inp;
    auto unix_tmp = fs::temp_directory_path()/unix_inp;
    util::remove_if_exists( win_tmp );
    util::remove_if_exists( unix_tmp );
    util::copy_file( data_common/win_inp,  win_tmp  );
    util::copy_file( data_common/unix_inp, unix_tmp );

    auto time_0 = util::timestamp( win_tmp );
    util::timestamp( unix_tmp, time_0 );

    REQUIRE( util::timestamp( win_tmp  ) ==
             util::timestamp( unix_tmp ) );

    REQUIRE( fs::file_size( win_tmp  ) == 64 );
    REQUIRE( fs::file_size( unix_tmp ) == 53 );

    this_thread::sleep_for( delta );
    REQUIRE( util::dos2unix( win_tmp,  true ) == true  );
    REQUIRE( util::dos2unix( unix_tmp, true ) == false );

    REQUIRE( fs::file_size( win_tmp  ) == 53 );
    REQUIRE( fs::file_size( unix_tmp ) == 53 );

    REQUIRE( util::timestamp( unix_tmp ) == time_0 );
    REQUIRE( util::timestamp( win_tmp )  == time_0 );

    auto time_1 = util::timestamp( win_tmp );

    this_thread::sleep_for( delta );
    REQUIRE( util::dos2unix( win_tmp  ) == false );
    REQUIRE( util::dos2unix( unix_tmp ) == false );

    REQUIRE( fs::file_size( win_tmp  ) == 53 );
    REQUIRE( fs::file_size( unix_tmp ) == 53 );

    REQUIRE( util::timestamp( win_tmp )  == time_1 );
    REQUIRE( util::timestamp( unix_tmp ) == time_1 );

    auto time_2 = util::timestamp( unix_tmp );

    this_thread::sleep_for( delta );
    REQUIRE( util::unix2dos( win_tmp  ) == true );
    REQUIRE( util::unix2dos( unix_tmp ) == true );

    REQUIRE( fs::file_size( win_tmp  ) == 64 );
    REQUIRE( fs::file_size( unix_tmp ) == 64 );

    REQUIRE( util::timestamp( win_tmp )  > time_2 );
    REQUIRE( util::timestamp( unix_tmp ) > time_2 );
}

TEST_CASE( "touch" )
{
    auto p = fs::temp_directory_path();
    util::log << "temp folder: " << p << "\n";
    auto base_time = util::timestamp( p );

    fs::path t1 = p / "AbCdEfGhIjK";
    fs::path t2 = p / "bCdEfGhIjKl/MnOp";

    util::remove_if_exists( t1 );

    ASSERT_( !fs::exists( t1 ) );

    using namespace std::chrono_literals;

    // Ideally, this must be  a  time  delta  that  is small, but
    // large enough to appear in file time stamps. And  on  Linux
    // that's what it is.  But  on  Windows,  it appears that the
    // time points we get from file time stamps don't contain any
    // sub-second  information,  so we need to make it at least a
    // second on that platform.
#ifdef __linux__
    auto const delta = 10ms;
#else
    // OSX too?
    auto const delta = 1001ms;
#endif

    this_thread::sleep_for( delta );
    util::touch( t1 );
    ASSERT( fs::exists( t1 ), t1 << " does not exist" );

    // Check  that  the time stamp on the file we just touched is
    // later  than  the original timestamp on temp folder (before
    // we created the file in it).
    bool gt = util::timestamp( t1 ) > base_time;
    REQUIRE( gt == true );

    // Check that an attempt to touch a file  in  a  non-existent
    // folder will throw.
    REQUIRE_THROWS( util::touch( t2 ) );
}

TEST_CASE( "read_write_file" )
{
    auto f = data_common / "3-lines.txt";

    auto v = util::read_file_lines( f );
    REQUIRE( v == (StrVec{ "line 1", "line 2", "line 3" }) );

    auto s = util::read_file_str( f );
    REQUIRE( s == "line 1\nline 2\nline 3" );

    // read file as vector of char.
    vector<char> v2 = util::read_file( data_common/"random.bin" );
    REQUIRE( v2.size() == 1920 );

    // Test a random byte in the file.
    REQUIRE( v2[0x6c0] == char( 0x6b ) );

    // Now write to a file and read it back in to verify.
    vector<char> v3{ 3, 4, 5, 6, 7 };

    auto p = fs::temp_directory_path() / "34567";
    util::remove_if_exists( p );

    util::write_file( p, v3 );
    REQUIRE( fs::exists( p ) );
    REQUIRE( fs::file_size( p ) == v3.size() );

    auto v4 = util::read_file( p );
    REQUIRE( v3 == v4 );

    auto copy = fs::temp_directory_path()/"3-lines.txt";
    util::copy_file( data_common/"3-lines.txt", copy );
    REQUIRE( fs::file_size( copy ) == 21 );
}

TEST_CASE( "rename" )
{
    auto f1 = fs::temp_directory_path()/"abcdefg";
    auto f2 = fs::temp_directory_path()/"gabcdef";

    util::remove_if_exists( f1 );
    util::remove_if_exists( f2 );
    REQUIRE( fs::exists( f1 ) == false );
    REQUIRE( fs::exists( f2 ) == false );

    bool res;

    // Should a) not throw, b) return false
    res = util::rename_if_exists( f1, f2 );
    REQUIRE( res == false );

    util::touch( f1 );

    res = util::rename_if_exists( f1, f2 );
    REQUIRE( res == true );

    REQUIRE( fs::exists( f1 ) == false );
    REQUIRE( fs::exists( f2 ) == true  );

    res = util::rename_if_exists( f1, f2 );
    REQUIRE( res == false );

    REQUIRE( fs::exists( f1 ) == false );
    REQUIRE( fs::exists( f2 ) == true  );

    util::touch( f1 );

    res = util::rename_if_exists( f1, f2 );
    REQUIRE( res == true );

    REQUIRE( fs::exists( f1 ) == false );
    REQUIRE( fs::exists( f2 ) == true  );
}

TEST_CASE( "filesystem" )
{
    bool b;

    util::CaseSensitive sens = util::CaseSensitive::YES;

    b = util::path_equals( "", "",                sens ); REQUIRE( b == true  );
    b = util::path_equals( "A", "",               sens ); REQUIRE( b == false );
    b = util::path_equals( "", "A",               sens ); REQUIRE( b == false );
    b = util::path_equals( "A/B", "A",            sens ); REQUIRE( b == false );
    b = util::path_equals( "A", "A/B",            sens ); REQUIRE( b == false );
    b = util::path_equals( "A/B", "A/B",          sens ); REQUIRE( b == true  );
    b = util::path_equals( "A//B///C//", "A/B/C", sens ); REQUIRE( b == true  );
    b = util::path_equals( "a/b/c", "A/B/C",      sens ); REQUIRE( b == false );
    b = util::path_equals( "A", "a",              sens ); REQUIRE( b == false );

    b = util::path_equals(    "A/B/C",  A( "/A/B/C" ), sens ); REQUIRE( b == false );
    b = util::path_equals( A( "/abc" ), A( "/abc"   ), sens ); REQUIRE( b == true  );
    b = util::path_equals( A( "/ABC" ), A( "/abc"   ), sens ); REQUIRE( b == false );

    sens = util::CaseSensitive::NO;

    b = util::path_equals( "", "",                sens ); REQUIRE( b == true  );
    b = util::path_equals( "A", "",               sens ); REQUIRE( b == false );
    b = util::path_equals( "", "A",               sens ); REQUIRE( b == false );
    b = util::path_equals( "A/B", "A",            sens ); REQUIRE( b == false );
    b = util::path_equals( "A", "A/B",            sens ); REQUIRE( b == false );
    b = util::path_equals( "A/B", "A/B",          sens ); REQUIRE( b == true  );
    b = util::path_equals( "A//B///C//", "A/B/C", sens ); REQUIRE( b == true  );
    b = util::path_equals( "a/b/c", "A/B/C",      sens ); REQUIRE( b == true  );
    b = util::path_equals( "A", "a",              sens ); REQUIRE( b == true  );

    b = util::path_equals(    "A/B/C",  A( "/A/B/C" ), sens ); REQUIRE( b == false );
    b = util::path_equals( A( "/abc" ), A( "/abc"   ), sens ); REQUIRE( b == true  );
    b = util::path_equals( A( "/ABC" ), A( "/abc"   ), sens ); REQUIRE( b == true  );
}

TEST_CASE( "lexically_normal" )
{
    auto f = util::lexically_normal;

    // Absolute paths
    REQUIRE( f( A( "/"                  )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a"                 )  ) == A( "/a"         )  );
    REQUIRE( f( A( "/.."                )  ) == A( "/"          )  );
    REQUIRE( f( A( "/../"               )  ) == A( "/"          )  );
    REQUIRE( f( A( "/../../../"         )  ) == A( "/"          )  );
    REQUIRE( f( A( "/..//../c/."        )  ) == A( "/c"         )  );
    REQUIRE( f( A( "/.//../../."        )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a/b/c/../../c"     )  ) == A( "/a/c"       )  );
    REQUIRE( f( A( "/a/b/c/../../../"   )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a/b/../../../../"  )  ) == A( "/"          )  );
    REQUIRE( f( A( "/aa/bb/cc/./../x/y" )  ) == A( "/aa/bb/x/y" )  );

#ifdef _WIN32
    REQUIRE_THROWS( f( "C:abc" ) );
    REQUIRE_THROWS( f( "/abc"  ) );
#endif

    // Relative paths
    REQUIRE( f( ""                  ) == "."         );
    REQUIRE( f( "a"                 ) == "a"         );
    REQUIRE( f( ".."                ) == ".."        );
    REQUIRE( f( "../"               ) == ".."        );
    REQUIRE( f( "../../../"         ) == "../../.."  );
    REQUIRE( f( "..//../c/."        ) == "../../c"   );
    REQUIRE( f( ".//../../."        ) == "../.."     );
    REQUIRE( f( "a/b/c/../../c"     ) == "a/c"       );
    REQUIRE( f( "a/b/c/../../../"   ) == "."         );
    REQUIRE( f( "a/b/../../../../"  ) == "../.."     );
    REQUIRE( f( "aa/bb/cc/./../x/y" ) == "aa/bb/x/y" );
}

TEST_CASE( "lexically_absolute" )
{
    auto f = util::lexically_absolute;

    // Absolute paths. This behavior should  be identical to just
    // passing to lexically_normal, so  we  just copy those tests
    // from above, even though it might be overkill.
    REQUIRE( f( A( "/"                  )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a"                 )  ) == A( "/a"         )  );
    REQUIRE( f( A( "/.."                )  ) == A( "/"          )  );
    REQUIRE( f( A( "/../"               )  ) == A( "/"          )  );
    REQUIRE( f( A( "/../../../"         )  ) == A( "/"          )  );
    REQUIRE( f( A( "/..//../c/."        )  ) == A( "/c"         )  );
    REQUIRE( f( A( "/.//../../."        )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a/b/c/../../c"     )  ) == A( "/a/c"       )  );
    REQUIRE( f( A( "/a/b/c/../../../"   )  ) == A( "/"          )  );
    REQUIRE( f( A( "/a/b/../../../../"  )  ) == A( "/"          )  );
    REQUIRE( f( A( "/aa/bb/cc/./../x/y" )  ) == A( "/aa/bb/x/y" )  );

    // Relative paths
    auto cp = fs::current_path();

    REQUIRE( f( ""    ) == cp         );
    REQUIRE( f( "a"   ) == cp/"a"     );
    REQUIRE( f( "."   ) == cp         );
    REQUIRE( f( "a/b" ) == cp/"a"/"b" );

    // Test the slash() function
    REQUIRE( util::slash( A( "/"     ),  ( "."     ) ) == A( "/."         ) );
    REQUIRE( util::slash(  ( "."     ), A( "/"     ) ) == A( "/"          ) );
    REQUIRE( util::slash(  ( "a/b/c" ), A( "/d/e"  ) ) == A( "/d/e"       ) );
    REQUIRE( util::slash( A( "/d/e"  ),  ( "a/b/c" ) ) == A( "/d/e/a/b/c" ) );
    REQUIRE( util::slash(  ( "a/b/c" ),  ( "d/e"   ) ) ==  ( "a/b/c/d/e"  ) );
}

TEST_CASE( "lexically_relative" )
{
    auto f = util::lexically_relative;

    // Relative paths.
    REQUIRE( f( "", "" ) == "." );

    REQUIRE( f( ".", ""  ) == "." );
    REQUIRE( f( "",  "." ) == "." );
    REQUIRE( f( ".", "." ) == "." );

    REQUIRE( f( "..", ""   ) == ".." );
    REQUIRE( f( ".",  ".." ) == ""   );
    REQUIRE( f( "..", "."  ) == ".." );
    REQUIRE( f( "..", ".." ) == "."  );

    REQUIRE( f( "a", ""  ) == "a"  );
    REQUIRE( f( "",  "a" ) == ".." );
    REQUIRE( f( "a", "a" ) == "."  );

    REQUIRE( f( "a", "b"   ) == "../a"    );
    REQUIRE( f( "a", "b/b" ) == "../../a" );
    REQUIRE( f( "a", "b/a" ) == "../../a" );
    REQUIRE( f( "a", "a/b" ) == ".."      );

    REQUIRE( f( "..",       "a" ) == "../.."       );
    REQUIRE( f( "../..",    "a" ) == "../../.."    );
    REQUIRE( f( "../../..", "a" ) == "../../../.." );

    REQUIRE( f( "..",       "a/b/c" ) == "../../../.."       );
    REQUIRE( f( "../..",    "a/b/c" ) == "../../../../.."    );
    REQUIRE( f( "../../..", "a/b/c" ) == "../../../../../.." );

    REQUIRE( f( ".", "../../.." ) == "" );

    REQUIRE( f( ".",  "../a" ) == ""   );
    REQUIRE( f( ".",  "a/.." ) == "."  );
    REQUIRE( f( "..", "../a" ) == ".." );
    REQUIRE( f( "..", "a/.." ) == ".." );

    REQUIRE( f( "..",       "a/b/c/.."          ) == "../../.." );
    REQUIRE( f( "../..",    "a/b/c/../.."       ) == "../../.." );
    REQUIRE( f( "../../..", "../../../a/b/c"    ) == "../../.." );
    REQUIRE( f( "../../..", "../../../../a/b/c" ) == ""         );

    REQUIRE( f( "..",       "a/b/../c"       ) == "../../.." );
    REQUIRE( f( "../..",    "a/b/../../c"    ) == "../../.." );
    REQUIRE( f( "../../..", "a/b/c/../../.." ) == "../../.." );
    REQUIRE( f( "../../..", "a/../b/../c/.." ) == "../../.." );

    REQUIRE( f( "a/b/c/d/e", "a/b/c/d/e" ) == "."     );
    REQUIRE( f( "a/b/c/d/e", "a/b/c"     ) == "d/e"   );
    REQUIRE( f( "a/b/c",     "a/b/c/d/e" ) == "../.." );

    REQUIRE( f( "a/b/x/y/z", "a/b/c/d/e" ) == "../../../x/y/z"           );
    REQUIRE( f( "u/v/x/y/z", "a/b/c/d/e" ) == "../../../../../u/v/x/y/z" );

#ifdef _WIN32
    // Invalid absolute paths
    REQUIRE( f( "/" ==  "a"  ) );
    REQUIRE( f( "a" ==  "/"  ) );
    REQUIRE( f( "C:" == "a"  ) );
    REQUIRE( f( "a" ==  "C:" ) );
#endif

    // Absolute paths.
    REQUIRE( f( A( "/" ), A( "/" ) ) == "." );

    REQUIRE( f( A( "/" ), "." ) == "" );
    REQUIRE( f( ".", A( "/" ) ) == "" );

    REQUIRE( f( A( "/.." ), A( "/"   ) ) == "." );
    REQUIRE( f( A( "/"   ), A( "/.." ) ) == "." );
    REQUIRE( f( A( "/.." ), A( "/.." ) ) == "." );

    REQUIRE( f( A( "/a" ), A( "/"  ) ) == "a"  );
    REQUIRE( f( A( "/"  ), A( "/a" ) ) == ".." );
    REQUIRE( f( A( "/a" ), A( "/a" ) ) == "."  );

    REQUIRE( f( A( "/a" ), A( "/b"   ) ) == "../a"    );
    REQUIRE( f( A( "/a" ), A( "/b/b" ) ) == "../../a" );
    REQUIRE( f( A( "/a" ), A( "/b/a" ) ) == "../../a" );
    REQUIRE( f( A( "/a" ), A( "/a/b" ) ) == ".."      );

    REQUIRE( f( A( "/.."       ), A( "/a" ) ) == ".." );
    REQUIRE( f( A( "/../.."    ), A( "/a" ) ) == ".." );
    REQUIRE( f( A( "/../../.." ), A( "/a" ) ) == ".." );

    REQUIRE( f( A( "/.."       ), A( "/a/b/c" ) ) == "../../.." );
    REQUIRE( f( A( "/../.."    ), A( "/a/b/c" ) ) == "../../.." );
    REQUIRE( f( A( "/../../.." ), A( "/a/b/c" ) ) == "../../.." );

    REQUIRE( f( A( "/" ), A( "/../a" ) ) == ".." );

    REQUIRE( f( A( "/"   ), A( "/a/.." ) ) == "."  );
    REQUIRE( f( A( "/.." ), A( "/../a" ) ) == ".." );
    REQUIRE( f( A( "/.." ), A( "/a/.." ) ) == "."  );

    REQUIRE( f( A( "/.."       ), A( "/a/b/c/.."       )  ) == "../.." );
    REQUIRE( f( A( "/../.."    ), A( "/a/b/c/../.."    )  ) == ".." );
    REQUIRE( f( A( "/../../.." ), A( "/../../../a/b/c" )  ) == "../../.." );

    REQUIRE( f( A( "/.."       ), A( "/a/b/../c"       )   ) == "../.." );
    REQUIRE( f( A( "/../.."    ), A( "/a/b/../../c"    )   ) == ".."    );
    REQUIRE( f( A( "/../../.." ), A( "/a/b/c/../../.." )   ) == "."     );
    REQUIRE( f( A( "/../../.." ), A( "/a/../b/../c/.." )   ) == "."     );

    REQUIRE( f( A( "/a/b/c/d/e" ), A( "/a/b/c/d/e" )   ) == "."     );
    REQUIRE( f( A( "/a/b/c/d/e" ), A( "/a/b/c"     )   ) == "d/e"   );
    REQUIRE( f( A( "/a/b/c"     ), A( "/a/b/c/d/e" )   ) == "../.." );

    REQUIRE( f( A( "/a/b/x/y/z" ), A( "/a/b/c/d/e" )   ) == "../../../x/y/z"           );
    REQUIRE( f( A( "/u/v/x/y/z" ), A( "/a/b/c/d/e" )   ) == "../../../../../u/v/x/y/z" );

    REQUIRE( f( A( "/a/b/c/d/e" ), A( "/a/./."      )   ) == "b/c/d/e" );
    REQUIRE( f( A( "/a/b/c"     ), A( "/a/./c/./.." )   ) == "b/c"     );
}

} // namespace testing
