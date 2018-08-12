
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
