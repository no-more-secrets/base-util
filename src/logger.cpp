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

} // namespace util
