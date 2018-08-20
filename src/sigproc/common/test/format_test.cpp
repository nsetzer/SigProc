#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/common/format.hpp"

using namespace sigproc::common;

SIGPROC_TEST(Format_sprintf_simple) {
    std::string expected = "<123>";
    std::string actual = fmt::sprintf("<%d>", 123);
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_extra_args) {
    std::string expected = "...123...";
    std::string actual = fmt::sprintf("...%d...", 123, 456);
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_missing_args) {
    std::string expected = "...123...%d...";
    std::string actual = fmt::sprintf("...%d...%d...", 123);
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_percent_000) {
    std::string expected = "test % test";
    std::string actual = fmt::sprintf("test %% test");
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_percent_001) {
    std::string expected = "%50";
    std::string actual = fmt::sprintf("%%%d", 50);
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_percent_001_alt) {
    std::string expected = "50%";
    std::string actual = fmt::sprintf("%d%%", 50);
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_float_000) {
    {
        std::string expected = "3.14";
        std::string actual = fmt::sprintf("%f", 3.14);
        ASSERT_STR_EQUAL(expected, actual);
    }

    {
        std::string expected = "3.14";
        std::string actual = fmt::sprintf("%.2f", 3.14);
        ASSERT_STR_EQUAL(expected, actual);
    }

    {
        std::string expected = "3.14";
        std::string actual = fmt::sprintf("%.2f", 3.1415);
        ASSERT_STR_EQUAL(expected, actual);
    }

    {
        std::string expected = "1234.5678";
        std::string actual = fmt::sprintf("%.4f", 1234.5678);
        ASSERT_STR_EQUAL(expected, actual);
    }
}

SIGPROC_TEST(Format_sprintf_align_center_000) {
    std::string expected = "  abc  ";
    std::string actual = fmt::sprintf("%=7s", "abc");
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_align_center_001) {
    std::string expected = "  abcd  ";
    std::string actual = fmt::sprintf("%=8s", "abcd");
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_align_center_002) {
    std::string expected = "abcdefg";
    std::string actual = fmt::sprintf("%=5s", "abcdefg");
    ASSERT_STR_EQUAL(expected, actual);
}

SIGPROC_TEST(Format_sprintf_pointer) {
    std::string expected = "0x1234";
    int* ptr = reinterpret_cast<int*>(0x1234);
    std::string actual = fmt::sprintf("%p", ptr);
    ASSERT_STR_EQUAL(expected, actual);
}





