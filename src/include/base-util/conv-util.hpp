/****************************************************************
* Text Encoding Conversion Utilities
****************************************************************/
#pragma once

#include "fs.hpp"

#include <vector>

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
std::vector<char> ascii_2_utf16le( std::vector<char> const& v,
                                   bool bom = false );

// Will call ascii_2_utf16le on a vector containing the  contents
// of  the  file;  note that byte order mark (BOM) is inserted at
// the start of the file by default. Will  throw  if  input  file
// contains any non-ascii characters.
void ascii_2_utf16le( fs::path const& p, bool bom = true );

} // namespace conv
