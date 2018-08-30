
#include "sigproc/common/exception.hpp"
#include "sigproc/bell/base.hpp"
#include "sigproc/bell/algorithm/dct.hpp"
#include "sigproc/bell/fftw/rfft.hpp"

#ifdef USE_FFMPEG
#include "sigproc/bell/ffmpeg/decode.hpp"
#endif

namespace sigproc {
    namespace bell {

using namespace sigproc::bell::fftw;

bool hasSuffix (const std::string& str, const std::string& suffix) {
    if (str.length() >= suffix.length()) {
        return (0 == str.compare (str.length() - suffix.length(), suffix.length(), suffix));
    } else {
        return false;
    }
}

template<typename T>
AudioDecoderBase<T>* newAudioDecoderFromPathImpl(const std::string& file_path, size_t Fs, size_t n_channels)
{
    ffmpeg::getFileCodecKind(file_path.c_str());

    AudioFormat fmt = AudioFormat::MP3;
    if (hasSuffix(file_path, ".wav")) {
        fmt = AudioFormat::WAV;
    } else if (hasSuffix(file_path, ".flac")) {
        fmt = AudioFormat::FLAC;
    }
    #ifdef USE_FFMPEG
        return new ffmpeg::Decoder<T>(fmt, static_cast<int>(Fs), static_cast<int>(n_channels));
    #endif
    SIGPROC_THROW("decoder not supported for: " << file_path);
}

template <>
AudioDecoderBase<uint8_t>* newAudioDecoderFromPath(const std::string& file_path, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromPathImpl<uint8_t>(file_path, static_cast<int>(Fs), static_cast<int>(n_channels));
}

template <>
AudioDecoderBase<int16_t>* newAudioDecoderFromPath(const std::string& file_path, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromPathImpl<int16_t>(file_path, static_cast<int>(Fs), static_cast<int>(n_channels));
}

template <>
AudioDecoderBase<float>* newAudioDecoderFromPath(const std::string& file_path, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromPathImpl<float>(file_path, static_cast<int>(Fs), static_cast<int>(n_channels));
}

template <>
AudioDecoderBase<double>* newAudioDecoderFromPath(const std::string& file_path, size_t Fs, size_t n_channels)
{
    return  newAudioDecoderFromPathImpl<double>(file_path, static_cast<int>(Fs), static_cast<int>(n_channels));
}


template<typename T>
AudioDecoderBase<T>* newAudioDecoderFromKindImpl(const std::string& kind, size_t Fs, size_t n_channels)
{
    #ifdef USE_FFMPEG
        return new ffmpeg::Decoder<T>(AudioFormat::MP3, static_cast<int>(Fs), static_cast<int>(n_channels));
    #endif
    SIGPROC_THROW("decoder not supported for: " << kind);
}

template <>
AudioDecoderBase<uint8_t>* newAudioDecoderFromKind(const std::string& kind, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromKindImpl<uint8_t>(kind, Fs, n_channels);
}

template <>
AudioDecoderBase<int16_t>* newAudioDecoderFromKind(const std::string& kind, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromKindImpl<int16_t>(kind, Fs, n_channels);
}

template <>
AudioDecoderBase<float>* newAudioDecoderFromKind(const std::string& kind, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromKindImpl<float>(kind, Fs, n_channels);
}

template <>
AudioDecoderBase<double>* newAudioDecoderFromKind(const std::string& kind, size_t Fs, size_t n_channels)
{
    return newAudioDecoderFromKindImpl<double>(kind, Fs, n_channels);
}


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