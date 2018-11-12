/****************************************************************
* Utilities for handling line endings
****************************************************************/
#include "base-util/line-endings.hpp"
#include "base-util/io.hpp"

using namespace std;

namespace util {

namespace {

// Functions that change line endings just take a vector  and  mu-
// tate it, which is fine for most use cases.
using Changer = void( vector<char>& );

// Will read in the contents of  the  file (which must exist) and
// apply the function to it to change line endings, then write it
// back out. The keepdate flag  indicates  whether the time stamp
// on the file should remain  unchanged.  By  default,  the  time
// stamp  will  be  touched  if  any changes to the file are made.
// Bool return value indicates whether file contents were changed
// or not (regardless of time stamp).
bool change_le( Changer* f, fs::path const& p, bool keepdate ) {

    auto v    = read_file( p );
    auto size = v.size();

    // This function should mutate the vector.
    f( v );

    // We can use the size of the  new vector relative to the old
    // size to determine whether  it  was  changed (i.e., whether
    // any line endings were changed); i.e., a line ending change
    // will leave the size unchanged if and only if there were no
    // line endings that needed to be  changed, in which case the
    // contents of  the  buffer  in  general  will  be  unchanged.
    if( v.size() == size )
        return false;

    // Get the pre-modification time stamp on the file in case we
    // need to restore it (i.e., keepdate == true).
    auto t0 = util::timestamp( p );

    write_file( p, v ); // Will always touch time stamp

    if( keepdate )
        // restore time stamp
        util::timestamp( p, t0 );

    return true; // true means that we changed the file contents.
}

} // anonymous namespace

// Open the given path and edit  it to remove all 0x0D characters.
// This  attempts to emulate the command line utility of the same
// name.  As  with  the  command,  the  `keepdate` flag indicates
// whether the time stamp on the file should remain unchanged. By
// default, the time stamp will be  touched if any changes to the
// file are made. Bool return value indicates  whether  file  con-
// tents were changed or not (regardless of time stamp).
bool dos2unix( fs::path const& p, bool keepdate ) {
    auto fn = []( vector<char>& _ ) { dos2unix( _ ); };
    return change_le( fn, p, keepdate );
}

// Open  the given path and edit it to change LF to CRLF. This at-
// tempts to emulate the command line utility of  the  same  name.
// As with the command, the `keepdate` flag indicates whether the
// time stamp on the file should  remain  unchanged.  By  default,
// the time stamp will be touched if  any changes to the file are
// made. NOTE: if the input container does not contain valid unix
// (e.g.,  if  it contains solitary CR's) then the output may not
// contain  valid  DOS  line endings. Bool return value indicates
// whether file contents were changed  or not (regardless of time-
// stamp).
bool unix2dos( fs::path const& p, bool keepdate ) {
    auto fn = []( vector<char>& _ ) { unix2dos( _ ); };
    return change_le( fn, p, keepdate );
}

}
