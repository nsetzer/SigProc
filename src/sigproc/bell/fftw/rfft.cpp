

#include <cmath>
#include <iostream>
#include <iomanip>

extern "C" {

#include "fftw3.h"

}

#include "sigproc/bell/fftw/rfft.hpp"

namespace sigproc {
    namespace bell {
        namespace fftw {

class RealFFTImpl
{
    double* m_data_i;
    double* m_data_o;
    size_t m_size;
    fftw_plan m_plan;
public:
   RealFFTImpl(size_t N, sigproc::bell::TransformKind kind);
   ~RealFFTImpl();

   size_t size() const { return m_size; }
   double* dataIn() { return m_data_i; }
   double* dataInEnd() { return m_data_i + m_size; }
   double* dataOut() { return m_data_o; }
   double* dataOutEnd() { return m_data_o + m_size; }

   void execute() {
        fftw_execute(m_plan);
   }

};

RealFFTImpl::RealFFTImpl(size_t N, sigproc::bell::TransformKind kind)
    : m_size(N)
{

    m_data_i = reinterpret_cast<double*>(fftw_malloc(sizeof(double) * m_size));
    m_data_o = reinterpret_cast<double*>(fftw_malloc(sizeof(double) * m_size));

    // http://www.fftw.org/fftw3_doc/Real_002dto_002dReal-Transform-Kinds.html#Real_002dto_002dReal-Transform-Kinds
    fftw_r2r_kind flags = static_cast<fftw_r2r_kind>(0);
    switch (kind) {
        case sigproc::bell::TransformKind::FORWARD:
            flags = FFTW_R2HC;
            break;
        case sigproc::bell::TransformKind::DCTII:
            flags = FFTW_REDFT10;
            break;
        case sigproc::bell::TransformKind::REVERSE:
            flags = FFTW_HC2R;
        default:
            break;
    }

    // http://www.fftw.org/fftw3_doc/Real_002dto_002dReal-Transforms.html
    // http://www.fftw.org/fftw3_doc/Planner-Flags.html#Planner-Flags
    m_plan = fftw_plan_r2r_1d(m_size, m_data_i, m_data_o, flags, FFTW_ESTIMATE);
}

RealFFTImpl::~RealFFTImpl() {
    fftw_destroy_plan(m_plan);
    fftw_free(m_data_o);
    fftw_free(m_data_i);
}

template<typename T>
RealFFT<T>::RealFFT(size_t samplerate, size_t N, sigproc::bell::TransformKind kind)
    : m_samplerate(samplerate)
    , m_impl(new RealFFTImpl(N, kind))
{}

template<typename T>
RealFFT<T>::~RealFFT() {
    delete m_impl;
}

template<typename T>
size_t RealFFT<T>::size()
{
    return m_impl->size();
}

template<typename T>
T* RealFFT<T>::inputBegin()
{
    return m_impl->dataIn();
}

template<typename T>
T* RealFFT<T>::inputEnd()
{
    return m_impl->dataInEnd();
}

template<typename T>
T* RealFFT<T>::outputBegin()
{
    return m_impl->dataOut();
}

template<typename T>
T* RealFFT<T>::outputEnd()
{
    return m_impl->dataOutEnd();
}

template<typename T>
void RealFFT<T>::execute()
{
    m_impl->execute();
}

template<typename T>
T RealFFT<T>::frequency(size_t index) {
     T k = static_cast<T>(index);
     T N = static_cast<T>(m_impl->size());
     T Fs = static_cast<T>(m_samplerate);
     if (index <= (m_impl->size()/2)) {
        return k*Fs/N;
    } else {
        return (k-N)*Fs/N;
    }
}

template<typename T>
bool RealFFT<T>::supports(sigproc::bell::TransformKind kind)
{
    switch (kind) {
        case sigproc::bell::TransformKind::FORWARD:
        case sigproc::bell::TransformKind::REVERSE:
        case sigproc::bell::TransformKind::DCTII:
            return true;
        default:
            return false;
    }
}


//template class RealFFT<float>;
template class RealFFT<double>;

        } // fftw
    } // bell
} // sigproc
