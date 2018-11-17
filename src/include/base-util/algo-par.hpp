/****************************************************************
* Parallel Algorithms
****************************************************************/
#pragma once

#include "base-util/error.hpp"
#include "base-util/util.hpp"

#include <algorithm>
#include <functional>
#include <thread>
#include <type_traits>
#include <vector>

namespace util::par {

// Will  return  the max number of simultaneous threads supported
// on this system. Result will always be >= 1.
int max_threads();

// Will take a vector of functions and will run  each  one  in  a
// separate  thread. The functions are expected to take no parame-
// ters  and  to  return  no values. This is a somewhat low-level
// function that should probably not be called  except  by  other
// functions in this module.
void in_parallel( std::vector<std::function<void()>> const& v );

/* Parallel map (returns  variants  to  capture  errors): apply a
 * function to elements in a range in parallel. This is being  im-
 * plemented until the parallel STL  becomes available. Note: the
 * range here expects to have a size() function. Each job
 * processes  its  own  chunk  of the array so as to minimize con-
 * tention between threads for the same memory. */
template<typename FuncT, typename InputT>
auto map_safe( FuncT                      func,
               std::vector<InputT> const& input,
               int                        jobs_in = 0 )
{
    // Number of jobs must be valid (which includes zero).
    ASSERT_( jobs_in >= 0 );

    // Interpret zero jobs as a request to use the maximum number
    // of threads available on this system.
    size_t jobs = (jobs_in == 0) ? max_threads() : jobs_in;

    // Create one thread for  each  job,  unless  the size of the
    // input is less than number of  requested jobs. jobs may end
    // up  being  zero  here, and that is ok: at this point, zero
    // jobs means that no threads will  be  spawned  and  nothing
    // will be done (unlike jobs_in == 0 which means to  use  the
    // max number of threads available).
    jobs = std::min( jobs, input.size() );

    // Get  the  underlying value type held by the range and then
    // get the type of result after calling the  function  on  it,
    // stripping away references and const.
    using Payload = std::decay_t<decltype(
        func( *std::begin( input ) )
    )>;

    // The results of calling the function will then be held in a
    // vector of variants, the  variants  being  to  contain  any
    // error  information  if  the  function  call  fails  (a.k.a.
    // throws). Copying should be  disabled  for  the elements of
    // this type, so this should  be  efficiently  moved  to  the
    // caller or, hopefully, NRVO'd.
    std::vector<Result<Payload>> outputs( input.size() );

    // One of the following functions will  be run in each thread.
    auto job = [&]( size_t job_idx ) -> void {

        // Divide up chunks so that threads don't contend for the
        // same memory.
        auto inc   = 1;
        auto size  = input.size()/jobs;
        auto start = job_idx*size;
        auto end   = (job_idx == jobs-1) ? input.size()
                                         : start+size;

        for( auto i = start; i < end; i += inc ) {
            try {
                outputs[i] = func( input[i] );
            } catch( std::exception const& e ) {
                outputs[i] = Error{ e.what() };
            } catch( ... ) {
                outputs[i] = Error{ "unknown exception" };
            }
        }
    };

    // Package each job into a void(void) function  that  we  can
    // then hand off to be executed in its own thread.
    std::vector<std::function<void()>> funcs( jobs );
    for( size_t i = 0; i < jobs; ++i )
        // Each job processes indexes from  the  input  array  by
        // starting on the index given (i), then  jumping  by  in-
        // tervals of size jobs.
        funcs[i] = [&job, i](){ return job( i ); };

    // Run the functions in parallel. Each function will run in a
    // single thread and will  handle  a  chunk  of the input ele-
    // ments, storing output in  the  outputs  array which it has
    // captured by reference.
    in_parallel( funcs );

    return outputs;
}

/* Parallel map (throws on error): apply a function  to  elements
 * in a range in parallel. This is being  implemented  until  the
 * parallel STL becomes available.  Note:  the range here expects
 * to  have  a size() function. Each job processes it's own chunk
 * of the array so to minimize contention between threads for the
 * same memory. */
template<typename FuncT, typename InputT>
auto map( FuncT                      func,
          std::vector<InputT> const& input,
          int                        jobs_in = 0 )
{
    // Number of jobs must be valid (which includes zero).
    ASSERT_( jobs_in >= 0 );

    // Interpret zero jobs as a request to use the maximum number
    // of threads available on this system.
    size_t jobs = (jobs_in == 0) ? max_threads() : jobs_in;

    // Create one thread for  each  job,  unless  the size of the
    // input is less than number of  requested jobs. jobs may end
    // up  being  zero  here, and that is ok: at this point, zero
    // jobs means that no threads will  be  spawned  and  nothing
    // will be done (unlike jobs_in == 0 which means to  use  the
    // max number of threads available).
    jobs = std::min( jobs, input.size() );

    // Get  the  underlying value type held by the range and then
    // get the type of result after calling the  function  on  it,
    // stripping away references and const.
    using Payload = std::decay_t<decltype(
        func( *std::begin( input ) )
    )>;

    // The results of calling  the  function.  Elements  will  be
    // moved into place or hopefully NRVO'd; ditto for the vector
    // as a whole when returned to caller
    std::vector<Payload> outputs( input.size() );

    // This will hold the success/failure result from each
    // thread.  nullopt means success, while a string means error.
    std::vector<std::optional<std::string>> results( jobs );

    // One of the following functions will  be run in each thread.
    auto job = [&]( size_t job_idx ) -> void {

        // Divide up chunks so that threads don't contend for the
        // same memory.
        auto inc   = 1;
        auto size  = input.size()/jobs;
        auto start = job_idx*size;
        auto end   = (job_idx == jobs-1) ? input.size()
                                         : start+size;

        for( auto i = start; i < end; i += inc ) {
            try {
                outputs[i] = func( input[i] );
                // If the function did  not  throw  an  exception
                // then continue.
                continue;
            } catch( std::exception const& e ) {
                results[job_idx] = e.what();
            } catch( ... ) {
                results[job_idx] = "unknown exception";
            }
            return; // error happened
        }
    };

    // Package each job into a void(void) function  that  we  can
    // then hand off to be executed in its own thread.
    std::vector<std::function<void()>> funcs( jobs );
    for( size_t i = 0; i < jobs; ++i )
        // Each job processes indexes from  the  input  array  by
        // starting on the index given (i), then  jumping  by  in-
        // tervals of size jobs.
        funcs[i] = [&job, i](){ return job( i ); };

    // Run the functions in parallel. Each function will run in a
    // single thread and will  handle  a  chunk  of the input ele-
    // ments, storing output in  the  outputs  array which it has
    // captured by reference.
    in_parallel( funcs );

    // Check  each  thread's results for any errors, and re-throw
    // the first one we find. !r means success,  and  the  ASSERT
    // macro  is  not supposed to evaluate the second argument un-
    // less the first one is false.
    for( auto const& r : results ) ASSERT( !r, *r );

    return outputs;
}

/* Parallel for_each: apply a function to  elements in a range in
 * parallel. This is being implemented  until the parallel STL be-
 * comes available. Note: the range here expects to have a size()
 * function. Each job processes its own chunk of the array so  as
 * to minimize contention  between  threads  for  the same memory.
 * Unlike  map_par,  this  function  does  not  retain the return
 * values of the function calls; i.e.,  the  functions  are  only
 * called  for  their effects. Nevertheless it will still monitor
 * the threads for exceptions, and, if any thread throws an error
 * it will be rethrown after all threads are  joined  along  with
 * original message (if  multiple  threads  throw exceptions then
 * only the first exception message encountered will be returned;
 * there is no point in trying to include all  of  them,  because
 * even  a single thread will stop processing items as soon as it
 * encounters an error). */
template<typename FuncT, typename InputT>
void for_each( std::vector<InputT> const& input,
               FuncT                      func,
               int                        jobs_in = 0 )
{
    // Number of jobs must be valid (which includes zero).
    ASSERT_( jobs_in >= 0 );

    // Interpret zero jobs as a request to use the maximum number
    // of threads available on this system.
    size_t jobs = (jobs_in == 0) ? max_threads() : jobs_in;

    // Create one thread for  each  job,  unless  the size of the
    // input is less than number of  requested jobs. jobs may end
    // up  being  zero  here, and that is ok: at this point, zero
    // jobs means that no threads will  be  spawned  and  nothing
    // will be done (unlike jobs_in == 0 which means to  use  the
    // max number of threads available).
    jobs = std::min( jobs, input.size() );

    // This will hold the success/failure result from each
    // thread.  nullopt means success, while a string means error.
    std::vector<std::optional<std::string>> results( jobs );

    // One of the following functions will  be run in each thread.
    auto job = [&]( size_t job_idx ) -> void {

        // Divide up chunks so that threads don't contend for the
        // same memory.
        auto inc   = 1;
        auto size  = input.size()/jobs;
        auto start = job_idx*size;
        auto end   = (job_idx == jobs-1) ? input.size()
                                         : start+size;

        for( auto i = start; i < end; i += inc ) {
            try {
                func( input[i] );
                // If the function was successfull then leave the
                // corresponding result as a nullopt (which means
                // success) and continue in the loop.
                continue;
            } catch( std::exception const& e ) {
                results[job_idx] = e.what();
            } catch( ... ) {
                results[job_idx] = "unknown exception";
            }
            return; // error happened
        }
    };

    // Package each job into a void(void) function  that  we  can
    // then hand off to be executed in its own thread.
    std::vector<std::function<void()>> funcs( jobs );
    for( size_t i = 0; i < jobs; ++i )
        // Each job processes indexes from  the  input  array  by
        // starting on the index given (i), then  jumping  by  in-
        // tervals of size jobs.
        funcs[i] = [&job, i](){ return job( i ); };

    // Run the functions in parallel. Each function will run in a
    // single thread and will  handle  a  chunk  of the input ele-
    // ments, storing success/failure in the results array  which
    // it has captured by reference.
    in_parallel( funcs );

    // Check  each  thread's results for any errors, and re-throw
    // the first one we find. !r means success,  and  the  ASSERT
    // macro  is  not supposed to evaluate the second argument un-
    // less the first one is false.
    for( auto const& r : results ) ASSERT( !r, *r );
}

} // namespace util::par
