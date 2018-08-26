
#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/bell/algorithm/dct.hpp"

using namespace sigproc::common;
using namespace sigproc::bell::algorithm;

// throw std::runtime_error(_exss.str().c_str());

#define CMP_DBL(x,y,eps,i) if (std::abs((y)-(x)) > eps) { \
    std::stringstream _exss; \
    _exss << "assert fail: line: " << __LINE__ << " index:" << i << " " << (x) << " != " << (y) << " | " << ((y)-(x)); \
    std::cerr << _exss.str() << std::endl; \
}

// FIR coefficient test, output should match coefficients
SIGPROC_TEST(dct_test)
{

    DCTII<double> dct(5, 3);
    std::vector<double> expected = {5, 0, 0};

    for (double* iter = dct.inputBegin(); iter != dct.inputEnd(); iter++) {
        *iter = 1.0;
    }

    dct.execute();

    for (double* iter = dct.outputBegin(); iter != dct.outputEnd(); iter++) {
        std::cout << *iter << std::endl;
    }


}
