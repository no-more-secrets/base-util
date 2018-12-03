/****************************************************************
* filesystem related utilities
****************************************************************/
#pragma once

#include "base-util/datetime.hpp"
#include "base-util/types.hpp"

#include <experimental/filesystem>

// Case-insensitive file system?
#if( _WIN32 || __APPLE__ )
#    define CASE_INSENSITIVE_FS() 1
#else
#    define CASE_INSENSITIVE_FS() 0
#endif

namespace fs = std::experimental::filesystem;

namespace util {

// This enum will be used to select options of  case  sensitivity
// when comparing paths.
enum class CaseSensitive { DEFAULT, YES, NO };

// This function joins two paths, but if the path on the rhs side
// qualifies as absolute than we  use  it as the result, ignoring
// the lhs. We shouldn't need this function  since  the  standard
// `/`  and  `/=` operators (which join paths) are supposed to do
// this for us, but  the  libstdc++ implementation doesn't appear
// to be working right at the moment.
fs::path slash( fs::path const& lhs, fs::path const& rhs );

// This will put the path into  normal  form  and  preserving  ab-
// solute/relative nature. Path must exist,  and will resolve sym-
// links.  The version of this function that does not require the
// file  to  exist  (and won't resolve links) is lexically_normal.
fs::path normpath( fs::path const& p );

// This  is  like normpath except that it makes the path absolute
// if it is not already (if it is relative, it  is  assumed  rela-
// tive to CWD. The version of this function  that  does  not  re-
// quire the file to exist  (and  won't resolve links) is lexical-
// ly_absolute.
fs::path absnormpath( fs::path const& p );

// Put  a  path into normal form without regard to whether or not
// it  exists  (and  do so without touching the filesystem at all.
// The C++17 filesystem library has this  method as a member func-
// tion of the path class, but it has not yet been implemented in
// gcc at the time of this writing.  Once gcc 8 is released, then
// this  function  can  be  deleted and the usage of it can be re-
// placed with p.lexicaly_normal().
fs::path lexically_normal( fs::path const& p );

// This  is  like  absnormpath  except that it will not query the
// file system. If the path p is relative  then  it  will  be  ap-
// pended  to  the  CWD  and  the result normalized using lexical-
// ly_normal. Result is an absolute path in  normal  for  without
// links resolved and which may not exist.
fs::path lexically_absolute( fs::path const& p );

// Implemenation of the  lexically_relative  function.  Find  the
// relative  path between the given path and base path without re-
// gard  to  whether or not it exists (and do so without touching
// the  filesystem  at  all; hence we also don't follow symlinks).
// The C++17 filesystem library has this  method as a member func-
// tion of the path class, but it has not yet been implemented in
// gcc at the time of this writing.  Once gcc 8 is released, then
// this function can probably be deleted and the usage of it  can
// be replaced with p.lexicaly_relative().
fs::path lexically_relative( fs::path const& p,
                             fs::path const& base );

// Flip any backslashes to forward slashes.
std::string fwd_slashes( std::string_view in );

// Flip any backslashes to forward slashes.
StrVec fwd_slashes( StrVec const& v );

// Flip any forward slashes to back slashes.
std::string back_slashes( std::string_view in );

// Flip any forward slashes to back slashes.
StrVec back_slashes( StrVec const& v );

// Constructs  a path from a pair of iterators to path components.
// Didn't see this available in  the  standard,  but  could  have
// missed it. The name is meant  for  it  to look like a construc-
// tor, even though it isn't.
fs::path path_( fs::path::const_iterator b,
                fs::path::const_iterator e );

// The purpose of this function  is  to  compare two paths lexico-
// graphically  with the option of case insensitivity. On Windows,
// the comparison will  default  to  being  case  insensitive. On
// Linux it will default to case-sensitive, but  in  either  case,
// this can be overridden.
bool path_equals( fs::path const& a,
                  fs::path const& b,
                  CaseSensitive   sen = CaseSensitive::DEFAULT );

// This function tries to  emulate  the  system  touch command in
// that it will a) create an  empty  file with current time stamp
// if one does not exist, b) will  update the time stamp on an ex-
// isting file or folder without changing contents, and  c)  will
// throw if any of the parent folders don't exist.
void touch( fs::path const& p );

// It seems that the fs::remove function is supposed to not throw
// an error if the file in question does not exist, but at  least
// at the time of writing, libstdc++'s implementation does, so we
// use this wrapper to avoid throwing in that case.
void remove_if_exists( fs::path const& p );

// We use this function  instead  of  fs::rename because it seems
// that inder MinGW the  fs::rename  does  not  behave  to  spec;
// namely, it will throw an error if the destination file already
// exists, as opposed to  overwriting  it  atomically. Also, this
// function will do  nothing  if  the  two  paths  compare  equal.
void rename( fs::path const& from, fs::path const& to );

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
                       bool log = false );

// Unfortunately we need this  function  because the function pro-
// vided  in the filesystem library (last_write_time) seems to re-
// turn time points with different interpretations  on  different
// platforms.  At  the  time of writing, it is observed that that
// function returns a UTC  chrono  system  time  point whereas on
// Windows (under MinGW) it returns  a  time  point  representing
// local  time. So in this library we always try to call this one
// which should always return the  same  type with the same inter-
// pretation (ZonedTimePointFS).
//
// NOTE: the different behavior of  this function under different
// platforms could be a bug that would eventually be  fixed,  but
// not sure.
ZonedTimePointFS timestamp( fs::path const& p );

// Set  timestamp;  the  various  platforms'  implementations  of
// last_write_time for *setting* timestamps  seem  to  agree,  so
// this one just forwards the call to last_write_time.
void timestamp( fs::path const& p, ZonedTimePointFS const& ztp );

} // namespace util

// It seems that, at the time of this writing, std::hash has  not
// been given a specialization for fs::path, so  we  will  inject
// one into the std namespace so that we can use it as the key in
// unordered maps. This is simple to do  because  the  filesystem
// library does already provide the hash_value method which  does
// the actual hashing of a path. Note that two paths' hashes will
// be  equal when equality is satisfied, so e.g. A//B is the same
// as A/B. If one is  added  in  the  future  then this may start
// causing compilation failures, in which  case it should just be
// deleted.
namespace std {

    template<> struct hash<fs::path> {
        auto operator()( fs::path const& p ) const noexcept {
            return fs::hash_value( p );
        }
    };

} // namespace std
