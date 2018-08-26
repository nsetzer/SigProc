
#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/bell/algorithm/directform.hpp"

using namespace sigproc::common;
using namespace sigproc::bell::algorithm;

double epsilon = 0.00001;
#define CMP_DBL(x,y) if (std::abs((y)-(x)) > epsilon) { \
    std::stringstream _exss; \
    _exss << "assert fail: " << __LINE__ << " " << (x) << " != " << (y) << " | " << ((y)-(x)); \
    throw std::runtime_error(_exss.str().c_str()); \
}

// FIR coefficient test, output should match coefficients
SIGPROC_TEST(DF2_B)
{

    std::vector<float> B = {0.5, 0.25, 0.75};
    std::vector<float> A = {1.0, 0.0, 0.0};
    DirectFormII<float> df2(B, A);

    std::vector<float> v = {1.0, 0.0, 0.0, 0.0};
    std::vector<float> e = {0.5, 0.25, 0.75, 0.0};

    df2.filt(&v[0], v.size());

    for (size_t i=0; i < e.size(); i++) {
        CMP_DBL(e[i], v[i]);
    }
}

// IIR coefficient test, decimate by half
SIGPROC_TEST(DF2_A1)
{

    std::vector<float> B = {1.0, 0.0, 0.0};
    std::vector<float> A = {1.0, -0.5, 0.0};
    DirectFormII<float> df2(B, A);

    std::vector<float> v = {1.0, 0.0, 0.0, 0.0};
    std::vector<float> e = {1.0, 0.5, 0.25, 0.125};

    df2.filt(&v[0], v.size());

    for (size_t i=0; i < e.size(); i++) {
        CMP_DBL(e[i], v[i]);
    }
}

// IIR coefficient test, decimate by half and flip sign
SIGPROC_TEST(DF2_A2)
{

    std::vector<float> B = {1.0, 0.0, 0.0};
    std::vector<float> A = {1.0, 0.5, 0.0};
    DirectFormII<float> df2(B, A);

    std::vector<float> v = {1.0, 0.0, 0.0, 0.0};
    std::vector<float> e = {1.0, -0.5, 0.25, -0.125};

    df2.filt(&v[0], v.size());

    for (size_t i=0; i < e.size(); i++) {
        CMP_DBL(e[i], v[i]);
    }
}

