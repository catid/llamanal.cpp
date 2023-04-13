/*
    Goals:

    * Support debug/info/warning/error levels, with verbose flag
    * Rotating disk file logs up to 10MB
    * Multi-threaded logging from background thread to avoid slowdown
*/

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <boost/log/trivial.hpp>

namespace analysis {


//------------------------------------------------------------------------------
// Logging Initialization

void init_logging(bool verbose);
void stop_logging();


} // namespace analysis

#endif // LOGGING_HPP
