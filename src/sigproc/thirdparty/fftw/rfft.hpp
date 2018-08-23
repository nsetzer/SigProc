
#ifndef SIGPROC_THIRDPARTY_FFTW_RFFT_HPP
#define SIGPROC_THIRDPARTY_FFTW_RFFT_HPP

namespace sigproc {
    namespace thirdparty {
        namespace fftw {

enum class FFTKind : char {
    UNKNOWN = 0,
    FORWARD = 1,
    REVERSE = 2,
    DCTII   = 3
};

class RealFFTImpl;

class RealFFT
{

    size_t m_samplerate;
    RealFFTImpl* m_impl;
public:
    RealFFT(size_t samplerate, size_t N, FFTKind kind);
    ~RealFFT();

    RealFFT(const RealFFT& ) = delete; // copy constructor
    RealFFT(const RealFFT&& ) = delete; // move constructor
    RealFFT &operator=( const RealFFT& ) = delete; // copy assignment operator
    RealFFT &operator=( const RealFFT&&  ) = delete; // move assignment operator

    double* inputBegin();
    double* inputEnd();
    double* outputBegin();
    double* outputEnd();

    double frequency(size_t index);

    void execute();

};

        } // fftw
    } // thirdparty
} // sigproc

#endif