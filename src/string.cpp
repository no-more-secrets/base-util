/****************************************************************
* String utilities
****************************************************************/
#include "base-util/datetime.hpp"
#include "base-util/macros.hpp"
#include "base-util/string.hpp"

#include <algorithm>

using namespace std;

namespace util {

// Returns true if s contains what.
bool contains( string_view s, string_view what ) {
    return s.find( what ) != string_view::npos;
}

// Returns true if s starts with w.
bool starts_with( string_view s, string_view w ) {
    return mismatch( begin( s ), end( s ),
                     begin( w ), end( w) ).second == end( w );
}

// Returns true if s ends with what.
bool ends_with( string_view s, string_view w ) {
    return mismatch( rbegin( s ), rend( s ),
                     rbegin( w ), rend( w ) ).second == rend( w );
}

// Strip all blank space off of  a  string  view and return a new
// one.
string_view strip( string_view sv ) {

    auto start = sv.find_first_not_of( " \t\n\r" );
    sv.remove_prefix( min( start, sv.size() ) );
    // Must do this here because sv is being mutated.
    auto last  = sv.find_last_not_of( " \t\n\r" )+1;
    sv.remove_suffix( sv.size() - min( last, sv.size() ) );
    return sv;
}

// Split a string on any character from the list. NOTE: this does
// not split on the `chars` string as a whole, it splits on any
// of the individual characters in the `chars`.
vector<string_view>
split_on_any( string_view sv, string_view chars ) {
    vector<string_view> res;
    while( true ) {
        auto next = sv.find_first_of( chars );
        if( next == string_view::npos ) break;
        res.push_back( sv.substr( 0, next ) );
        // Remove what we just added, plus c
        sv.remove_prefix( next+1 );
    }
    res.push_back( sv );
    return res;
}

// Split a string on a character.
vector<string_view> split( string_view sv, char c ) {
    return split_on_any( sv, string_view( &c, 1 ) );
}

// Split  a  string, strip all elements, and remove empty strings
// from result.
vector<string_view> split_strip_any( string_view sv,
                                     string_view chars ) {
    auto res = split_on_any( sv, chars );
    transform( begin( res ), end( res ), begin( res ), strip );
    auto new_end = std::remove_if(
            begin( res ), end( res ), L( _.empty() ) );
    res.erase( new_end, end( res ) );
    return res;
}

// Split  a  string, strip all elements, and remove empty strings
// from result.
vector<string_view> split_strip( string_view sv, char c ) {
    return split_strip_any( sv, string_view( &c, 1 ) );
}

vector<string> wrap_text_fn( string_view text,
                             IsStrOkFunc const& is_ok ) {
  auto words = util::split_strip_any( text, " \n\r\t" );
  vector<string> res;
  string line;
  for( auto const& word : words ) {
    string proposed;
    proposed = line;
    if( proposed.empty() )
        proposed = word;
    else
        proposed += ' ' + string( word );

    if( is_ok( proposed ) ) {
        line = proposed;
    } else {
        if( line.empty() )
            // word on its own line.
            res.emplace_back( move( proposed ) );
        else {
            // push current line and put new word on next line.
            res.emplace_back( move( line ) );
            line = word;
        }
    }
  }
  if( !line.empty() )
      res.emplace_back( move( line ) );
  return res;
}

vector<string> wrap_text( string_view text, int max_length ) {
    return wrap_text_fn( text, [max_length]( string_view sv ) {
        return int( sv.size() ) <= max_length;
    });
}

// Convert element type.
vector<string> to_strings( vector<string_view> const& svs ) {

    vector<string> res; res.reserve( svs.size() );
    for( auto sv : svs )
        res.emplace_back( sv );
    return res;
}

// Convert string to path
fs::path to_path( string_view sv ) {
    return fs::path( sv );
}

// Convert element type.
vector<fs::path> to_paths( vector<string> const& ss ) {

    vector<fs::path> res; res.reserve( ss.size() );
    for( auto const& s : ss )
        res.emplace_back( s );
    return res;
}

/****************************************************************
* To-String utilities
****************************************************************/
// NOTE: These puts quotes around the string! The reason for this
// behavior  is that we want to try to perform the to_string oper-
// ation  (in general) such that it has some degree of reversibil-
// ity. For example, converting  the  integer  55  and the string
// "55" to strings should yield different  results so that we can
// distinguish the types from the  string representations (and or
// convert  back, at least approximately). So therefore, whenever
// the  to_string methods convert a already-string-like entity to
// a string, it will insert quotes in the string itself.
string to_string( string const& s ) {
    return "\"" + s + "\"";
}
string to_string( string_view const& s ) {
    // Below may seem redundant, but we want to put quotes around
    // the string.
    return to_string( string( s ) );
}

std::string to_string( Error const& e ) { return e.msg; }

// NOTE:  This  puts  quotes around the string! Also, it is not a
// template  specialization  because  for  some reason gcc always
// wants to select the version for ints/floats below  instead  of
// this one when we give it string literals (i.e., type deduction
// is not doing what we want). But  having this one causes gcc to
// select it when we give it a string literal.
std::string to_string( char const* s ) {
    return "\"" + string( s ) + "\"";
}

// NOTE: This puts single quotes around the character!
string to_string( char const& s ) {
    string res( 1, s );
    return "'" + res + "'";
}

// Note two important things about this function: 1) it will will
// force the string to be converted to a std::string  by  calling
// its string() member function,  despite  the  fact that on some
// platforms (e.g. Windows) paths are stored internally in  UTF16.
// Also, it will put quotes around it. To convert  a  path  to  a
// string without quotes use the  path's  string() method (or one
// of its variants).
string to_string( fs::path const& p ) {
   return "\"" + p.string() + "\"";
}

// Will output a local time with format:
//
//   2018-01-15 21:30:01.396823389
//
// where there is no information  about  time  zone assumed or at-
// tached to the result.
string to_string( SysTimePoint const& p ) {
    return util::fmt_time( p );
}

// Will output an absolute time with format:
//
//   2018-01-15 21:30:01.396823389+0000
//
// where the date and time are adjusted so as to output it in the
// UTC time zone (hence the +0000 at the end).
string to_string( ZonedTimePoint const& p ) {
    return util::fmt_time( p, util::tz_utc() );
}

/****************************************************************
* From-String utilities
****************************************************************/

// This is to replace std::stoi -- it will enforce that the input
// string is not empty and  that  the parsing consumes the entire
// string.
int stoi( string const& s, int base ) {
    ASSERT( !s.empty(), "cannot convert empty string to int" );
    size_t written;
    auto res = stoi( s, &written, base );
    ASSERT( written == s.size(), "failed to parse entire string "
            << quoted( s ) << " into an integer." );
    return res;
}

} // namespace util
