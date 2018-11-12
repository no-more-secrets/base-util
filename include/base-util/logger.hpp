/****************************************************************
* Logging
****************************************************************/
#pragma once

#include "base-util/non-copyable.hpp"

#include <iostream>

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

// This  operator  overload simply forwards requests to cout when
// logging is enabled, otherwise does nothing.
template<typename T>
Logger& operator<<( Logger& lgr, T const& item ) {

    // This is not strictly necessary  for this function, but for
    // convenience to the user of this  library we pull in all of
    // our  custom  operator<<  overloads  from  the util library.
    using ::util::operator<<;

    if( Logger::enabled )
        std::cout << item;

    return lgr;
}

} // namespace util
