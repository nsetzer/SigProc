
#include <iostream>
#include <vector>

#include "sigproc/bell/ffmpeg/decode.hpp"
#include "sigproc/common/format.hpp"
#include "sigproc/common/exception.hpp"

using namespace sigproc::common;

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
}

namespace sigproc {
    namespace bell {
        namespace ffmpeg {

template<typename T>
class BufferedVector
{
    size_t m_index;
    std::vector<T> m_data;
public:
    BufferedVector()
        : m_index(0)
        , m_data()
    {}
    ~BufferedVector() {}

    const T* data() const { return &m_data[m_index]; }
    size_t size() const { return m_data.size() - m_index; }

    void push_back(const T* data, size_t n_elements) {
        // free memory that has been consumed
        if (m_index > 0) {
            m_data.erase(m_data.begin(), m_data.begin() + m_index);
            m_index = 0;
        }
        // increase capactity to hold new data
        size_t n = m_data.size();
        size_t cap = n + n_elements;
        m_data.reserve( cap );
        // copy new data into this
        //std::copy(&m_data[n], data, sizeof(T) * n_elements);

        //copy(&data[0], &data[n_elements], back_inserter(m_data));
        for (size_t i=0; i< n_elements; i++) {
            m_data.push_back(data[i]);
        }
    }

    void erase(size_t n_elements) {
        m_index += n_elements;
    }

    void reserve(size_t overhead) {
        size_t cap = m_data.size() + overhead;
        m_data.reserve(cap);
    }

};

template<typename T>
enum AVSampleFormat get_sample_format() {
    SIGPROC_THROW("unsupported type");
}

// return interleaved PCM data
template<>
enum AVSampleFormat get_sample_format<uint8_t>() {
    return AV_SAMPLE_FMT_S16;
}

// return planar 16 bit data
template<>
enum AVSampleFormat get_sample_format<int16_t>() {
    return AV_SAMPLE_FMT_S16P;
}

template<>
enum AVSampleFormat get_sample_format<float>() {
    return AV_SAMPLE_FMT_FLTP;
}

template<>
enum AVSampleFormat get_sample_format<double>() {
    return AV_SAMPLE_FMT_DBLP;
}

/*
 * template traits method for pushing samples to a set of buffers
 *
 * buffers: a set of channel separated buffers to push to.
 * nb_channels: number of channels, also length of buffers vector
 * linesize: number of bytes per channel plane
 * data: raw pointer to buffer data
 * datalen: number of bytes available in data
 */
template<typename T>
void push_samples(std::vector<BufferedVector<T>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    SIGPROC_THROW("unsupported type");
}

template<>
void push_samples<uint8_t>(std::vector<BufferedVector<uint8_t>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are intentionally ignored for this type
    // write some documentation,
    // use a traits function for allocating the buffer array correctly
    std::cout << " uint8_t linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
    buffers[0].push_back(data, datalen);
}

template<>
void push_samples<int16_t>(std::vector<BufferedVector<int16_t>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    const int16_t* cdata = reinterpret_cast<const int16_t*>(data);
    int datasize = datalen / sizeof(int16_t);
    buffers[0].push_back(cdata, datasize);
}

template<>
void push_samples<float>(std::vector<BufferedVector<float>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    const float* cdata = reinterpret_cast<const float*>(data);
    int datasize = datalen / sizeof(float);
    buffers[0].push_back(cdata, datasize);
}

template<>
void push_samples<double>(std::vector<BufferedVector<double>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    std::cout << " double linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
    const double* cdata = reinterpret_cast<const double*>(data);
    int datasize = datalen / sizeof(double);
    buffers[0].push_back(cdata, datasize);
}


template <typename T>
class ResamplerImpl
{
    int64_t m_src_ch_layout;
    int64_t m_dst_ch_layout;
    // source channels which comes from the frame
    int m_src_nb_channels;
    int m_dst_nb_channels;
    // sample format which comes from the frame
    enum AVSampleFormat m_src_sample_fmt;
    // TODO make it possible to get floats output as double,
    //   AV_SAMPLE_FMT_DBL : interleaved
    enum AVSampleFormat m_dst_sample_fmt;

    // sample rate which comes from the frame
    int m_src_rate;
    int m_dst_rate;

    int m_dst_linesize;

    int m_src_nb_samples;
    int m_dst_nb_samples;
    int m_max_dst_nb_samples;

    struct SwrContext *m_swr_ctx = NULL;

    uint8_t** m_dst_data = NULL;

public:
    ResamplerImpl()
    {
        m_src_ch_layout = AV_CH_LAYOUT_STEREO;
        m_dst_ch_layout = AV_CH_LAYOUT_MONO;
        m_src_nb_channels = 0;
        m_dst_nb_channels = 0;
        // source sample format is set by the decoder
        m_src_sample_fmt = AV_SAMPLE_FMT_FLTP;
        m_dst_sample_fmt = get_sample_format<T>();
        m_src_rate = 0;
        m_dst_rate = 0;
        m_dst_linesize = 0;
        m_src_nb_samples = 0;
        m_dst_nb_samples = 0;
        m_max_dst_nb_samples = 0;
    }
    ~ResamplerImpl() {}

    void set_input_opts(AVCodecContext *ctx, AVFrame *frame) {
        m_src_nb_channels = ctx->channels;
        if (m_src_nb_channels > 2) {
            throw std::runtime_error("invalid number of channels");
        }
        m_src_rate = frame->sample_rate;
        if (m_src_rate < 100) {
            SIGPROC_THROW("source rate lower than expected: " << m_src_rate);
        }
        m_src_ch_layout = (m_src_nb_channels==2)?AV_CH_LAYOUT_STEREO:AV_CH_LAYOUT_MONO;
        m_src_sample_fmt = static_cast<AVSampleFormat>(frame->format);

    }

    void set_output_opts(int n_channels, int sample_rate) {
        m_dst_nb_channels = n_channels;
        if (m_dst_nb_channels > 2) {
            throw std::runtime_error("invalid number of channels");
        }
        m_dst_rate = sample_rate;
        if (m_dst_rate < 100) {
            SIGPROC_THROW("source rate lower than expected: " << m_src_rate);
        }
        m_dst_ch_layout = (n_channels==2)?AV_CH_LAYOUT_STEREO:AV_CH_LAYOUT_MONO;
    }

    bool configure() {
        int ret;

        if (m_src_rate==0 || m_dst_rate==0) {
            // occasionally a bad frame sneaks through for the input source
            std::cerr << "trying to configure with a bad configuration" << std::endl;
            return false;
        }
        m_swr_ctx = swr_alloc();
        if (!m_swr_ctx) {
            throw std::runtime_error("m_swr_ctx alloc");
        }

        av_opt_set_int(m_swr_ctx, "in_channel_layout",    m_src_ch_layout, 0);
        av_opt_set_int(m_swr_ctx, "in_sample_rate",       m_src_rate, 0);
        av_opt_set_sample_fmt(m_swr_ctx, "in_sample_fmt", m_src_sample_fmt, 0);

        av_opt_set_int(m_swr_ctx, "out_channel_layout",    m_dst_ch_layout, 0);
        av_opt_set_int(m_swr_ctx, "out_sample_rate",       m_dst_rate, 0);
        av_opt_set_sample_fmt(m_swr_ctx, "out_sample_fmt", m_dst_sample_fmt, 0);
        av_opt_set_int(m_swr_ctx, "filter_type", SWR_FILTER_TYPE_CUBIC, 0);

        if ((ret = swr_init(m_swr_ctx)) < 0) {
            throw std::runtime_error("m_swr_ctx init");
        }

        m_max_dst_nb_samples = 0;
        m_dst_nb_samples = 0;

        return true;
    }

    bool is_configured() {
        return m_swr_ctx != nullptr;
    }

    size_t resample(AVFrame *frame, std::vector<BufferedVector<T>>& buffers) {
        int ret;

        if (m_src_rate==0) {
            SIGPROC_THROW("source rate not initialized");
        }

        if (m_dst_rate==0) {
            SIGPROC_THROW("destination rate not initialized");
        }

        if (m_swr_ctx==nullptr) {
            SIGPROC_THROW("context not configured");
        }

        // check if we need to reallocate space for the output
        m_dst_nb_samples = av_rescale_rnd(swr_get_delay(m_swr_ctx, m_src_rate) +
                                        frame->nb_samples, m_dst_rate, m_src_rate, AV_ROUND_UP);
        if (m_dst_nb_samples > m_max_dst_nb_samples) {
            if (m_dst_data==nullptr) {
                ret = av_samples_alloc_array_and_samples(
                    &m_dst_data, &m_dst_linesize, m_dst_nb_channels,
                    m_dst_nb_samples, m_dst_sample_fmt, 0);

            } else{
                av_freep(m_dst_data);
                ret = av_samples_alloc(
                    m_dst_data, &m_dst_linesize, m_dst_nb_channels,
                    m_dst_nb_samples, m_dst_sample_fmt, 1);
            }
            if (ret < 0) {
                throw std::runtime_error("fatal allocation error");
            }
            m_max_dst_nb_samples = m_dst_nb_samples;
        }

        ret = swr_convert(m_swr_ctx,  m_dst_data, m_dst_nb_samples,
                          (const uint8_t **)&frame->data, frame->nb_samples);
        if (ret < 0) {
            throw std::runtime_error("convert");
        }


        int dst_bufsize = av_samples_get_buffer_size(
            &m_dst_linesize, m_dst_nb_channels, ret, m_dst_sample_fmt, 1);

        push_samples<T>(buffers, m_dst_nb_channels, m_dst_linesize, *m_dst_data, dst_bufsize);

        return dst_bufsize;
    }

};

template <typename T>
class DecoderImpl
{
    const AVCodec* m_codec = NULL;
    AVCodecContext* m_codec_ctx= NULL;
    AVCodecParserContext* m_parser = NULL;
    AVFrame* m_decoded_frame = NULL;
    AVPacket* m_pkt = NULL;
    BufferedVector<uint8_t> m_input_buffer;
    std::vector<BufferedVector<T>> m_output_buffer;

    ResamplerImpl<T> m_resampler;
    AVCodecID m_audio_codec_id;
    int m_output_channels;
    int m_output_samplerate;

public:
    DecoderImpl(int format, int sample_rate, int n_channels)
        : m_input_buffer()
        , m_resampler() {

        m_output_buffer.resize(n_channels);

        // these are the three parameters that MUST be set by the user
        // a helper function could be written, given a filepath
        // or a file extension
        set_codec(format);
        m_output_channels = n_channels;
        m_output_samplerate = sample_rate;
        init();
    }

    ~DecoderImpl() {
        release();
    }

    void push_data(uint8_t* data, size_t n_elements) {
        m_input_buffer.push_back(data, n_elements);
    }

    BufferedVector<T>& output(size_t index) {
        if (index >= static_cast<size_t>(m_output_channels)) {
            SIGPROC_THROW("invalid channel index: " << index);
        }
        return m_output_buffer[index];
    }

    void decode() {

        m_input_buffer.reserve(AV_INPUT_BUFFER_PADDING_SIZE);

        bool cont = true;
        while (cont) {
            cont = decode_packet();
        }
    }

private:

    void set_codec(int format) {
        m_audio_codec_id = AV_CODEC_ID_MP3;
    }

    void init() {

        m_codec = avcodec_find_decoder(m_audio_codec_id);
        if (!m_codec) {
            release();
            SIGPROC_THROW("failed to find codec for: " << m_audio_codec_id);
        }

        m_codec_ctx = avcodec_alloc_context3(m_codec);
        if (!m_codec_ctx) {
            release();
            throw std::runtime_error("codec_ctx");
        }

        m_pkt = av_packet_alloc();
        if (!m_pkt) {
            release();
            throw std::runtime_error("packet");
        }

        av_init_packet(m_pkt);

        m_parser = av_parser_init(m_codec->id);
        if (!m_parser) {
            release();
            throw std::runtime_error("parser");
        }

        if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
            release();
            throw std::runtime_error("open2");
        }

    }

    void release() {
        if (m_parser!=nullptr)
            av_parser_close(m_parser);
        if (m_codec_ctx!=nullptr)
            avcodec_free_context(&m_codec_ctx);
        if (m_decoded_frame!=nullptr)
            av_frame_free(&m_decoded_frame);
        if (m_pkt!=nullptr)
            av_packet_free(&m_pkt);
    }

    bool decode_packet() {

        int ret;

        if (!m_decoded_frame) {
            if (!(m_decoded_frame = av_frame_alloc())) {
                throw std::runtime_error("frame");
            }
        }

        ret = av_parser_parse2(m_parser, m_codec_ctx,
                               &m_pkt->data, &m_pkt->size,
                               m_input_buffer.data(),
                               m_input_buffer.size(),
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (ret < 0) {
            throw std::runtime_error("error while parsing");
        }

        // erase the consumed elements
        m_input_buffer.erase(ret);

        if (m_pkt->size > 0) {
            decode_frame();
        }

        return m_pkt->size > 0;
    }

    void decode_frame() {

        int ret;

        ret = avcodec_send_packet(m_codec_ctx, m_pkt);
        if (ret < 0) {
            return;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(m_codec_ctx, m_decoded_frame);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                return;
            }
            else if (ret < 0) {
                return;
            }

            // first time through with a valid frame,
            // configure the resampler
            if (!m_resampler.is_configured()) {
                m_resampler.set_input_opts(m_codec_ctx, m_decoded_frame);
                m_resampler.set_output_opts(m_output_channels, m_output_samplerate);
                m_resampler.configure();
            }

            // resample, writing output to the given buffer
            // I pretty much always need a resampler, the codec
            // controls the output format (int, float, planar, interleaved)
            // the resampler allows for changing the input from
            // any format to the requested format.
            m_resampler.resample(m_decoded_frame, m_output_buffer);

            /*
            // by default the mp3 codec outputs planar floats
            // this is an example of copying planar floats out as
            // interleaved data
            int data_size = av_get_bytes_per_sample(m_codec_ctx->sample_fmt);

            for (int i = 0; i < m_decoded_frame->nb_samples; i++) {
                for (int ch = 0; ch < m_codec_ctx->channels; ch++) {
                    fwrite(m_decoded_frame->data[ch] + data_size*i, 1, data_size, m_tempfile);
                }
            }
            */

        }
    }
};

template <typename T>
Decoder<T>::Decoder(int format, int sample_rate, int n_channels)
    : m_impl(new DecoderImpl<T>(format, sample_rate, n_channels))
{
}

template <typename T>
Decoder<T>::~Decoder()
{

}

template <typename T>
void Decoder<T>::push_data(uint8_t* data, size_t n_elements)
{
    m_impl->push_data(data, n_elements);
    m_impl->decode();
}

template <typename T>
size_t Decoder<T>::output_size(size_t index) const
{
    return m_impl->output(index).size();
}

template <typename T>
const T* Decoder<T>::output_data(size_t index) const
{
    return m_impl->output(index).data();
}

template <typename T>
void Decoder<T>::output_erase(size_t index, size_t n_elements)
{
    m_impl->output(index).erase(n_elements);
}


/**
 * FFmpegInit handles one time operations required for FFmpeg to function
 */
class FFmpegInit
{
public:
    FFmpegInit() {
        //avcodec_init();
        // disable ffmpeg logging
        av_log_set_level(0);
        // register all codecs
        avcodec_register_all();
    }
    ~FFmpegInit() = default;
};

FFmpegInit _init;

template class Decoder<uint8_t>;
template class Decoder<int16_t>;
template class Decoder<float>;
template class Decoder<double>;

        } // ffmpeg
    } // bell
} // sigproc