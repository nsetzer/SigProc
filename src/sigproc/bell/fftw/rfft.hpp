
#ifndef SIGPROC_BELL_FFTW_RFFT_HPP
#define SIGPROC_BELL_FFTW_RFFT_HPP

#include "sigproc/bell/base.hpp"

namespace sigproc {
    namespace bell {
        namespace fftw {

class RealFFTImpl;

template<typename T>
class RealFFT : public sigproc::bell::TransformBase<T>
{

    size_t m_samplerate;
    RealFFTImpl* m_impl;
public:
    RealFFT(size_t samplerate, size_t N, sigproc::bell::TransformKind kind);
    virtual ~RealFFT();

    RealFFT(const RealFFT& ) = delete; // copy constructor
    RealFFT(const RealFFT&& ) = delete; // move constructor
    RealFFT &operator=( const RealFFT& ) = delete; // copy assignment operator
    RealFFT &operator=( const RealFFT&&  ) = delete; // move assignment operator

    virtual size_t size() const;

    virtual T* inputBegin();
    virtual T* inputEnd();
    virtual T* outputBegin();
    virtual T* outputEnd();

    virtual T frequency(size_t index);

    void execute();

    static bool supports(sigproc::bell::TransformKind kind);

};

        } // fftw
    } // bell
} // sigproc

#endif
