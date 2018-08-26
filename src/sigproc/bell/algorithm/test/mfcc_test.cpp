
#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/bell/algorithm/mfcc.hpp"

using namespace sigproc::common;
using namespace sigproc::bell::algorithm;

// throw std::runtime_error(_exss.str().c_str());

#define CMP_DBL(x,y,eps,i) if (std::abs((y)-(x)) > eps) { \
    std::stringstream _exss; \
    _exss << "assert fail: line: " << __LINE__ << " index:" << i << " " << (x) << " != " << (y) << " | " << ((y)-(x)); \
    std::cerr << _exss.str() << std::endl; \
}

// FIR coefficient test, output should match coefficients
SIGPROC_TEST(mfcc_filterbank)
{

    size_t N = 256;
    mfcc::FilterBank<double> filterBank(8000, N, 10, 350, 3500, true);

    std::vector<double> in;
    std::vector<double> out;
    in.resize(N);
    std::fill(in.begin()+11, in.begin()+20, 1.0);

    std::cout << out.size() << std::endl;

    for (double& v : in) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    filterBank.filter(in, out);


    for (double& v : out) {
        std::cout << v << std::endl;
    }



}


