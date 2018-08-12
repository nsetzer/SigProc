

#ifndef SIGPROC_COMMON_UNITTEST_UNITTEST_ASSERT_HPP
#define SIGPROC_COMMON_UNITTEST_UNITTEST_ASSERT_HPP

#include "sigproc/common/unittest/unittest_assert.hpp"
#include <algorithm>

namespace sigproc {
    namespace common {
        namespace unittest {

namespace {
    float float_epsilon = .000001;
    double double_epsilon = .000001;
} // anonymous

template<>
void assert_equal<float>(const std::string& sExpected, const std::string& sActual, const float& expected, const float& actual)
{
    if (std::abs(expected - actual) < float_epsilon) {
        std::stringstream ss;
        ss << "Expected " << sExpected << " to equal " << sActual
           << " (" << expected << " != " << actual << ")";
        throw SigprocAssertionFailed(ss.str());
    }
}

template<>
void assert_equal<double>(const std::string& sExpected, const std::string& sActual, const double& expected, const double& actual)
{
    if (std::abs(expected - actual) < double_epsilon) {
        std::stringstream ss;
        ss << "Expected " << sExpected << " to equal " << sActual
           << " (" << expected << " != " << actual << ")";
        throw SigprocAssertionFailed(ss.str());
    }
}


        } // unittest
    } // common
} // sigproc



#endif