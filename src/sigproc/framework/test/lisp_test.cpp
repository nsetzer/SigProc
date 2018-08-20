#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/common/format.hpp"
#include "sigproc/framework/lisp.hpp"

using namespace sigproc::framework;
using namespace sigproc::common;

SIGPROC_TEST(LispStream_eval_0) {

    LispStream stream(true);

    //stream.push("+ 1 (+ 2 (+ 3))");
    //stream.push("+ 3.5 1)");
    stream.push("(+ (* 2 2) (/ 8 2) (- 6 2))");

    if (stream.root() != nullptr) {
        std::cout << *stream.root() << std::endl;
        std::cout << *stream.eval() << std::endl;
        std::cout << *stream.root() << std::endl;
    }

}

