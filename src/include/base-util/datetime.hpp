/****************************************************************
* Date/Time Utilities
****************************************************************/
#pragma once

#include "base-util/types.hpp"

#include <chrono>
#include <string>

namespace util {

// We  use  seconds  as the most general way to represent the dif-
// ference  between  two time zones at a given point in time, and
// use this typedef for readability.  The  sign  of a TZOffset is
// such  that  it  should  be  added  to a UTC time to obtain the
// target  time  zone.  If  it is zero, that represents UTC. Note
// that values of this type should not be cached because they can
// change from day to day as e.g. daylight savings time starts or
// stops.
using TZOffset = std::chrono::seconds;

// Return  the  offset in seconds from the local time zone to UTC.
// Note that the result of this  function will be memoized for ef-
// ficiency, so it should not be called from a process that  runs
// for longer than a day on the day when  daylight  savings  time
// changes (seems to to be a big deal in practice).
TZOffset tz_local();

// This one is to enable readability.
inline TZOffset tz_utc() { return TZOffset( 0 ); }

// Returns  a string representation of the offset between UTC and
// the  time zone descibed by the given TZOffset. The string will
// be in the format (+/-)hhmm, e.g. "-0500" for New York, "+0000"
// for UTC. NOTE: the reason  that  we  are implementing this our-
// selves is because  it  seems  that  the  strftime (and related
// methods) are not able to correctly emit this string on Windows
// under MinGW, which they  do  on  Linux  with  the %z formatter.
std::string tz_hhmm( TZOffset off = tz_local() );

/****************************************************************
* Represenations of times
****************************************************************/
// Introduction:
//
// In this library, by default, we measure points in time  as  du-
// rations of time since an Epoch time of Jan 1  1970,  but  this
// Epoch time carries no time zone  qualifier  -- time zone is un-
// specified. Even such types as  time_t or chrono time_point are
// interpteted this way. As such, they do not specify an absolute
// point  in time without being supplemented by a time zone quali-
// fier. We refer to these  time  measures as "local Epoch time."
// We can use any time type as a local Epoch time, such as
// time_t, chrono seconds, chrono  time  points,  chrono  minutes,
// etc.
//
// There  is  only one type that is used to represent an absolute
// time, and that is the zt_point ("zoned time point"). See below
// for more info on that class. Essentially, it is just a trivial
// wrapper around some local Epoch  time  that exists in order to
// force the user to be explicit about the time zone with respect
// to which the local Epoch time is interpreted.

// Zoned Time Point. This is just  a  trivial  wrapper  around  a
// time_point  that tags it has having an interpretation of refer-
// ring  to an absolute point in time. An object of this type rep-
// resents an explicit expression on the part of the instantiator
// as to the precise definition of the epoch time with respect to
// which  the  wrapped time_point is understood. Of importance is
// that it does not allow construction or conversion to  or  from
// the  underlying chrono time point without the involvement of a
// time zone offset that must be specified by the caller.
template<typename DurationT>
struct zt_point {

    DurationT tp;

    // Delete any constructors that don't force user to specify a
    // a time zone with which to interpret the time point. We  ex-
    // plicitely  delete  these so that they will cause the appro-
    // priate diagnostic messages to appear to the user.
    zt_point() = delete;
    zt_point( DurationT const& ) = delete;

    // Construct a zoned (absolute) time point given a local time
    // time point and its offset from UTC.
    zt_point( DurationT const& in_tp, TZOffset off )
        : tp( in_tp - off ) {}

    DurationT to_local( TZOffset off ) const
        { return tp + off; }

    bool operator==( zt_point<DurationT> const& rhs ) const
        { return tp == rhs.tp; }

    bool operator<( zt_point<DurationT> const& rhs ) const
        { return tp < rhs.tp; }

    bool operator>( zt_point<DurationT> const& rhs ) const
        { return tp > rhs.tp; }
};

// Standard/default specialization used in this library.
using ZonedTimePoint = zt_point<SysTimePoint>;

/****************************************************************
* Time formatting
****************************************************************/
// Among the time formatting functions that we  have  below,  the
// only formatting function that will emit time  zone  qualifiers
// when  formatting  times  is  the  overload that takes a zoned_-
// time_point. All of the other overloads will format a time with
// as  much  precision  as that type allows, but will always stop
// short of attaching a time zone qualifier.

// To start off, we  explicitly  delete  the  overload that takes
// time_t  because  we want to discourage using it in general. If
// you need to format a time_t then what you should do is  explic-
// itly construct a chrono::seconds  and  call  the next overload.
std::string fmt_time( time_t time ) = delete;

// Formats  a  local  epoch  time specified in seconds in the fol-
// lowing format: "2018-01-15 20:52:48". Strings of this form can
// be compared lexicographically for he  purposes of comparing by
// time ordering.
std::string fmt_time( std::chrono::seconds time );

// Formats a local epoch time represented by a system clock  time
// point in  the  format:  "2018-01-15  20:52:48.421397398".  The
// output will always be precisely 29  characters  long;  if  the
// time point given to the function does  not  have  nano  second
// precision then latter digits may just be  padded  with  zeroes.
// Note that strings of  this  form  can be compared lexicographi-
// cally to compare ordering.
std::string fmt_time( SysTimePoint const& p );

// Formats a zoned time point by first adjusting its duration  ac-
// cording  to the time zone offset given, then formatting the du-
// ration type as a local epoch time  using one of the other fmt_-
// time functions, then finally attaching a time  zone  qualifier.
// The length of the output depends on the duration type on which
// zt_point is parametrized,  since  that  depends which fmt_time
// function  gets called to format that duration. However, if the
// DurationT  is a chrono system_clock time point then the output
// will always be precisely 34 characters  long and will have the
// formats, for example:
//
//     2018-01-15 15:52:48.421397398-0500
//     2018-01-15 20:52:48.421397398+0000
//
// NOTE: these strings cannot be  compared  lexicographically  un-
// less the timezones are the same.
template<typename DurationT>
std::string fmt_time( zt_point<DurationT> const& p,
                      TZOffset off = tz_local() ) {

    // Which  overload  of fmt_time gets called depends on the du-
    // ration type.
    return fmt_time( p.to_local( off ) ) + tz_hhmm( off );
}

} // namespace util

// For  convenience,  dump  this  two  into  the global namespace.
using util::ZonedTimePoint;
