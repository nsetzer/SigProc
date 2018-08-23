

#ifndef SIGPROC_COMMON_UNITTEST_UNITTEST_ASSERT_HPP
#define SIGPROC_COMMON_UNITTEST_UNITTEST_ASSERT_HPP

#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace common {
        namespace unittest {

class SigprocAssertionFailed : public sigproc::common::exception::SigprocException
{
public:
    SigprocAssertionFailed( const std::string& msg )
        : sigproc::common::exception::SigprocException( msg )
    {}
    SigprocAssertionFailed( const char* msg )
        : sigproc::common::exception::SigprocException( msg )
    {}
    ~SigprocAssertionFailed(){}
};

template<typename T>
void assert_equal(const std::string& sExpected, const std::string& sActual, T expected, T actual) {
    if (expected != actual) {
        std::stringstream ss;
        ss << "Expected " << sExpected << " to equal " << sActual
           << " (" << expected << " != " << actual << ")";
        throw SigprocAssertionFailed(ss.str());
    }
}

template<typename T>
void assert_not_null(const std::string& sExpected, const T& expected) {
    if (expected == nullptr) {
        std::stringstream ss;
        ss << "Expected " << sExpected << " to not be null";
        throw SigprocAssertionFailed(ss.str());
    }
}

template<typename T>
void assert_null(const std::string& sExpected, const T& expected) {
    if (expected != nullptr) {
        std::stringstream ss;
        ss << "Expected " << sExpected << " to be null";
        throw SigprocAssertionFailed(ss.str());
    }
}

        } // unittest
    } // common
} // sigproc



#define ASSERT_EQUAL(x, y) sigproc::common::unittest::assert_equal(#x, #y, x, y)
#define ASSERT_STR_EQUAL(x, y) sigproc::common::unittest::assert_equal<std::string>(#x, #y, x, y)
#define ASSERT_NOT_NULL(x) sigproc::common::unittest::assert_not_null(#x, x)
#define ASSERT_NULL(x) sigproc::common::unittest::assert_null(#x, x)


#define ASSERT_FAIL(x) { \
    std::stringstream ss; \
    ss << "Assertion Failed: " << x; \
    throw sigproc::common::unittest::SigprocAssertionFailed(ss.str()); \
}

#endif