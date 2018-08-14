
#ifndef SIGPROC_COMMON_UNITTEST_UNITTEST_HPP
#define SIGPROC_COMMON_UNITTEST_UNITTEST_HPP

#include <vector>
#include <string>
#include <iostream>

#include "sigproc/common/unittest/unittest_assert.hpp"

namespace sigproc {
    namespace common {
        namespace unittest {


class UnitTest
{
    std::string m_name;
public:
    UnitTest(const std::string& name)
        : m_name(name)
    {}
    ~UnitTest() {}

    virtual void run() = 0;

    const std::string& name() { return m_name; }
};

class UnitTestRegistry
{
public:
    UnitTestRegistry() {}
    ~UnitTestRegistry() {}

    static std::vector<UnitTest*>* registry;

    static void reg(UnitTest* test);

    static int run_tests();

};

#define SIGPROC_TEST(TESTNAME) \
class TESTNAME : public sigproc::common::unittest::UnitTest { \
  public: \
    TESTNAME() : sigproc::common::unittest::UnitTest(#TESTNAME) { \
        sigproc::common::unittest::UnitTestRegistry::reg(this); \
    } \
    ~TESTNAME() {} \
    virtual void run() final; \
}; \
static TESTNAME TESTNAME ## _instance; void TESTNAME::run()


        } // unittest
    } // common
} // sigproc

#endif