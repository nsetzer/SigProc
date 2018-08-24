
#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/bell/algorithm/directform.hpp"
#include "sigproc/bell/algorithm/zbfilter.hpp"

using namespace sigproc::common;
using namespace sigproc::bell::algorithm;

// throw std::runtime_error(_exss.str().c_str());

#define CMP_DBL(x,y,eps,i) if (std::abs((y)-(x)) > eps) { \
    std::stringstream _exss; \
    _exss << "assert fail: line: " << __LINE__ << " index:" << i << " " << (x) << " != " << (y) << " | " << ((y)-(x)); \
    std::cerr << _exss.str() << std::endl; \
}

// FIR coefficient test, output should match coefficients
SIGPROC_TEST(zbfilter_peq)
{

    std::vector<float> B;
    std::vector<float> A;
    std::vector<float> v;

    float Fs = 8000;
    float Fc = 440;
    float Bw = 50;
    size_t N = 128;

    for (float& a : A) {
        a = -a;
    }
    zbpeqfilter<float>(B, A, Fc, Fs, Bw, 0);
    DirectFormII<float> df2(B, A);

    for (int i=0; i < N; i++) {
        v.push_back( sin(2 * M_PI * Fc/Fs * i));
    }

    df2.filt(&v[0], v.size());

    for (int i=0; i < N; i++) {
        float e = sin(2 * M_PI * Fc/Fs * i);
        CMP_DBL(e, v[i], 0.02, i);
    }

}


