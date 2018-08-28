

#ifndef SIGPROC_BELL_BASE_HPP
#define SIGPROC_BELL_BASE_HPP

#include <vector>
#include <iostream>
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace bell {

template<typename T>
class AudioDecoderBase
{
public:
    AudioDecoderBase() {}
    virtual ~AudioDecoderBase() {}

    virtual void push_data(uint8_t* data, size_t n_elements) = 0;

    /*
    implementation defined output, index is the channel index of the output
    */
    virtual size_t output_size(size_t index) const = 0;
    virtual const T* output_data(size_t index) const  = 0;
    virtual void output_erase(size_t index, size_t n_elements) = 0;
};

/*
 * Return a new audio decoder which supports decoding the given kind
 * this may inspect the file header to determine the appropriate format
 * or will fallback to using the extension
 * the caller is still expected to use push_data
 * to push the contents of the file
 *
 * an exception is thrown if the format is not supported
 * The resulting decoder will yield samples at the requested sample rate
 * and with the number of channels. The decoder may resample the signal
 */
template <typename T>
AudioDecoderBase<T>* newAudioDecoder(std::string file_path, size_t Fs, size_t n_channels) {
    SIGPROC_THROW("invalid decoder type");
}

template <>
AudioDecoderBase<uint8_t>* newAudioDecoder(std::string file_path, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<int16_t>* newAudioDecoder(std::string file_path, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<float>* newAudioDecoder(std::string file_path, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<double>* newAudioDecoder(std::string file_path, size_t Fs, size_t n_channels);

/*
 * Return a new audio decoder which supports decoding the given kind
 * for symplicity, kind is the lower case file format extension
 *  e.g. wav, mp3, flac, etc
 * "raw" may be used to signed 16 bit PCM audio data. TODO: not implemented
 * an exception is thrown if the format is not supported
 * The resulting decoder will yield samples at the requested sample rate
 * and with the number of channels. The decoder may resample the signal
 */
template <typename T>
AudioDecoderBase<T>* newAudioDecoderFromKind(std::string kind, size_t Fs, size_t n_channels) {
    SIGPROC_THROW("invalid decoder type");
}

template <>
AudioDecoderBase<uint8_t>* newAudioDecoderFromKind(std::string kind, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<int16_t>* newAudioDecoderFromKind(std::string kind, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<float>* newAudioDecoderFromKind(std::string kind, size_t Fs, size_t n_channels);
template <>
AudioDecoderBase<double>* newAudioDecoderFromKind(std::string kind, size_t Fs, size_t n_channels);


enum class TransformKind : char {
    UNKNOWN      = 0,
    FORWARD      = 1, // DFT
    REVERSE      = 2, // Inverse DFT
    MAGNITUDE    = 3, //forward transform, normalized by N/2
    LOGMAGNITUDE = 4, // log of forward transform, normalized by N/2
    DCTII        = 4  //
};

template<typename T>
class TransformBase
{
public:
    TransformBase() {}
    virtual ~TransformBase() {}

    virtual size_t size() const =0;

    virtual T* inputBegin() =0;
    virtual T* inputEnd() =0;
    virtual T* outputBegin() =0;
    virtual T* outputEnd() =0;

    virtual void execute() =0;

};


template <typename T>
TransformBase<T>* newRealTransform(TransformKind kind, size_t Fs, size_t N) {
    SIGPROC_THROW("invalid transform type");
}

template <>
TransformBase<float>* newRealTransform(TransformKind kind, size_t Fs, size_t N);

template <>
TransformBase<double>* newRealTransform(TransformKind kind, size_t Fs, size_t N);

template <typename T>
TransformBase<T>* newCosineTransform(TransformKind kind, size_t N, size_t K) {
    SIGPROC_THROW("invalid transform type");
}

template <>
TransformBase<float>* newCosineTransform(TransformKind kind, size_t N, size_t K);

template <>
TransformBase<double>* newCosineTransform(TransformKind kind, size_t N, size_t K);

    } // bell
} // sigproc

std::ostream& operator << (std::ostream& os, const sigproc::bell::TransformKind& kind);

#endif