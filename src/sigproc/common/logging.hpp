
#ifndef SIGPROC_COMMON_LOGGING_HPP
#define SIGPROC_COMMON_LOGGING_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "sigproc/common/format.hpp"

namespace sigproc {
    namespace common {
        namespace logging {
/*
enum class Level: char {
    OFF     =  0,
    ERROR   = 10,
    WARNING = 20,
    INFO    = 30,
    DEBUG   = 40,
    TRACE   = 50
};
*/

class Logger
{

public:
    Logger() {}
    ~Logger() {}

    static int level;
};




        } // logging
    } // common
} // sigproc

#endif