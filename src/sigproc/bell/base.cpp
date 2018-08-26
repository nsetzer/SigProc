
#include "sigproc/common/exception.hpp"
#include "sigproc/bell/base.hpp"
#include "sigproc/bell/algorithm/dct.hpp"
#include "sigproc/bell/fftw/rfft.hpp"

namespace sigproc {
    namespace bell {

using namespace sigproc::bell::fftw;

template <>
TransformBase<float>* newRealTransform<float>(TransformKind kind, size_t Fs, size_t N)
{
    // in the future I may have alternative implementations
    // that could be turned on and off with compiler flags
    //if (sigproc::bell::fftw::RealFFT::supports(kind)) {
    //    return new RealFFT<float>(Fs, N, kind);
    //}
    SIGPROC_THROW("Unsupported Transform Kind: " << kind);
}

template <>
TransformBase<double>* newRealTransform<double>(TransformKind kind, size_t Fs, size_t N)
{
    // in the future I may have alternative implementations
    // that could be turned on and off with compiler flags
    if (sigproc::bell::fftw::RealFFT<double>::supports(kind)) {
        return new RealFFT<double>(Fs, N, kind);
    }
    SIGPROC_THROW("Unsupported Transform Kind: " << kind);
}


template <>
TransformBase<float>* newCosineTransform(TransformKind kind, size_t N, size_t K)
{
    return new algorithm::DCTII<float>(N, K);
}

template <>
TransformBase<double>* newCosineTransform(TransformKind kind, size_t N, size_t K)
{
    return new algorithm::DCTII<double>(N, K);
}


    } // bell
} // sigproc

std::ostream& operator << (std::ostream& os, const sigproc::bell::TransformKind& kind)
{
    switch (kind) {
        case sigproc::bell::TransformKind::FORWARD:
            os << "FORWARD";
            break;
        case sigproc::bell::TransformKind::REVERSE:
            os << "REVERSE";
            break;
        case sigproc::bell::TransformKind::DCTII:
            os << "DCTII";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}