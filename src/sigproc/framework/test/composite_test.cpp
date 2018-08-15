
#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/framework/composite.hpp"

using namespace sigproc::framework;

SIGPROC_TEST(CompositeEncode_int) {

    Composite c(1474);

    std::stringstream ss;
    ss << c;

    ASSERT_STR_EQUAL("1474", ss.str());

}


SIGPROC_TEST(CompositeCast_bool) {

    {
        bool n = true;
        Composite c(n);
        ASSERT_EQUAL(n, c.as_bool());
    }

    {
        int8_t n = 76;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        int16_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        int32_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        int64_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        uint8_t n = 76;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        uint16_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        uint32_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

    {
        uint64_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<bool>(n), c.as_bool());
    }

}

SIGPROC_TEST(CompositeCast_int) {

    {
        bool n = true;
        Composite c(n);
        ASSERT_EQUAL(static_cast<int64_t>(n), c.as_int());
    }

    {
        int8_t n = 76;
        Composite c(n);
        ASSERT_EQUAL(static_cast<int64_t>(n), c.as_int());
    }

    {
        int16_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<int64_t>(n), c.as_int());
    }

    {
        int32_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(static_cast<int64_t>(n), c.as_int());
    }

    {
        int64_t n = 1024;
        Composite c(n);
        ASSERT_EQUAL(n, c.as_int());
    }

}