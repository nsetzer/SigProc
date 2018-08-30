

#ifndef SIGPROC_BELL_FFMPEG_DECODE_HPP
#define SIGPROC_BELL_FFMPEG_DECODE_HPP

#include <memory>

#include "sigproc/bell/base.hpp"

namespace sigproc {
    namespace bell {
        namespace ffmpeg {

void getFileCodecKind(const char* filepath);

template <typename T>
class DecoderImpl;

/**
 * Decode audio from a file and return samples
 * at a desired sample rate and number of channels
 *
 * The template parameter controls the output format of the data
 *
 *   uint8_t - PCM samples, interleaved channels
 *   int16_t - PCM samples, channel separated
 *   float   - 32bit float samples, channel separated
 *   double  - 64bit float samples, channel separated
 *
 * other types are not supported and will cause an exception to be thrown
 *
 * the floating point options are optimized for input into an FFT
 */
template <typename T>
class Decoder : public AudioDecoderBase<T>
{
    std::unique_ptr<DecoderImpl<T>> m_impl;
public:
    Decoder(AudioFormat format, int sample_rate, int n_channels);
    ~Decoder();

    // push bytes from a file (wav, mp3, flac, etc)
    void push_data(uint8_t* data, size_t n_elements);

    // access decoded samples
    // signed types support multiple channels
    // unsigned types contain interleaved data

    //number of samples available for a given channel
    // size is always the number of samples for the given template type
    size_t output_size(size_t index) const;

    // begining of the output array (contiguous)
    const T* output_data(size_t index) const;

    // mark the first N samples for deletion
    void output_erase(size_t index, size_t n_elements);

};


        } // ffmpeg
    } // bell
} // sigproc

#endif
