

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstring>

#include "sigproc/bell/base.hpp"

using namespace sigproc::bell;

template<typename T>
void fwrite(std::ostream& fout, const T* ptr) {
    fout.write(reinterpret_cast<const char*>(ptr), sizeof(T));
}

template<>
void fwrite<char>(std::ostream& fout, const char* ptr) {
    fout.write(ptr, strlen(ptr));
}

void write_wave_header(int16_t num_channels, int32_t sample_rate, std::ostream& fout) {

    constexpr int16_t kPcm = 0x01;
    constexpr int16_t kBitsPerSample = 16;
    constexpr int32_t kBitrate = 16;
    constexpr int32_t kFileSize = 0x0FFFFFFF + 1;
    constexpr int32_t kHeaderSize = 44;
    constexpr int32_t kDataSize = kFileSize - kHeaderSize + 8;

    const int32_t bitrate = (num_channels * sample_rate * kBitrate) >> 3;
    const int16_t block_align = 2 * num_channels;

    fwrite(fout, "RIFF");
    fwrite(fout, &kFileSize);
    fwrite(fout, "WAVE");
    fwrite(fout, "fmt ");
    fwrite(fout, &kBitrate); // subchunk size
    fwrite(fout, &kPcm);
    fwrite(fout, &num_channels);
    fwrite(fout, &sample_rate);
    fwrite(fout, &bitrate);
    fwrite(fout, &block_align);
    fwrite(fout, &kBitsPerSample);
    fwrite(fout, "data");
    fwrite(fout, &kDataSize);
}

void resample(std::istream* fin, std::ostream* fout, AudioDecoderBase<double>& decoder)
{
    constexpr size_t data_capacity = 2048;
    uint8_t data[data_capacity];
    size_t data_length;

    while (fin->good()) {
        fin->read(reinterpret_cast<char*>(data), data_capacity);
        decoder.push_data(data, fin->gcount());

        size_t n_elements = decoder.output_size(0);
        if (n_elements>0) {
            fout->write(reinterpret_cast<const char*>(decoder.output_data(0)),
                        n_elements);
            decoder.output_erase(0, n_elements);
        }
    }
}

int main(int argc, char* argv[])
{

    std::istream* fin = &std::cin;
    std::ostream* fout = &std::cout;

    std::vector<std::string> args;
    for (int i=0; i<argc; i++) {
        args.push_back(argv[i]);
    }

    if (args.size() != 3) {
        std::cerr << "usage: $0 in out" << std::endl;
        return -1;
    }

    fin = new std::ifstream(args[1].c_str());
    fout = new std::ofstream(args[2].c_str());
    //if (args.size() > 1 && args[1] != "-") {
    //    fin = new std::ifstream(args[1].c_str());
    //}

    //if (args.size() > 2 && args[2] != "-") {
    //    fout = new std::ofstream(args[2].c_str());
    //}

    try {
        AudioDecoderBase<double>* pDecoder = newAudioDecoderFromPath<double>(args[1], 16000, 1);

        write_wave_header(1, 16000, *fout);
        resample(fin, fout, *pDecoder);

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    if (fin != &std::cin) {
        static_cast<std::ifstream*>(fin)->close();
        delete fin;
    }

    if (fout != &std::cout) {
        static_cast<std::ofstream*>(fout)->close();
        delete fout;
    }
}


