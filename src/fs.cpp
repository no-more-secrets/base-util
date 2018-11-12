/****************************************************************
* filesystem related utilities
****************************************************************/
#include "base-util/macros.hpp"
#include "base-util/fs.hpp"
#include "base-util/logger.hpp"
#include "base-util/string-util.hpp"

#include <algorithm>
#include <fstream>

#ifdef _WIN32
#   include <windows.h>
#endif

using namespace std;

namespace util {

namespace {

void validate( fs::path const& p ) {
#ifdef _WIN32
    ASSERT( p.has_root_name() == p.has_root_directory(),
            "path " << p << " must either have both a root name "
            "and a root directory, or must have neither." );
#else
    (void)p;
#endif
}

}

// This function joins two paths, but if the path on the rhs side
// qualifies as absolute than we  use  it as the result, ignoring
// the lhs. We shouldn't need this function  since  the  standard
// `/`  and  `/=` operators (which join paths) are supposed to do
// this for us, but  the  libstdc++ implementation doesn't appear
// to be working right at the moment.
fs::path slash( fs::path const& lhs, fs::path const& rhs ) {
    return rhs.is_absolute() ? rhs : (lhs/rhs);
}

// This will put the path into  normal  form  and  preserving  ab-
// solute/relative nature. Path must exist,  and will resolve sym-
// links.  The version of this function that does not require the
// file  to  exist  (and won't resolve links) is lexically_normal.
fs::path normpath( fs::path const& p ) {
    return fs::canonical( p );
}

// This  is  like normpath except that it makes the path absolute
// if it is not already (if it is relative, it  is  assumed  rela-
// tive to CWD. The version of this function  that  does  not  re-
// quire the file to exist  (and  won't resolve links) is lexical-
// ly_absolute.
fs::path absnormpath( fs::path const& p ) {
    return fs::canonical( fs::absolute( p ) );
}

/* Implementation of lexically_normal.  Put  a  path  into normal
 * form without regard to whether  or  not  it  exists (and do so
 * without touching the filesystem  at  all;  hence we also don't
 * follow symlinks). The C++17 filesystem library has this method
 * as a member function of  the  path  class,  but it has not yet
 * been implemented in gcc at the time of this writing. Once  gcc
 * 8 is released, then this function can probably be deleted  and
 * the  usage of it can be replaced with p.lexicaly_normal(). How-
 * ever, note that  the  implementation  of  this  function  will
 * likely be slightly different from  the  one in the standard li-
 * brary.  In  particular,  at  the  moment,  it does not convert
 * slashes  using  make_preferred. Note that the code doesn't per-
 * form these actions in the same  order  as the steps below, but
 * the overall effect should be the same.
 *
 * From http://en.cppreference.com/w/cpp/filesystem/path, a  path
 * can be normalized by following this algorithm:
 *
 *  1) If the path is empty, stop (normal form of an  empty  path
 *     is an empty path)
 *  2) Replace each directory-separator  (which  may  consist  of
 *     multiple slashes) with  a single path::preferred_separator.
 *  3) Replace each slash character in the root-name with
 *     path::preferred_separator.
 *  4) Remove each dot and any immediately following
 *     directory-separator.
 *  5) Remove each non-dot-dot filename immediately followed by a
 *     directory-separator and a dot-dot, along with  any  immedi-
 *     ately following directory-separator.
 *  6) If there is  root-directory,  remove  all dot-dots and any
 *     directory-separators immediately following them.
 *  7) If the  last  filename  is  dot-dot,  remove  any trailing
 *     directory-separator.
 *  8) If the path is empty, add  a  dot (normal form of ./ is .)
 */
fs::path lexically_normal( fs::path const& p ) {

    validate( p );

    bool is_abs = p.is_absolute(), is_rel = p.is_relative();
    fs::path res;
    for( auto const& c : p ) {
        if( c == "." )
            // The single dot we can always skip  at  this  stage.
            // The only time we need  one  is  when we are normal-
            // izing an empty path, which must  be  converted  to
            // single dot, however we will do  that  at  the  end.
            continue;
        if( c == ".." ) {
            // If we have encountered  a  ..  and  if  we have at
            // least one component in the  result  so far then we
            // can  eliminate  it. We need to guard this with one
            // additional  check  (has_parent_path)  in  the case
            // that we have an absolute  path because an absolute
            // path can consist just  of  the  root  (/) but will
            // still report having a file name (/) which we don't
            // want to remove.
            if( res.has_filename() &&
               (res.has_parent_path() || is_rel) ) {
                // If we are  a  relative  path  then  there is a
                // chance that the  filename  could  be .. (since
                // those are allowed to accumulate at  the  start
                // of  a  relative path), but which we don't want
                // to remove. Also, on Windows,  it  is  possible
                // that res is a root directory here because  the
                // root directory will also  be  accompanied by a
                // root name, which gives it a  non-empty  parent
                // path (unlike  on  Linux  where  that situation
                // would  not  have  passed  through the above if
                // statement);  therefore,  we  need one explicit
                // check that res is not a root directory.
                if( res.filename() != ".." &&
                    !res.filename().has_root_directory() ) {
                    res = res.parent_path();
                    continue;
                }
            }
            if( is_abs )
                // On an abs path we can ignore .. at  the  begin-
                // ning of paths. We  know  that  we're at the be-
                // ginning because, if we we  not  at  the  begin-
                // ning,  the above block would have continued be-
                // cause a) we'd have a  parent path, and b) that
                // parent path would not be a .. because previous
                // ..'s  get  filtered  out  on abs paths (due to
                // this very piece of logic).
                continue;
            // It's a relative path, so that  means  we  need  to
            // keep ..'s at the beginning of paths,  so  we  fall
            // through.
        }
        res /= c;
    }
    // Result will never be empty.
    return res.empty() ? "." : res;
}

// This  is  like  absnormpath  except that it will not query the
// file system. If the path p is relative  then  it  will  be  ap-
// pended  to  the  CWD  and  the result normalized using lexical-
// ly_normal. Result is an absolute path in  normal  for  without
// links resolved and which may not exist.
fs::path lexically_absolute( fs::path const& p ) {
    return lexically_normal( slash( fs::current_path(), p ) );
}

/* Implemenation of the  lexically_relative  function.  Find  the
 * relative  path between the given path and base path without re-
 * gard  to  whether or not it exists (and do so without touching
 * the  filesystem  at  all; hence we also don't follow symlinks).
 * The C++17 filesystem library has this  method as a member func-
 * tion of the path class, but it has not yet been implemented in
 * gcc at the time of this writing.  Once gcc 8 is released, then
 * this function can probably be deleted and the usage of it  can
 * be replaced with p.lexicaly_relative(). However, note that the
 * implementation of this function will likely  be  slightly  dif-
 * ferent from the one in the standard library.
 *
 * Return value: empty path on error, result otherwise.
 *
 * NOTE: both arguments will  be  normalized, and result returned
 * will always be in normal form  unless an error (empty path) is
 * returned.
 *
 * NOTE:  this function doesn't always work right in certain edge
 * cases involving both relative paths and the presence of double
 * dots  in the base  path due to  the fact that  this is lexical.
 * The function  tries to detect these scenarios  and will return
 * an error (empty  path) in those cases.
 *
 * From en.cppreference.com/w/cpp/filesystem/path/lexically_normal
 * the algorithm is as follows:
 *
 *   if (root_name()   != base.root_name() ) ||
 *      (is_absolute() != base.is_absolute() ||
 *      (!has_root_directory() && base.has_root_directory())
 *        returns a default-constructed path.
 *
 *   Otherwise,  determines the first mismatched element of *this
 *   and base as if by:
 *
 *      auto [a, b] = mismatch(begin(), end(),
 *                             base.begin(), base.end())
 *
 *      if  a  ==  end()  and b == base.end(), returns path(".");
 *
 *   Otherwise,  if the number of dot-dot filename elements in [b,
 *   base.end())  is greater than the number of filename elements
 *   that are neither dot nor  dot-dot,  returns  a  default  con-
 *   structed path. otherwise returns an object composed from:  a
 *   default-constructed path() followed by as many  applications
 *   of operator/=(path("..")) as there were filename elements in
 *   [b,  base.end())  that  are  neither  dot  nor dot-dot minus
 *   number  of  dot-dot  elements in that range, followed by one
 *   application of operator/= for each element in the  half-open
 *   range [a, end())
 */
fs::path lexically_relative( fs::path const& p_,
                             fs::path const& base_ ) {

    // These will also call validate so we don't need to here.
    fs::path p    = lexically_normal( p_    );
    fs::path base = lexically_normal( base_ );

    if( (p.root_name()   != base.root_name())   ||
        (p.is_absolute() != base.is_absolute()) ||
        (!p.has_root_directory() && base.has_root_directory()) )
         return {};

    bool is_abs = p.is_absolute() /* == base.is_absolute() */;

    auto [a, b] = mismatch( begin( p ), end( p ),
                            begin( base ), end( base ) );

    auto n_dd = count( b, end( base ), ".." );
    auto n_d  = count( b, end( base ), "."  );
    auto dist = distance( b, end( base ) );
    auto n_r  = dist - 2*n_dd - n_d;

    ASSERT_( dist >= (n_d + n_dd) );

    // The next test is to be sure that we return an empty path
    // (meaning error I suppose) in situations like the following,
    // where, given that we can't query the filesystem, we cannot
    // be sure of the correct answer:
    //   lexically_relative( ".", "a/.." ) == "."; // ok
    //   lexically_relative( ".", "../a" ) == ???; // bad
    // Though this only arises with relative paths.
    if( n_dd > 0 && !is_abs ) {
        auto norm = lexically_normal( path_( b, end( base ) ) );
        if( count( begin( norm ), end( norm ), ".." ) > 0 )
            return {};
    }

    fs::path res;
    while( n_r-- > 0 )
        res /= fs::path( ".." );

    for( auto i = a; i != end( p ); ++i )
        res /= *i;

    // Result, which at this  point  is  interpreted as "correct"
    // will be changed to normal form if it is empty, that way we
    // can use "empty" to mean "couldn't find solution".
    return lexically_normal( res );
}

// Flip any backslashes to foward slashes.
string fwd_slashes( string_view in ) {
    string out( in );
    replace( begin( out ), end( out ), '\\', '/' );
    return out;
}

// Flip any backslashes to forward slashes.
StrVec fwd_slashes( StrVec const& v ) {
    vector<string> res( v.size() );
    auto resolve = []( string_view sv ) {
        return fwd_slashes( sv );
    };
    transform( begin( v ), end( v ), begin( res ), resolve );
    return res;
}

// Constructs  a path from a pair of iterators to path components.
// Didn't see this available in  the  standard,  but  could  have
// missed it.
fs::path path_( fs::path::const_iterator b,
				fs::path::const_iterator e ) {
    fs::path res;
    for( auto i = b; i != e; ++i )
        res /= *i;
    return res;
}

// Flip any forward slashes to back slashes.
string back_slashes( string_view in ) {
    string out( in );
    replace( begin( out ), end( out ), '/', '\\' );
    return out;
}

// Flip any forward slashes to back slashes.
StrVec back_slashes( StrVec const& v ) {
    vector<string> res( v.size() );
    auto resolve = []( string_view sv ) {
        return back_slashes( sv );
    };
    transform( begin( v ), end( v ), begin( res ), resolve );
    return res;
}

// The purpose of this function  is  to  compare two paths lexico-
// graphically  with the option of case insensitivity. On Windows,
// the comparison will  default  to  being  case  insensitive. On
// Linux it will default to case-sensitive, but  in  either  case,
// this can be overridden.
bool path_equals( fs::path const& a,
                  fs::path const& b,
                  CaseSensitive   sen ) {

    fs::path a_n = lexically_normal( a );
    fs::path b_n = lexically_normal( b );

    if( sen == CaseSensitive::DEFAULT )
#ifdef __linux__
        sen = CaseSensitive::NO;
#else
        sen = CaseSensitive::YES;
#endif

    // At this point we only have either YES or NO.
    if( sen == CaseSensitive::YES )
        return (a_n == b_n);

    // Now we must do a case-insensitive comparison.  The  string
    // type used by fs::path could  vary by platform, so whenever
    // we convert path to string we need to do it like so.
    using String = fs::path::string_type;

    auto predicate = []( auto const& p1, auto const& p2) {
        return iequals( String( p1 ), String( p2 ) );
    };

    // This will iterate through path components and compare each
    // one,  so  that e.g. consecutive path separators will be ig-
    // nored.
    return equal( begin( a_n ), end( a_n ),
                  begin( b_n ), end( b_n ),
                  predicate );
}

// This function tries to  emulate  the  system  touch command in
// that it will a) create an  empty  file with current time stamp
// if one does not exist, b) will  update the time stamp on an ex-
// isting file or folder without changing contents, and  c)  will
// throw if any of the parent folders don't exist.
void touch( fs::path const& p ) {
    if( !fs::exists( p ) ) {
        ofstream o( p.string(), ios_base::out | ios_base::app );
        // This can fail if a parent folder does not exist, so we
        // really need to check.
        ASSERT( o.good(), "failed to create " << p );
        return;
    }

    // The path exists, and may be either a file  or  folder,  so
    // just update the timestamp.
    auto ltp = fs::file_time_type::clock::now();
    ZonedTimePoint ztp( ltp, tz_utc() );
    timestamp( p, ztp );
}

// It seems that the fs::remove function is supposed to not throw
// an error if the file in question does not exist, but at  least
// at the time of writing, libstdc++'s implementation does, so we
// use this wrapper to avoid throwing in that case.
void remove_if_exists( fs::path const& p ) {
    if( !fs::exists( p ) )
        return;
    fs::remove( p );
}

// We use this function  instead  of  fs::rename because it seems
// that inder MinGW the  fs::rename  does  not  behave  to  spec;
// namely, it will throw an error if the destination file already
// exists, as opposed to  overwriting  it  atomically. Also, this
// function will do  nothing  if  the  two  paths  compare  equal.
void rename( fs::path const& from, fs::path const& to ) {

    if( from == to )
        return;

    // Setup a function that takes two file names, does the  move
    // (with replacement of  existing  files)  and  then  returns
    // something "true" on error.
    auto func =
#ifdef _WIN32
        []( char const* x, char const* y ) -> bool {
            return !MoveFileEx( x, y, MOVEFILE_REPLACE_EXISTING );
        };
#else
        ::rename;
#endif

    // Now do the rename and check return code.
    ASSERT( !func( from.string().c_str(), to.string().c_str() ),
        "error renaming " << from << " to " << to );
}

// This utility will rename a file only if it exists. If it  does
// not  exist  it  will  do nothing. If log is true, the function
// will  log if the file is renamed, but will not log if the file
// is not renamed. Return  value  indicates  whether the file was
// renamed. Note that the renaming will happen via the fs::rename
// method which will throw an exception if a rename is  attempted
// but fails; therefore, a value a  false returned from this func-
// tion indicates that the file did not exist.
bool rename_if_exists( fs::path const& from,
                       fs::path const& to,
                       bool log ) {

    if( !fs::exists( from ) )
        return false;

    // File exists, so we must attempt to rename it.
    if( log )
        util::log << "moving " << from << " to " << to << "\n";

    // Should throw an exception  on  failure.  Note we are using
    // util::rename  instead  of fs::rename for reasons explained
    // in the comments to that function.
    util::rename( from, to );

    return true;
}

// Unfortunately we need this  function  because the function pro-
// vided  in the filesystem library (last_write_time) seems to re-
// turn time points with different interpretations  on  different
// platforms.  At  the  time of writing, it is observed that that
// function returns a UTC  chrono  system  time  point whereas on
// Windows (under MinGW) it returns  a  time  point  representing
// local  time. So in this library we always try to call this one
// which should always return the  same  type with the same inter-
// pretation (ZonedTimePoint).
//
// NOTE: the different behavior of  this function under different
// platforms could be a bug that would eventually be  fixed,  but
// not sure.
ZonedTimePoint timestamp( fs::path const& p ) {

    auto ltp = fs::last_write_time( p );

    // In this next block we choose the time zone based on  obser-
    // vations  made  of  the  behavior  of the implementation of
    // last_write_time on the various platforms.
#ifdef __MINGW32__
    // MinGW
    auto zone = tz_local();
#else
#ifdef _WIN32
    // MSVC
    #error "not tested on MSVC; need to determine what kind of "
           "time point MSVC returns and write this code "
           "accordingly."
#else
    // *nix
    auto zone = tz_utc();
#endif
#endif

    // Now construct an absolute  time  point by interpreting the
    // result of last_write_time with  the  time zone represented
    // by `zone`.
    return ZonedTimePoint( ltp, zone );
}

// Set  timestamp;  the  various  platforms'  implementations  of
// last_write_time for *setting* timestamps  seem  to  agree,  so
// this one just forwards the call to last_write_time.
void timestamp( fs::path const& p, ZonedTimePoint const& ztp ) {
#ifdef _WIN32
    // Somehow on the MinGW  implementation  (or  maybe its a Win-
    // dows thing, not sure) the last_write_time  function  takes
    // the time that we give it  and  interprets it as local time,
    // as  opposed  to  Linux which interprets it as UTC time, so
    // when we are converting the zoned time  to  local  time  we
    // must do so with different offsets.
    auto off = tz_local();
#else
    auto off = tz_utc();
#endif
    // It seems that all the platforms  accept  a  chrono  system
    // time  point  and interpret it as UTC time when setting the
    // timestamp.
    fs::last_write_time( p, ztp.to_local( off ) );
}

} // util
