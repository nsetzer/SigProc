

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
   RealFFTImpl(size_t N, FFTKind kind);
   ~RealFFTImpl();

   size_t size() const { return m_size; }
   double* dataIn() { return m_data_i; }
   double* dataInEnd() { return m_data_i + m_size; }
   double* dataOut() { return m_data_o; }

   void execute() {
        fftw_execute(m_plan);
   }

};

RealFFTImpl::RealFFTImpl(size_t N, FFTKind kind)
    : m_size(N)
{

    m_data_i = reinterpret_cast<double*>(fftw_malloc(sizeof(double) * m_size));
    m_data_o = reinterpret_cast<double*>(fftw_malloc(sizeof(double) * m_size));

    // http://www.fftw.org/fftw3_doc/Real_002dto_002dReal-Transform-Kinds.html#Real_002dto_002dReal-Transform-Kinds
    fftw_r2r_kind flags = static_cast<fftw_r2r_kind>(0);
    switch (kind) {
        case FFTKind::FORWARD:
            flags = FFTW_R2HC;
            break;
        case FFTKind::DCTII:
            flags = FFTW_REDFT10;
            break;
        case FFTKind::REVERSE:
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

RealFFT::RealFFT(size_t samplerate, size_t N, FFTKind kind)
    : m_samplerate(samplerate)
    , m_impl(new RealFFTImpl(N, kind))
{}

RealFFT::~RealFFT() {
    delete m_impl;
}

double* RealFFT::inputBegin()
{
    return m_impl->dataIn();
}

double* RealFFT::inputEnd()
{
    return m_impl->dataInEnd();
}

double* RealFFT::outputBegin()
{
    return m_impl->dataOut();
}

double* RealFFT::outputEnd()
{
    return m_impl->dataOut() + (m_impl->size()/2);
}

void RealFFT::execute()
{
    m_impl->execute();
}

double RealFFT::frequency(size_t index) {
     double k = static_cast<double>(index);
     double N = static_cast<double>(m_impl->size());
     double Fs = static_cast<double>(m_samplerate);
     if (index <= (m_impl->size()/2)) {
        return k*Fs/N;
    } else {
        return (k-N)*Fs/N;
    }
}

        } // fftw
    } // bell
} // sigproc
