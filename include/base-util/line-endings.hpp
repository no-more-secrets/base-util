/****************************************************************
* Utilities for handling line endings
****************************************************************/
#pragma once

#include "base-util/fs.hpp"
#include "base-util/util.hpp"

#include <vector>

namespace util {

// This function will simply remove and 0x0D characters from  the
// input  (mutating  the argument). The new size of the container
// will therefore always be less or equal to  its  original  size.
template<typename Container>
void dos2unix( Container& c,
    std::enable_if_t<std::is_same_v<
        typename Container::iterator::value_type, char>>*
            /*unused*/ = nullptr ) {
    util::remove_if( c, L( _ == 0x0d ) );
}

// This  function  will  simply search for any 0x0A character and
// insert a 0x0D character before it unless there is already such
// a character before it in which case it will be left alone. The
// new size of the container will  therefore always be greater or
// equal  to  that of the original. Note that this function sends
// the altered contents to a new container and  then  moves  into
// the input container, effectively replacing the contents of the
// input. NOTE: if the input container  does  not  contain  valid
// unix (e.g., if it contains solitary CR's) then the output  may
// not contain valid DOS line endings.
template<typename Container>
void unix2dos( Container& in,
    std::enable_if_t<std::is_same_v<
        typename Container::iterator::value_type, char>>*
            /*unused*/ = nullptr ) {

    // Some quick experiments suggest that the average ascii text
    // file will grow about 3-4% in size after this operation, so
    // we will reserve an additional 5%  to  try to avoid an addi-
    // tional allocation.  However,  this  is  of  course  just a
    // heuristic which will not be optimal or even helpful in all
    // cases.
    constexpr size_t denominator{20}; // 1/20 == 5%
    size_t estimate = in.size() + (in.size() / denominator);
    Container out; out.reserve( estimate );

    constexpr char LF = 0x0A;
    constexpr char CR = 0x0D;

    for( auto i = std::begin( in ); i != std::end( in ); ++i ) {
        if( *i == LF ) {
            // We have encountered an  LF character, so therefore
            // we need to insert a CR character before  this  one
            // either if we're at the beginning or if there isn't
            // already a CR character before this one.
            if( i == std::begin( in ) || *(i-1) != CR )
                out.push_back( CR );
        }
        out.push_back( *i );
    }

    // Replace input with new container.
    in = std::move( out );
}

// Open the given path and edit  it to remove all 0x0D characters.
// This  attempts to emulate the command line utility of the same
// name.  As  with  the  command,  the  `keepdate` flag indicates
// whether the timestamp on the  file should remain unchanged. By
// default, the timestamp will be  touched  if any changes to the
// file are made. Bool return value indicates  whether  file  con-
// tents were changed or not (regardless of timestamp).
bool dos2unix( fs::path const& p, bool keepdate = false );

// Open  the given path and edit it to change LF to CRLF. This at-
// tempts to emulate the command line utility of  the  same  name.
// As with the command, the `keepdate` flag indicates whether the
// timestamp on the file should remain unchanged. By default, the
// timestamp will be touched if any  changes to the file are made.
// NOTE:  if  the  input  container  does  not contain valid unix
// (e.g.,  if  it contains solitary CR's) then the output may not
// contain  valid  DOS  line endings. Bool return value indicates
// whether file contents were changed  or not (regardless of time-
// stamp).
bool unix2dos( fs::path const& p, bool keepdate = false );

}
