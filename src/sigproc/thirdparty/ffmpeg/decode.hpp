

#ifndef SIGPROC_THIRDPARTY_FFMPEG_HPP
#define SIGPROC_THIRDPARTY_FFMPEG_HPP

#include <memory>

namespace sigproc {
    namespace thirdparty {
        namespace ffmpeg {

class DecoderImpl;

// todo: templatize the output,
// allow for returing
//      uint8_t - simple byt arrays (useful for files)
//      uint16_t - PCM samples
//      float  - may be usefull
//      double - best for FFTW
// input is always byte arrays for symplicity (reading from a file)

/**
 * Decode audio from a file and return interleaved samples
 * at a desired sample rate and number of channels
 */
class Decoder
{
    std::unique_ptr<DecoderImpl> m_impl;
public:
    Decoder(int format, int sample_rate, int n_channels);
    ~Decoder();

    void push_data(uint8_t* data, size_t n_elements);

    size_t output_size() const;
    const uint8_t* output_data() const;
    void output_erase(size_t n_elements);

};


        } // ffmpeg
    } // thirdparty
} // sigproc

#endif