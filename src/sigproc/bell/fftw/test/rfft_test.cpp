#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/bell/base.hpp"
#include "sigproc/bell/fftw/rfft.hpp"

using namespace sigproc::common;
using namespace sigproc::bell::fftw;


// https://dsp.stackexchange.com/questions/633/what-data-should-i-use-to-test-an-fft-implementation-and-what-accuracy-should-i

double epsilon = 0.00001;
#define CMP_DBL(x,y) if (std::abs((y)-(x)) > epsilon) { \
    std::stringstream _exss; \
    _exss << "assert fail: " << __LINE__ << " " << (x) << " != " << (y) << " | " << ((y)-(x)); \
    throw std::runtime_error(_exss.str().c_str()); \
}

/**
 * Check that the mapping for bin index to frequency is correct
 */
SIGPROC_TEST(RFFT_freq)
{
    size_t iN = 256;
    size_t iFs = 8000;
    RealFFT<double> fft1(iFs, iN, sigproc::bell::TransformKind::FORWARD);

    double N = static_cast<double>(iN);
    double Fs = static_cast<double>(iFs);

    CMP_DBL(0.0, fft1.frequency(0));
    CMP_DBL(Fs/N, fft1.frequency(1));
    // technically, this is both positive and negative Fs/2.0
    CMP_DBL(Fs/2.0, fft1.frequency(iN/2));
    CMP_DBL(-Fs/2.0 + Fs/N, fft1.frequency(iN/2+1));
    CMP_DBL(Fs/2.0 - Fs/N, fft1.frequency(iN/2-1));

}

/**
 * show that the following equation holds for FFT forward:
 * FFT(a1x1 + a2x2) = a1FFT(x1) + a2FFT(x2)
 */
SIGPROC_TEST(RFFT_linearity)
{
    size_t N = 256;
    size_t Fs = 8000;
    RealFFT<double> fft1(Fs, N, sigproc::bell::TransformKind::FORWARD);
    RealFFT<double> fft2(Fs, N, sigproc::bell::TransformKind::FORWARD);
    RealFFT<double> fft3(Fs, N, sigproc::bell::TransformKind::FORWARD);

    // some arbitrary constants
    double a1 = 1.0/3.0;
    double a2 = 2.0/3.0;
    double x1 = 0.75;
    double x2 = 0.25;

    // generate a set of signals
    {
        double* iter = fft1.inputBegin();
        double* end = fft1.inputEnd();
        int i=0;
        while(iter != end) {
            *iter++ = x1 * sin(2.0 * M_PI * 440.0 / Fs * i);
            i++;
        }
    }

    {
        double* iter = fft2.inputBegin();
        double* end = fft2.inputEnd();
        int i=0;
        while(iter != end) {
            *iter++ = x2 * sin(2.0 * M_PI * 880.0 / Fs * i);
            i++;
        }
    }

    {
        double* iter = fft3.inputBegin();
        double* end = fft3.inputEnd();
        int i=0;
        while(iter != end) {
            double v1 = x1 * sin(2.0 * M_PI * 440.0 / Fs * i);
            double v2 = x2 * sin(2.0 * M_PI * 880.0 / Fs * i);
            *iter++ = a1*v1 + a2*v2;
            i++;
        }
    }

    fft1.execute();
    fft2.execute();
    fft3.execute();

    {
        double* iter1 = fft1.outputBegin();
        double* end1 = fft1.outputEnd();

        double* iter2 = fft2.outputBegin();
        double* end2 = fft2.outputEnd();

        double* iter3 = fft3.outputBegin();
        double* end3 = fft3.outputEnd();

        while(iter3 != end3) {
            double left = *iter3++;
            double right = (a1 * (*iter1++)) + (a2 * (*iter2++));
            CMP_DBL(left, right);
        }
    }
}
