/****************************************************************
* Logging
****************************************************************/
#include "base-util/logger.hpp"

namespace util {

bool Logger::enabled = false;

Logger& Logger::logger() noexcept {
    static Logger global_logger;
    return global_logger;
}

/* This is a reference to the  global  logger  object  for  conve-
 * nience, similarly to cout/cerr. */
Logger& log = Logger::logger();

// IOManipulator is a function pointer.  e.g., `endl`
Logger& operator<<( Logger& lgr, IOManipulator item ) {

    // This is not strictly necessary  for this function, but for
    // convenience to the user of this  library we pull in all of
    // our  custom  operator<<  overloads  from  the util library.
    using ::util::operator<<;

    if( Logger::enabled )
        item( std::cout );

    return lgr;
}

} // namespace util
