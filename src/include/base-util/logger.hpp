/****************************************************************
* Logging
****************************************************************/
#pragma once

#include "base-util/non-copyable.hpp"

#include <iostream>
#include <type_traits>

namespace util {

/* This is a singleton class, the object of which will represent
 * the global logger object. */
struct Logger : public singleton {

public:
    // Get the global logger instance.
    static Logger& logger() noexcept;

    // When this flag is  false,  logging  has  no  effect. If it
    // false by default.
    static bool    enabled;

private:
    Logger() = default;

};

/* This is a reference to the  global  logger  object  for  conve-
 * nience, similarly to cout/cerr. Wanted to use `clog`  for  the
 * name,  but  that one is already in std and want to avoid confu-
 * sion. */
extern Logger& log;

// This is to allow logging e.g. `endl`, although this should
// probably be avoided for the some reasons that one should avoid
// using endl in general.
using IOManipulator = std::ostream&(*)( std::ostream& );
Logger& operator<<( Logger& lgr, IOManipulator item );

// This  operator  overload simply forwards requests to cout when
// logging is enabled, otherwise does nothing.
template<typename T>
Logger& operator<<( Logger& lgr, T const& item ) {

    // Here we explicitly decay the argument type.  This would
    // have happened implicitely, but then clang-tidy complains.
    // One case where decay happens is then logging a string
    // literal, in which case e.g. T == const char[5], which gets
    // decayed to a function pointer.
    //
    // We need T const& here instead of just T because otherwise
    // the const will be stripped from the T which we don't want.
    // i.e., there may be another const in the type T itself that
    // we don't want to lose.
    using decayed_t = std::decay_t<T const&>;

    // This is not strictly necessary  for this function, but for
    // convenience to the user of this  library we pull in all of
    // our  custom  operator<<  overloads  from  the util library.
    using ::util::operator<<;

    if( Logger::enabled )
        std::cout << static_cast<decayed_t>( item );

    return lgr;
}

} // namespace util
