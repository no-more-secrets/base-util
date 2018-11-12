/****************************************************************
* StopWatch
****************************************************************/
#pragma once

#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace util {

/* This class can be  used  to  mark  start/stop times of various
 * events and to get the  durations  in  various useful forms. */
class StopWatch {

public:
    // For convenience: will start, run, stop.
    template<typename FuncT>
    void timeit( std::string_view name, FuncT func ) {
        start( name ); func(); stop( name );
    }

    // Start  the  clock for a given event name. If an event with
    // this name already exists then  it  will be overwritten and
    // any end times for it will be deleted.
    void start( std::string_view name );
    // Register an end time for an event. Will throw if there was
    // no start time for the event.
    void stop( std::string_view name );

    // Get results for an even in the given units.  If  either  a
    // start or end time for the event has  not  been  registered
    // then these will throw.
    int64_t milliseconds( std::string_view name ) const;
    int64_t seconds     ( std::string_view name ) const;
    int64_t minutes     ( std::string_view name ) const;

    // Gets the results for an event and then formats them  in  a
    // way that is most readable given the duration.
    std::string human( std::string_view name ) const;
    // Get a list of all results in human  readable  form.  First
    // element of pair is the event name and the  second  is  the
    // result of calling human() for that event.
    using result_pair = std::pair<std::string, std::string>;
    std::vector<result_pair> results() const;

private:
    using clock_type   = std::chrono::steady_clock;
    using time_point   = std::chrono::time_point<clock_type>;
    using events_timer = std::map<std::string, time_point>;

    bool event_complete( std::string_view name ) const;

    events_timer start_times;
    events_timer end_times;

};

/* This is for convenience. Will  start a timer upon construction,
 * and will stop it upon destruction and print time to stderr. */
class ScopedWatch {

public:
    explicit ScopedWatch( std::string_view title )
        : name( title ) { watch.start( name ); }

    ScopedWatch( ScopedWatch const& ) = default;
    ScopedWatch( ScopedWatch&&      ) = default;

    ScopedWatch& operator=( ScopedWatch const& ) = delete;
    ScopedWatch& operator=( ScopedWatch&& )      = delete;

    ~ScopedWatch() {
        watch.stop( name );
        // Must  go  to  cerr  here to avoid interfering with pro-
        // grams that communicate their output via stdout.
        std::cerr << name << " time: " << watch.human( name )
                  << "\n";
    }

private:
    StopWatch   watch;
    std::string name;
};

// For convenience: will start, run,  stop, and return the result
// of the function. Seems to work also for functions that  return
// void.
template<typename FuncT>
auto timeit( std::string_view name, FuncT func )
        -> decltype( func() ) {
    ScopedWatch watch( name );
    return func();
}

// Example usage:
// auto res = TIMER( "my function", f( 1, 2, 3 ) );
#define TIMEIT( name, code ) \
    util::timeit( name, [&]() { return code; } );

} // namespace util
