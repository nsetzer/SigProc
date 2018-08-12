

#ifndef SIGPROC_COMMON_EXCEPTION_HPP
#define SIGPROC_COMMON_EXCEPTION_HPP

#include <exception>
#include <string>
#include <sstream>

namespace sigproc {
    namespace common {
        namespace exception {

class SigprocException : public std::runtime_error
{
public:
    SigprocException( const std::string& msg )
        : std::runtime_error( msg )
    {}
    SigprocException( const char* msg )
        : std::runtime_error( msg )
    {}
    ~SigprocException(){}
};

        } // exception
    } // common
} // sigproc


#define UNUSED(x) (void) x

#define Val(x) " " #x << " = " << x

#define SIGPROC_THROW_EXCEPTION(t, x) do { \
    std::stringstream _ex_ss; \
    _ex_ss << x; \
    throw t(_ex_ss.str()); \
} while(0)

#define SIGPROC_THROW(x) SIGPROC_THROW_EXCEPTION( \
    sigproc::common::exception::SigprocException, x)

#endif