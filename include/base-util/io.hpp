/****************************************************************
* IO related utilities
****************************************************************/
#pragma once

#include "base-util/fs.hpp"
#include "base-util/types.hpp"

namespace util {

// Read  a  file in its entirety into a vector of chars. This may
// be a bit less efficient than possible because the vector, when
// created, will initialize all of its bytes  to  zero  which  we
// don't actually need.
std::vector<char> read_file( fs::path const& p );

// Open the file, truncate it,  and  write  given  vector  to  it.
void write_file( fs::path const& p, std::vector<char> const& v );

// We should not need this function  because  the  filesystem  li-
// brary provides fs::copy_file which would ideally be better  to
// use. However, it was observed at  the  time  of  this  writine
// (with  gcc  7.2)  that  the fs::copy_file method was not faith-
// fully copying files  on  Windows  that  were  created on Linux.
// Specifically, it was observed that,  copying  a text file with
// Linux  line endings on Windows resulted in a new file with Win-
// dows line endings, which is  not  desired.  Hence we have this
// function which will copy the file in  binary  mode  faithfully.
void copy_file( fs::path const& from, fs::path const& to );

// Read a text file into a string in its entirety.
std::string read_file_str( fs::path const& p );

// Read  a text file into a string in its entirety and then split
// it into lines.
StrVec read_file_lines( fs::path const& p );

// Take a path whose last  component  (file name) contains a glob
// expression and  return  results  by  searching  the  directory
// listing for all files (and folders if flag is true) that match
// the glob pattern. Only *  and  ?  are supported, and those spe-
// cial  characters  can only appear in the file name of the path.
// The filename (with wildcard characters)  must match the entire
// file name from start to finish. If one of the folders  in  the
// path  does  not  exist, an exception is thrown. If the path is
// relative then it is relative to CWD. Paths returned will  have
// their  absolute/relative  nature  preserved  according  to the
// input  p.  Also,  if input path is empty, it will return empty.
// Note that, unlike some shells' globbing behavior, the wildcard
// function does not give  any  special  treatment to files whose
// names begin with a dot ("hidden files" on Linux).
PathVec wildcard( fs::path const& p, bool with_folders = true );

} // namespace std
