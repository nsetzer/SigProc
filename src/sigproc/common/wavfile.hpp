
#ifndef SIGPROC_COMMON_WAVFILE_HPP
#define SIGPROC_COMMON_WAVFILE_HPP

#include <iostream>
#include <fstream>

namespace sigproc {
    namespace common {

class WavStreamWriter
{
    std::ostream* m_stream;
    int32_t m_sample_rate;
    int16_t m_num_channels;
public:
    WavStreamWriter(int32_t sample_rate, int16_t num_channels);
    WavStreamWriter(std::ostream* stream, int32_t sample_rate, int16_t num_channels);
    ~WavStreamWriter();

    void set_stream(std::ostream* stream) {
        m_stream = stream;
        write_header();
    };

    size_t write(const char* buf, size_t length);
    size_t write(const int16_t* buf, size_t length);

private:
    void write_header();

};

// on close, seek to the begining and rewrite the header
class WavFileWriter
{
    WavStreamWriter m_writer;
    std::ofstream* m_pFile;
public:
    WavFileWriter(const std::string& path, int32_t sample_rate, int16_t num_channels);
    ~WavFileWriter();

    size_t write(const char* buf, size_t length) {
        return m_writer.write(buf, length);
    }

    size_t write(const int16_t* buf, size_t length) {
        return m_writer.write(buf, length);
    }
};


class WavStreamReader
{
    int16_t m_fmt = 0;
    int16_t m_bits_per_sample = 0;
    int32_t m_file_size = 0;
    int32_t m_data_size = 0;
    int32_t m_bitrate = 0;
    int16_t m_block_align = 0;
    int32_t m_sample_rate = 0;
    int16_t m_num_channels = 0;

    std::istream* m_stream;
public:
    WavStreamReader();
    WavStreamReader(std::istream* stream);
    ~WavStreamReader();

    void set_stream(std::istream* stream) {
        m_stream = stream;
        read_header();
    };

    size_t read(char* buf, size_t length);
    size_t read(int16_t* buf, size_t length);

    int32_t sample_rate() { return m_sample_rate; }
    int16_t num_channels() { return m_num_channels; }
    int16_t byte_rate() { return m_bits_per_sample >> 3; }

private:
    void read_header();
};

class WavFileReader
{
    WavStreamReader m_reader;
    std::ifstream* m_pFile;
public:
    WavFileReader(const std::string& path);
    ~WavFileReader();

    size_t read(char* buf, size_t length) {
        return m_reader.read(buf, length);
    }

    size_t read(int16_t* buf, size_t length) {
        return m_reader.read(buf, length);
    }

    int32_t sample_rate() { return m_reader.sample_rate(); }
    int16_t num_channels() { return m_reader.num_channels(); }
    int16_t byte_rate() { return m_reader.byte_rate(); }

};

    } // common
} // sigproc

#endif
