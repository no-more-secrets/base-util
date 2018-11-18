/****************************************************************
* Text Encoding Conversion Utilities
****************************************************************/
#include "base-util/conv.hpp"

#include "base-util/io.hpp"
#include "base-util/macros.hpp"

using namespace std;

namespace conv {

// This  function takes a vector of chars that are assumed to con-
// tain ascii-encoded text and it will convert them to UTF16LE by
// simply widening each byte with a zero byte. The input is  veri-
// fied to be valid ASCII (i.e., numerical values  of  characters
// <=  127)  and  the function will throw if a violation is found
// (which might indicate the presence  of  a  non-ascii  encoding
// which  then  cannot  be  converted using this simple method of
// this  function).  Optionally  it will insert a byte order mark
// (BOM) of 0xFF 0xFE at the start of the resultant vector. Note:
// if concatentaning multiple UTF16 strings, only the initial one
// should contain a byte order mark.
//
// If the input is empty the the  function will yield an empty re-
// sult even if bom is true.
vector<char> ascii_2_utf16le( vector<char> const& v, bool bom ) {

    vector<char> res; res.reserve( (bom ? 2 : 0) + v.size()*2 );

    // Important: if the input is  empty  then return an empty re-
    // sult even if bom == true.
    if( v.empty() )
        return res;

    uint8_t const bom_1{ 0xFF }, bom_2{ 0xFE };
    if( bom )
        res.emplace_back( bom_1 ), res.emplace_back( bom_2 );

    for( auto const& c : v ) {
        ASSERT( c >= 0, "non-ascii character " << int( c ) <<
                        " found in input" );
        res.emplace_back( c ), res.emplace_back( 0 );
    }

    return res;
}

// Will call ascii_2_utf16le on a vector containing the  contents
// of  the  file;  note that byte order mark (BOM) is inserted at
// the start of the file by default. Will  throw  if  input  file
// contains any non-ascii characters.
void ascii_2_utf16le( fs::path const& p, bool bom ) {
    util::write_file( p,
            ascii_2_utf16le( util::read_file( p ), bom ) );
}

} // namespace conv
