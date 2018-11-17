/****************************************************************
* String utilities
****************************************************************/
#pragma once

#include "base-util/datetime.hpp"
#include "base-util/error.hpp"
#include "base-util/misc.hpp"
#include "base-util/types.hpp"

#include <cctype>
#include <experimental/filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace fs = std::experimental::filesystem;

namespace util {

// Returns true if s contains what.
bool contains( std::string_view s, std::string_view what );

// Returns true if s starts with what.
bool starts_with( std::string_view s, std::string_view what );

// Returns true if s ends with what.
bool ends_with( std::string_view s, std::string_view what );

// Case-insensitive comparison. This is intended to work for both
// char strings and wchar strings.
template<typename StringT>
bool iequals( StringT const& s1, StringT const& s2 ) {
    // This check is for efficiency.
    if( s1.size() != s2.size() )
        return false;

    auto predicate = []( auto l, auto r ) {
        if constexpr ( sizeof( StringT ) == 1 )
            // This is until [bugprone-suspicious-semicolon] gets
            // fixed (should be shortly) to handle constexpr if's.
            // NOLINTNEXTLINE( bugprone-suspicious-semicolon )
            return (std::tolower( l ) == std::tolower( r ));
        int l_i( l ), r_i( r );
        constexpr int signed_byte_max{127};
        if( l_i > signed_byte_max || r_i > signed_byte_max )
            // not sure how to make higher-order chars  lower-
            // case, so just compare them.
            return (l_i == r_i);
        return (tolower( l_i ) == tolower( r_i ));
    };

    return std::equal( std::begin( s1 ), std::end( s1 ),
                       std::begin( s2 ), std::end( s2 ),
                       predicate );
}

// This  will  intersperse  `what` into the vector of strings and
// join the result. It  will  attempt  to compute require reserve
// space before hand to minimize memory allocations.
template<typename T>
std::string join( std::vector<T> const& v,
                  std::string_view      what ) {

    if( !v.size() ) return {};
    // First attempt to compute how much space we need, which  we
    // should be able to do exactly.
    size_t total = 0;
    for( auto const& e : v )
        total += e.size();
    total += what.size()*(v.size() - 1); // v.size() > 0 always
    // Now construct the result (reserve +1 for good measure).
    std::string res; res.reserve( total+1 );
    bool first = true;
    for( auto const& e : v ) {
        if( !first )
            res += what;
        res += e;
        first = false;
    }
    // Just to make sure  we  did  the  calculation right; if not,
    // then we might pay extra in memory allocations.
    ASSERT( res.size() == total, "res.size() == " << res.size() <<
                                 " and total == " << total );
    return res;
}

// Strip all blank space off of a string view and return
// a new one.
std::string_view strip( std::string_view sv );

// Split a string on a character.
std::vector<std::string_view>
split( std::string_view sv, char c );

// Split a string on any character from the list. NOTE: this does
// not split on the `chars` string as a whole, it splits on any
// of the individual characters in the `chars`.
std::vector<std::string_view>
split_on_any( std::string_view sv, std::string_view chars );

// Split a string, strip all elements, and remove empty strings
// from result.
std::vector<std::string_view>
split_strip( std::string_view sv, char c );

// Split a string, strip all elements, and remove empty strings
// from result. NOTE: this does not split on the `chars` string
// as a whole, it splits on any of the individual characters in
// the `chars`.
std::vector<std::string_view> split_strip_any(
        std::string_view sv, std::string_view chars );

using IsStrOkFunc = std::function<bool( std::string_view )>;

// Will wrap the text using the is_ok callback. The callback
// should return true if the string given to it has an acceptible
// length. It is assumed that if the function returns false for a
// given string that it will also return false for all strings
// longer than it, and conversely for `true`. Thus the
// wrap_text_fn function will return a vector of lines such that
// each line would case the is_ok function to return true. The
// exception to this is when a word is itself "too long", in
// which case it will be put on its own line anyway.
//
// The `text` parameter may contain any manner of spaces, tabs,
// or newlines between words, and these will all be stripped away
// (not retained in result).
std::vector<std::string> wrap_text_fn( std::string_view text,
                                       IsStrOkFunc const& is_ok );

// Wraps text such that each resulting line will be <= to the
// max_length. The exception is if a word is itself "too long" in
// which case it will be put on its own line anyway.
std::vector<std::string> wrap_text( std::string_view text,
                                    int max_length );

// Convert element type.
std::vector<std::string>
to_strings( std::vector<std::string_view> const& svs );

// Convert string to path
fs::path to_path( std::string_view sv );

// Convert element type.
std::vector<fs::path>
to_paths( std::vector<std::string> const& ss );

/****************************************************************
* To-String utilities
*
* util::to_string  family of overloaded functions are intended so
* that  a  user  can call them on any commonly-used type and they
* will return a sensibly  formatted result. Unlike std::to_string
* these overloads work on  various  containers  as  well, such as
* vectors and tuples. For simple  numeric  types  util::to_string
* delegates  to  std::to_string.  If  all else fails, the default
* overload  attempts  to use a string stream to do the conversion.
*
* See the special note below  on  the  std::string  overload.  In
* short, Whenever the to_string methods  convert a string (or any
* string-like entity) to a string, they will insert quotes in the
* string itself.
****************************************************************/
template<typename T>
std::string to_string( T const& /*arg*/ );

// NOTE: This puts single quotes around the character!
template<>
std::string to_string<char>( char const& c );

// NOTE: These puts quotes around the string! The reason for this
// behavior  is that we want to try to perform the to_string oper-
// ation  (in general) such that it has some degree of reversibil-
// ity. For example, converting  the  integer  55  and the string
// "55" to strings should yield different  results so that we can
// distinguish the types from the string representations  (and/or
// convert  back, at least approximately). So therefore, whenever
// the  to_string methods convert a already-string-like entity to
// a string, it will insert quotes in the string itself.
template<>
std::string to_string<std::string>( std::string const& s );
template<>
std::string to_string<std::string_view>(
        std::string_view const& s );

// NOTE:  This  puts  quotes around the string! Also, it is not a
// template  specialization  because  for  some reason gcc always
// wants to select the version for ints/floats below  instead  of
// this one when we give it string literals (i.e., type deduction
// is not doing what we want). But  having this one causes gcc to
// select it when we give it a string literal.
std::string to_string( char const* s );

// Note two important things about this function: 1) it will will
// force the string to be converted to a std::string  by  calling
// its string() member function,  despite  the  fact that on some
// platforms (e.g. Windows) paths are stored internally in  UTF16.
// Also, it will put quotes around it. To convert  a  path  to  a
// string without quotes use the  path's  string() method (or one
// of its variants).
template<>
std::string to_string<fs::path>( fs::path const& p );

// Simply delegate to the wrapped type.
template<typename T>
std::string to_string( Ref<T> const& rw );

// Not  sure if this one is also needed, but doesn't seem to hurt.
template<typename T>
std::string to_string( CRef<T> const& rw );

template<typename T>
std::string to_string( std::optional<T> const& opt );

// Will do JSON-like notation. E.g. (1,2,3)
template<typename... Args>
std::string to_string( std::tuple<Args...> const& tp );

// This function exists for the purpose of  having  the  compiler
// deduce the Indexes variadic integer arguments that we can then
// use to index the tuple; it probably is not useful to call this
// method  directly  (it is called by to_string). Was not able to
// find a more elegant way of unpacking an arbitrary tuple passed
// in as an argument apart from using  this  helper  function  in-
// volving the index_sequence.
template<typename Tuple, size_t... Indexes>
StrVec tuple_elems_to_string(
        Tuple const& tp,
        std::index_sequence<Indexes...> /*unused*/ );

// This function exists for the purpose of  having  the  compiler
// deduce the Indexes variadic integer arguments that we can then
// use  to  index  the variant; it probably is not useful to call
// this method directly (it is called by to_string).
template<typename Variant, size_t... Indexes>
std::string variant_elems_to_string(
        Variant const& v,
        std::index_sequence<Indexes...> /*unused*/ );

template<typename... Args>
std::string to_string( std::variant<Args...> const& v );

template<>
std::string to_string<Error>( Error const& e );

// Will do JSON-like notation. E.g. (1,"hello")
template<typename U, typename V>
std::string to_string( std::pair<U, V> const& p );

// Prints in JSON style notation. E.g. [1,2,3]
template<typename T>
std::string to_string( std::vector<T> const& v );

// Will output a local time with format:
//
//   2018-01-15 21:30:01.396823389
//
// where there is no information  about  time  zone assumed or at-
// tached to the result.
template<>
std::string to_string( SysTimePoint const& p );

// Will output an absolute time with format:
//
//   2018-01-15 21:30:01.396823389+0000
//
// where the date and time are adjusted so as to output it in the
// UTC time zone (hence the +0000 at the end).
template<>
std::string to_string( ZonedTimePoint const& p );

template<typename T>
std::ostream& operator<<( std::ostream&         out,
                          std::vector<T> const& v );

template<typename U, typename V>
std::ostream& operator<<( std::ostream&          out,
                          std::pair<U, V> const& p );

/****************************************************************
* From-String utilities
****************************************************************/

constexpr int default_base{10}; // base 10 is decimal

// This is to replace std::stoi -- it will enforce that the input
// string is not empty and  that  the parsing consumes the entire
// string.
int stoi( std::string const& s, int base = default_base );

} // namespace util

// Implementations of template  function  bodies  in  here. We do
// this  not only for organizational purposes, but in order for a
// to_string method to be able to call any other to_string method
// (say, if we have a tuple nested within a tuple) then must  all
// be declared first before the function bodies  are  encountered.
#include "string.inl"
