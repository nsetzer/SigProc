

#include <cstring>
#include "sigproc/common/wavfile.hpp"
#include "sigproc/common/exception.hpp"
#include "sigproc/common/format.hpp"

namespace sigproc {
    namespace common {

namespace {

template<typename T>
void fwrite(std::ostream& fout, const T* ptr) {
    fout.write(reinterpret_cast<const char*>(ptr), sizeof(T));
}

template<>
void fwrite<char>(std::ostream& fout, const char* ptr) {
    fout.write(ptr, strlen(ptr));
}

template<typename T>
bool fread(std::istream& fin, T* ptr) {
    fin.read(reinterpret_cast<char*>(ptr), sizeof(T));
    return fin.gcount()==sizeof(T);
}

size_t fread(std::istream& fin, char* ptr, size_t n) {
    fin.read(ptr, n);
    return fin.gcount();
}

} // anonymous

WavStreamWriter::WavStreamWriter(int32_t sample_rate, int16_t num_channels)
    : m_stream(nullptr)
    , m_sample_rate(sample_rate)
    , m_num_channels(num_channels)
{

}

WavStreamWriter::WavStreamWriter(std::ostream* stream, int32_t sample_rate, int16_t num_channels)
    : m_stream(stream)
    , m_sample_rate(sample_rate)
    , m_num_channels(num_channels)
{
    write_header();
}

WavStreamWriter::~WavStreamWriter()
{

}

size_t WavStreamWriter::write(const char* buf, size_t length)
{
    m_stream->write(buf, length);
    return length;
}

size_t WavStreamWriter::write(const int16_t* buf, size_t length)
{
    return write(reinterpret_cast<const char*>(buf), length<<1);
}


void WavStreamWriter::write_header()
{
    constexpr int16_t kPcm = 0x01;
    constexpr int16_t kBitsPerSample = 16;
    constexpr int32_t kBitrate = 16;
    constexpr int32_t kFileSize = 0x0FFFFFFF + 1;
    constexpr int32_t kHeaderSize = 44;
    constexpr int32_t kDataSize = kFileSize - kHeaderSize + 8;

    const int32_t bitrate = (m_num_channels * m_sample_rate * kBitrate) >> 3;
    const int16_t block_align = 2 * m_num_channels;

    fwrite(*m_stream, "RIFF");
    fwrite(*m_stream, &kFileSize);
    fwrite(*m_stream, "WAVE");
    fwrite(*m_stream, "fmt ");
    fwrite(*m_stream, &kBitrate); // subchunk size
    fwrite(*m_stream, &kPcm);
    fwrite(*m_stream, &m_num_channels);
    fwrite(*m_stream, &m_sample_rate);
    fwrite(*m_stream, &bitrate);
    fwrite(*m_stream, &block_align);
    fwrite(*m_stream, &kBitsPerSample);
    fwrite(*m_stream, "data");
    fwrite(*m_stream, &kDataSize);

}

WavFileWriter::WavFileWriter(const std::string& path, int32_t sample_rate, int16_t num_channels)
    : m_writer(sample_rate, num_channels)
{
    m_pFile = new std::ofstream(path.c_str());
    if (!(*m_pFile)) {
        delete m_pFile;
        m_pFile = nullptr;
        SIGPROC_THROW("Unable to Open: " << path);
    }
    m_writer.set_stream(m_pFile);
}

WavFileWriter::~WavFileWriter()
{
    // todo rewrite wave header
    if (m_pFile != nullptr) {
        m_pFile->close();
        delete m_pFile;
    }
}

WavStreamReader::WavStreamReader()
{

}

WavStreamReader::WavStreamReader(std::istream* stream)
    : m_stream(stream)
{
    read_header();
}

WavStreamReader::~WavStreamReader()
{

}

// read PCM samples
// in the future implement ulaw, alaw decoding
size_t WavStreamReader::read(char* buf, size_t length)
{
    m_stream->read(buf, length);
    return m_stream->gcount();
}

size_t WavStreamReader::read(int16_t* buf, size_t length)
{
    m_stream->read(reinterpret_cast<char*>(buf), length<<1);
    return m_stream->gcount()>>1;
}


void WavStreamReader::read_header()
{

    char s[5];
    s[4]=0;
    int32_t offset;
    int32_t chunksize = 0;

    fread(*m_stream, s, 4);
    if (memcmp("RIFF", s, 4) != 0) {
        SIGPROC_THROW("Invalid Wave Header: not a riff file " << s);
    }

    fread(*m_stream, &m_file_size);

    fread(*m_stream, s, 4);
    if (memcmp("WAVE", s, 4) != 0) {
        SIGPROC_THROW("Invalid Wave Header: wave not found");
    }

    fread(*m_stream, s, 4);
    // while s is not 'fmt ', read int32_t and seek that many bytes
    while (memcmp("fmt ", s, 4) != 0) {
        fmt::osprintf(std::cerr, "skipping section %C\n", s);
        // skip sections until the format section is found
        fread(*m_stream, &offset);
        m_stream->seekg(offset, m_stream->cur);
        fread(*m_stream, s, 4);
    }

    fread(*m_stream, &chunksize); // 32, this subchunk size
    fread(*m_stream, &m_fmt);     // 16
    if (m_fmt != 0x01) {
        SIGPROC_THROW("Invalid Wave Header: format is not PCM");
    }

    fread(*m_stream, &m_num_channels);     // 16
    fread(*m_stream, &m_sample_rate);      // 32
    fread(*m_stream, &m_bitrate);          //
    fread(*m_stream, &m_block_align);      //
    fread(*m_stream, &m_bits_per_sample);  //

    fread(*m_stream, s, 4);
    if (memcmp("data", s, 4) != 0) {
        SIGPROC_THROW(fmt::sprintf("Invalid Wave Header: data not found %C", s));
    }
    fread(*m_stream, &m_data_size);

   // warn m_bitrate != (m_num_channels * m_sample_rate * m_bits_per_sample) >> 3;
   // warn m_block_align != 2 * m_num_channels;
}


WavFileReader::WavFileReader(const std::string& path)
{
    m_pFile = new std::ifstream(path.c_str());
    if (!(*m_pFile)) {
        delete m_pFile;
        SIGPROC_THROW("Unable to Open: " << path);
    }
    m_reader.set_stream(m_pFile);
}

WavFileReader::~WavFileReader()
{
    if (m_pFile != nullptr) {
        m_pFile->close();
        delete m_pFile;
    }
}

    } // common
} // sigproc
