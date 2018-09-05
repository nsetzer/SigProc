
#include <iostream>
#include <vector>
#include <cstring>

#include "sigproc/bell/ffmpeg/decode.hpp"
#include "sigproc/common/format.hpp"
#include "sigproc/common/exception.hpp"
#include "sigproc/common/wavfile.hpp"

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

namespace {

AVCodecID getFormat(AudioFormat format) {

    switch (format) {

    case AudioFormat::PCMS16LE:
        std::cerr << " format s16le " << std::endl;
        return AV_CODEC_ID_PCM_S16LE;
    case AudioFormat::WAV:
    std::cerr << " format wav " << std::endl;
        return AV_CODEC_ID_PCM_S16LE;
    case AudioFormat::MP2:
    case AudioFormat::MP3:
    case AudioFormat::MP4:
        std::cerr << " format mp3 " << AV_CODEC_ID_MP3 << std::endl;
        return AV_CODEC_ID_MP3;
    case AudioFormat::FLAC:
        std::cerr << " format flac " << AV_CODEC_ID_FLAC << std::endl;
        return AV_CODEC_ID_FLAC;
    case AudioFormat::M4A:
    case AudioFormat::AAC:
    case AudioFormat::UNKNOWN:
    default:
        std::cerr << " format unkown " << std::endl;
        SIGPROC_THROW("ffmpeg: unsupported format");
    }
}

} // anonymous

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
        if (m_index > m_data.size()) {
            m_index = m_data.size();
        }
    }

    void reserve(size_t overhead) {
        size_t cap = m_data.size() + overhead;
        m_data.reserve(cap);
    }

    template<typename VALUE_TYPE>
    bool read(VALUE_TYPE* ptr) {
        if (size() < sizeof(VALUE_TYPE)) {
            return false;
        }

        memcpy(ptr, &m_data[m_index], sizeof(VALUE_TYPE));
        m_index += sizeof(VALUE_TYPE);

        return true;
    }

    bool read(char* ptr, size_t n) {
        if (size() < n) {
            return false;
        }
        memcpy(ptr, &m_data[m_index], n);
        m_index += n;
        return true;
    }

    size_t index() {
        return m_index;
    }

    // set and get old value
    size_t index(size_t new_index) {
        size_t old_index = m_index;
        if (new_index > m_data.size()) {
            m_index = m_data.size();
        } else {
            m_index = new_index;
        }
        return old_index;
    }

};

void getFileCodecKind(const char* filepath) {
    // https://rodic.fr/blog/libavcodec-tutorial-decode-audio-file/

    int err=0;
    AVInputFormat *iformat = nullptr;
    AVFormatContext *fmt_ctx = NULL;
    AVDictionary *format_opts = NULL;
    AVCodecID audio_codec_id = AV_CODEC_ID_NONE;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        std::cerr << "failed to allocate context" << std::endl;
        avformat_free_context(fmt_ctx);
        return;
    }

    if ((err = avformat_open_input(
        &fmt_ctx, filepath, iformat, &format_opts)) < 0) {
        std::cerr << "failed  open input context: " << err << std::endl;
        avformat_free_context(fmt_ctx);
        return;
    }

    iformat = fmt_ctx->iformat;

    if (iformat==nullptr) {
        std::cerr << "no format:" << std::endl;
        avformat_free_context(fmt_ctx);
        return;
    }

    std::cout << "flags: " << iformat->flags << std::endl;
    std::cout << "name: " << iformat->name << std::endl;
    std::cout << "long_name: " << iformat->long_name << std::endl;
    std::cout << "audio_codec_id: " << fmt_ctx->audio_codec_id << std::endl;

    for (size_t i = 0; i < fmt_ctx->nb_streams; ++i) {
        AVCodecContext* tc = fmt_ctx->streams[i]->codec;
        std::cout << "codec_type: " << tc->codec_type << std::endl;
        std::cout << "codec_id: " << tc->codec_id << std::endl;
    }

    avformat_free_context(fmt_ctx);
}

template<typename T>
enum AVSampleFormat get_sample_format() {
    SIGPROC_THROW("ffmpeg: unsupported type");
}

// return interleaved PCM data
template<>
enum AVSampleFormat get_sample_format<uint8_t>() {
    std::cerr << "sample format: pcm interleaved" << std::endl;
    return AV_SAMPLE_FMT_S16;
}

// return planar 16 bit data
template<>
enum AVSampleFormat get_sample_format<int16_t>() {
    std::cerr << "sample format: pcm planar" << std::endl;
    return AV_SAMPLE_FMT_S16P;
}

template<>
enum AVSampleFormat get_sample_format<float>() {
    std::cerr << "sample format: float planar" << std::endl;
    return AV_SAMPLE_FMT_FLTP;
}

template<>
enum AVSampleFormat get_sample_format<double>() {
    std::cerr << "sample format: double planar" << std::endl;
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
    SIGPROC_THROW("ffmpeg: unsupported type");
}

template<>
void push_samples<uint8_t>(std::vector<BufferedVector<uint8_t>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are intentionally ignored for this type
    // write some documentation,
    // use a traits function for allocating the buffer array correctly
    //std::cout << " uint8_t linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
    buffers[0].push_back(data, datalen);
}

template<>
void push_samples<int16_t>(std::vector<BufferedVector<int16_t>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    //std::cout << " int16_t linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
    const int16_t* cdata = reinterpret_cast<const int16_t*>(data);
    int datasize = datalen / sizeof(int16_t);
    buffers[0].push_back(cdata, datasize);
    //float rms = 0.0;
    //for (int i =0; i < datasize; i++) {
    //    float v = cdata[i] / 32768.0;
    //    rms += v*v;
    //}
    //std::cout << "rms: " << rms << std::endl;
}

template<>
void push_samples<float>(std::vector<BufferedVector<float>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    //std::cout << " float linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
    const float* cdata = reinterpret_cast<const float*>(data);
    int datasize = datalen / sizeof(float);
    buffers[0].push_back(cdata, datasize);
}

template<>
void push_samples<double>(std::vector<BufferedVector<double>>& buffers, int nb_channels, int linesize, const uint8_t* data, int datalen)
{
    // TODO channels are ignored for this type, and they should not be
    //std::cout << " double linesize: " << linesize << " channels: " << nb_channels << " data_size: " << datalen << std::endl;
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
        m_src_rate = 0;
        m_dst_rate = 0;
        m_dst_linesize = 0;
        m_src_nb_samples = 0;
        m_dst_nb_samples = 0;
        m_max_dst_nb_samples = 0;
    }
    ~ResamplerImpl() {}

    // initialize the resampler to accept interleaved PCM data
    void set_input_opts(int n_channels, int sample_rate) {
        m_src_nb_channels = n_channels;
        if (m_src_nb_channels > 2) {
            SIGPROC_THROW("ffmpeg: invalid number of channels");
        }
        m_src_rate = sample_rate;
        if (m_src_rate < 100) {
            SIGPROC_THROW("ffmpeg: source rate lower than expected: " << m_src_rate);
        }
        m_src_ch_layout = (m_src_nb_channels==2)?AV_CH_LAYOUT_STEREO:AV_CH_LAYOUT_MONO;
        m_src_sample_fmt = AV_SAMPLE_FMT_S16;
    }

    void set_input_opts(AVCodecContext *ctx, AVFrame *frame) {
        m_src_nb_channels = ctx->channels;
        if (m_src_nb_channels > 2) {
            SIGPROC_THROW("ffmpeg: invalid number of channels");
        }
        m_src_rate = frame->sample_rate;
        if (m_src_rate < 100) {
            SIGPROC_THROW("ffmpeg: source rate lower than expected: " << m_src_rate);
        }
        m_src_ch_layout = (m_src_nb_channels==2)?AV_CH_LAYOUT_STEREO:AV_CH_LAYOUT_MONO;
        m_src_sample_fmt = static_cast<AVSampleFormat>(frame->format);
    }

    void set_output_opts(int n_channels, int sample_rate) {
        m_dst_nb_channels = n_channels;
        if (m_dst_nb_channels > 2) {
            SIGPROC_THROW("ffmpeg: invalid number of channels");
        }
        m_dst_rate = sample_rate;
        if (m_dst_rate < 100) {
            SIGPROC_THROW("ffmpeg: source rate lower than expected: " << m_src_rate);
        }
        m_dst_ch_layout = (n_channels==2)?AV_CH_LAYOUT_STEREO:AV_CH_LAYOUT_MONO;
        m_dst_sample_fmt = get_sample_format<T>();
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
            SIGPROC_THROW("ffmpeg: m_swr_ctx alloc");
        }

        av_opt_set_int(m_swr_ctx, "in_channel_layout",    m_src_ch_layout, 0);
        av_opt_set_int(m_swr_ctx, "in_sample_rate",       m_src_rate, 0);
        av_opt_set_sample_fmt(m_swr_ctx, "in_sample_fmt", m_src_sample_fmt, 0);

        av_opt_set_int(m_swr_ctx, "out_channel_layout",    m_dst_ch_layout, 0);
        av_opt_set_int(m_swr_ctx, "out_sample_rate",       m_dst_rate, 0);
        av_opt_set_sample_fmt(m_swr_ctx, "out_sample_fmt", m_dst_sample_fmt, 0);
        //av_opt_set_int(m_swr_ctx, "filter_type", SWR_FILTER_TYPE_CUBIC, 0);

        if ((ret = swr_init(m_swr_ctx)) < 0) {
            SIGPROC_THROW("ffmpeg: m_swr_ctx init");
        }

        m_max_dst_nb_samples = 0;
        m_dst_nb_samples = 0;

        std::cerr << "inp: m_src_nb_channels " << m_src_nb_channels
            << " m_src_ch_layout " << m_src_ch_layout
            << " m_src_rate " << m_src_rate
            << " m_src_sample_fmt " << m_src_sample_fmt << std::endl;
        std::cerr << "inp: m_dst_nb_channels " << m_dst_nb_channels
            << " m_dst_ch_layout " << m_dst_ch_layout
            << " m_dst_rate " << m_dst_rate
            << " m_dst_sample_fmt " << m_dst_sample_fmt << std::endl;

        return true;
    }

    bool is_configured() {
        return m_swr_ctx != nullptr;
    }

    size_t resample(AVFrame *frame, std::vector<BufferedVector<T>>& buffers) {
        int ret;

        if (m_src_rate==0) {
            SIGPROC_THROW("ffmpeg: source rate not initialized");
        }

        if (m_dst_rate==0) {
            SIGPROC_THROW("ffmpeg: destination rate not initialized");
        }

        if (m_swr_ctx==nullptr) {
            SIGPROC_THROW("ffmpeg: context not configured");
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
                SIGPROC_THROW("ffmpeg: fatal allocation error");
            }
            m_max_dst_nb_samples = m_dst_nb_samples;
        }

        ret = swr_convert(m_swr_ctx,  m_dst_data, m_dst_nb_samples,
                          (const uint8_t **)&frame->data, frame->nb_samples);
        if (ret < 0) {
            SIGPROC_THROW("ffmpeg: convert");
        } else if (ret > 0) {

            int dst_bufsize = av_samples_get_buffer_size(
            &m_dst_linesize, m_dst_nb_channels, ret, m_dst_sample_fmt, 1);

            if (dst_bufsize < 0) {
                SIGPROC_THROW("ffmpeg: error samples "
                    << " m_dst_linesize " << m_dst_linesize
                    << " m_dst_nb_channels " << m_dst_nb_channels
                    << " ret " << ret
                );
            }

            push_samples<T>(buffers, m_dst_nb_channels, m_dst_linesize, *m_dst_data, dst_bufsize);

            return dst_bufsize;

        } else {
            return 0;
        }
    }

    size_t resample(const uint8_t* frame_data, size_t nb_frame_samples, std::vector<BufferedVector<T>>& buffers) {
        int ret;

        if (m_src_rate==0) {
            SIGPROC_THROW("ffmpeg: source rate not initialized");
        }

        if (m_dst_rate==0) {
            SIGPROC_THROW("ffmpeg: destination rate not initialized");
        }

        if (m_swr_ctx==nullptr) {
            SIGPROC_THROW("ffmpeg: context not configured");
        }

        // check if we need to reallocate space for the output
        m_dst_nb_samples = av_rescale_rnd(swr_get_delay(m_swr_ctx, m_src_rate) +
                                        nb_frame_samples, m_dst_rate, m_src_rate, AV_ROUND_UP);
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
                SIGPROC_THROW("ffmpeg: fatal allocation error");
            }
            m_max_dst_nb_samples = m_dst_nb_samples;
        }

        ret = swr_convert(m_swr_ctx,  m_dst_data, m_dst_nb_samples,
                          &frame_data, nb_frame_samples);
        if (ret < 0) {
            SIGPROC_THROW("ffmpeg: convert");
        } else if (ret > 0) {

            int dst_bufsize = av_samples_get_buffer_size(
            &m_dst_linesize, m_dst_nb_channels, ret, m_dst_sample_fmt, 1);

            if (dst_bufsize < 0) {
                SIGPROC_THROW("ffmpeg: error samples "
                    << " m_dst_linesize " << m_dst_linesize
                    << " m_dst_nb_channels " << m_dst_nb_channels
                    << " ret " << ret
                );
            }

            push_samples<T>(buffers, m_dst_nb_channels, m_dst_linesize, *m_dst_data, dst_bufsize);

            return dst_bufsize;

        } else {
            return 0;
        }

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

    FILE *m_tmpfile=NULL;

public:
    DecoderImpl(AVCodecID format, int sample_rate, int n_channels)
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
            SIGPROC_THROW("ffmpeg: invalid channel index: " << index);
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

    void set_codec(AVCodecID format) {
        std::cerr << "set codec: " << format << std::endl;
        m_audio_codec_id = format; // format;
    }

    void init() {

        m_tmpfile = fopen("./out.raw", "wb");
        if (!m_tmpfile) {
            SIGPROC_THROW("ffmpeg: ./out.raw");
        }

        m_codec = avcodec_find_decoder(m_audio_codec_id);
        if (!m_codec) {
            release();
            SIGPROC_THROW("ffmpeg: failed to find codec for: " << m_audio_codec_id);
        }

        m_codec_ctx = avcodec_alloc_context3(m_codec);
        if (!m_codec_ctx) {
            release();
            SIGPROC_THROW("ffmpeg: codec_ctx");
        }

        m_pkt = av_packet_alloc();
        if (!m_pkt) {
            release();
            SIGPROC_THROW("ffmpeg: packet");
        }

        av_init_packet(m_pkt);

        m_parser = av_parser_init(m_codec->id);
        if (!m_parser) {
            release();
            SIGPROC_THROW("ffmpeg: parser " << m_codec->id);
        }

        if (avcodec_open2(m_codec_ctx, m_codec, NULL) < 0) {
            release();
            SIGPROC_THROW("ffmpeg: open2");
        }

        if (!(m_decoded_frame = av_frame_alloc())) {
            release();
            SIGPROC_THROW("ffmpeg: frame");
        }

    }

    void release() {
        if (m_tmpfile!=nullptr) {
            fclose(m_tmpfile);
        }
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

        //if (!m_decoded_frame) {
        //    if (!(m_decoded_frame = av_frame_alloc())) {
        //        SIGPROC_THROW("ffmpeg: frame");
        //    }
        //}

        ret = av_parser_parse2(m_parser, m_codec_ctx,
                               &m_pkt->data, &m_pkt->size,
                               m_input_buffer.data(),
                               m_input_buffer.size(),
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (ret < 0) {
            SIGPROC_THROW("ffmpeg: error while parsing");
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
            fmt::osprintf(std::cerr, "avcodec error: unable to send packet: %d\n", ret);
            return;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(m_codec_ctx, m_decoded_frame);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                //std::cerr << "return value a: " << ret << std::endl;
                return;
            }
            else if (ret < 0) {
                fmt::osprintf(std::cerr, "avcodec error: unable to receive frame: %d\n", ret);
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


            // by default the mp3 codec outputs planar floats
            // this is an example of copying planar floats out as
            // interleaved data
            /*
            int data_size = av_get_bytes_per_sample(m_codec_ctx->sample_fmt);

            for (int i = 0; i < m_decoded_frame->nb_samples; i++) {
                for (int ch = 0; ch < m_codec_ctx->channels; ch++) {
                    fwrite(m_decoded_frame->data[ch] + data_size*i, 1, data_size, m_tmpfile);
                }
            }
            */


        }
    }
};

template <typename T>
Decoder<T>::Decoder(AudioFormat format, int sample_rate, int n_channels)
    : m_impl(new DecoderImpl<T>(getFormat(format), sample_rate, n_channels))
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



template <typename T>
class WavDecoderImpl
{

    BufferedVector<uint8_t> m_input_buffer;
    std::vector<BufferedVector<T>> m_output_buffer;
    int32_t m_input_samplerate = 0;
    int16_t m_input_channels = 0;

    ResamplerImpl<T> m_resampler;
    int m_output_channels;
    int m_output_samplerate;

public:
    WavDecoderImpl(int sample_rate, int n_channels)
        : m_input_buffer()
        , m_resampler() {
        m_output_buffer.resize(n_channels);
        m_output_channels = n_channels;
        m_output_samplerate = sample_rate;
    }

    ~WavDecoderImpl() {
    }

    void push_data(uint8_t* data, size_t n_elements) {
        m_input_buffer.push_back(data, n_elements);
    }

    BufferedVector<T>& output(size_t index) {
        if (index >= static_cast<size_t>(m_output_channels)) {
            SIGPROC_THROW("ffmpeg: invalid channel index: " << index);
        }
        return m_output_buffer[index];
    }

    void decode() {
        decode_header();
        decode_packet();
    }

private:
    void decode_packet() {

        const uint8_t* data = m_input_buffer.data();
        size_t size = m_input_buffer.size();
        if (size%2==1) {
            size -= 1;
        }
        m_resampler.resample(data, size>>1, m_output_buffer);
        m_input_buffer.erase(size);

    }
    void decode_header() {

        if (!m_resampler.is_configured()) {
            size_t index = m_input_buffer.index();
            if (decode_header_impl()) {
                m_resampler.set_input_opts(m_input_channels, m_input_samplerate);
                m_resampler.set_output_opts(m_output_channels, m_output_samplerate);
                m_resampler.configure();
            } else {
                // not enough data to read the header, reset
                m_input_buffer.index(index);
            }
        }
    }

    bool decode_header_impl() {

        int16_t fmt = 0;
        int16_t bits_per_sample = 0;
        int32_t file_size = 0;
        int32_t data_size = 0;
        int32_t bitrate = 0;
        int16_t block_align = 0;

        char s[5];
        s[4]=0;
        int32_t offset;
        int32_t chunksize = 0;

        if(!m_input_buffer.read(s, 4)){ return false; }
        if (memcmp("RIFF", s, 4) != 0) {
            SIGPROC_THROW("ffmpeg: Invalid Wave Header: not a riff file " << s);
        }

        if(!m_input_buffer.read(&file_size)){ return false; }

        if(!m_input_buffer.read(s, 4)){ return false; }
        if (memcmp("WAVE", s, 4) != 0) {
            SIGPROC_THROW("ffmpeg: Invalid Wave Header: wave not found");
        }

        if(!m_input_buffer.read(s, 4)){ return false; }
        // while s is not 'fmt ', read int32_t and seek that many bytes
        while (memcmp("fmt ", s, 4) != 0) {
            fmt::osprintf(std::cerr, "skipping section %C\n", s);
            // skip sections until the format section is found
            if(!m_input_buffer.read(&offset)){ return false; }
            m_input_buffer.index(m_input_buffer.index()+offset);
            if(!m_input_buffer.read(s, 4)){ return false; }
        }

        if(!m_input_buffer.read(&chunksize)){ return false; }
        if(!m_input_buffer.read(&fmt)){ return false; }
        if (fmt != 0x01) {
            SIGPROC_THROW("ffmpeg: Invalid Wave Header: format is not PCM");
        }

        if(!m_input_buffer.read(&m_input_channels)){ return false; }
        if(!m_input_buffer.read(&m_input_samplerate)){ return false; }
        if(!m_input_buffer.read(&bitrate)){ return false; }
        if(!m_input_buffer.read(&block_align)){ return false; }
        if(!m_input_buffer.read(&bits_per_sample)){ return false; }

        if(!m_input_buffer.read(s, 4)){ return false; }
        if (memcmp("data", s, 4) != 0) {
            SIGPROC_THROW(fmt::sprintf("ffmpeg: Invalid Wave Header: data not found %C", s));
        }
        if(!m_input_buffer.read(&data_size)){ return false; }

        return true;
    }
};

template <typename T>
WavDecoder<T>::WavDecoder(int sample_rate, int n_channels)
    : m_impl(new WavDecoderImpl<T>(sample_rate, n_channels))
{
}

template <typename T>
WavDecoder<T>::~WavDecoder()
{

}

template <typename T>
void WavDecoder<T>::push_data(uint8_t* data, size_t n_elements)
{
    m_impl->push_data(data, n_elements);
    m_impl->decode();
}

template <typename T>
size_t WavDecoder<T>::output_size(size_t index) const
{
    return m_impl->output(index).size();
}

template <typename T>
const T* WavDecoder<T>::output_data(size_t index) const
{
    return m_impl->output(index).data();
}

template <typename T>
void WavDecoder<T>::output_erase(size_t index, size_t n_elements)
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


template class WavDecoder<uint8_t>;
template class WavDecoder<int16_t>;
template class WavDecoder<float>;
template class WavDecoder<double>;

        } // ffmpeg
    } // bell
} // sigproc
