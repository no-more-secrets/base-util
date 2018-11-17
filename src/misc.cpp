/****************************************************************
* Utilities
****************************************************************/
#include "base-util/misc.hpp"

using namespace std;

namespace util {

// Given a range size and chunk  size  this  will  partition  the
// range into equal sized chunks of size chunk_size with the last
// chunk possibly being smaller than  chunk_size. The result will
// be a list of pairs, each pair  describing a chunk as a pair of
// offsets  each  representing the distance from the start of the
// range to the (beginning, end) of the chunk, and where the  end
// is a one-passed-the-end marker.
//
//   E.g.: chunks( 7, 2 ) == [(0,2),(2,4),(4,6),(6,7)]
//
// The resulting offsets can be added to iterators to index  into
// containers.
PairVec<size_t, size_t> chunks( size_t size,
                                size_t chunk_size ) {
    if( size == 0 )
        return {};

    ASSERT( chunk_size > 0,
           "if size > 0 then chunk_size must be > 0" );

    PairVec<size_t, size_t> res;
    res.reserve( (chunk_size / size) + 1);

    size_t start = 0, end = chunk_size;

    while( start < size ) {
        if( end > size )
            end = size;
        ASSERT_( start < end && start < size && end <= size );
        res.emplace_back( pair{ start, end } );
        start += chunk_size;
        end   += chunk_size;
    }

    ASSERT_( start >= size );
    ASSERT_( end   >= size );

    return res;
}


} // util
