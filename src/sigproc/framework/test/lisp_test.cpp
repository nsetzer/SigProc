#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/framework/lisp.hpp"

using namespace sigproc::framework;

SIGPROC_TEST(CompositeEncode_int) {

    LispStream stream(true);

    stream.push("+ 1 (+ 2 (+ 3))");

    if (stream.root() != nullptr) {
        std::cout << *stream.root() << std::endl;
        std::cout << *stream.eval() << std::endl;
        std::cout << *stream.root() << std::endl;
    }

}