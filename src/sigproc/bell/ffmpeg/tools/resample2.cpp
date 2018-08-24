

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <algorithm>


#include "sigproc/bell/ffmpeg/decode.hpp"
#include "sigproc/bell/algorithm/directform.hpp"
#include "sigproc/bell/algorithm/zbfilter.hpp"
using namespace sigproc::bell;
using namespace sigproc::bell::algorithm;

#define SAMPLE_RATE 20000
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

void resample(std::istream* fin, std::ostream* fout, ffmpeg::Decoder& decoder)
{
    constexpr size_t data_capacity = 2048;
    uint8_t data[data_capacity];
    float fdata[16384];
    size_t data_length;

    std::vector<float> B;
    std::vector<float> A;
    zbpeqfilter<float>(B, A, 2000, SAMPLE_RATE, 300, 3);
    DirectFormII<float> df2(B, A);

    while (fin->good()) {
        fin->read(reinterpret_cast<char*>(data), data_capacity);
        decoder.push_data(data, fin->gcount());

        if (decoder.output_size()>0) {
            const size_t N = decoder.output_size()/2;
            const int16_t* idata = reinterpret_cast<const int16_t*>(decoder.output_data());
            for (size_t i=0; i<N; i++) {
                fdata[i] = idata[i]/32768.0;
            }

            df2.filt(fdata, decoder.output_size()/2);

            for (size_t i=0; i<N; i++) {
                float f = fdata[i];
                f = (f>.99)?.99:(f<-.99)?-.99:f;
                //uint16_t t = static_cast<uint16_t>(32000.0 * fmax(fmin(fdata[i], 1.0), -1.0));
                int16_t t = static_cast<int16_t>(32000.0 * f);
                //uint16_t t = static_cast<uint16_t>(32767.0 * fdata[i]);
                fout->write(reinterpret_cast<char*>(&t), sizeof(int16_t));
            }

            //fout->write(reinterpret_cast<const char*>(decoder.output_data()),
            //            decoder.output_size());
            decoder.output_erase(decoder.output_size());
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

    if (args.size() > 1 && args[1] != "-") {
        fin = new std::ifstream(args[1].c_str());
    }

    if (args.size() > 2 && args[2] != "-") {
        fout = new std::ofstream(args[2].c_str());
    }

    try {
        ffmpeg::Decoder decoder(0, SAMPLE_RATE, 1);

        write_wave_header(1, SAMPLE_RATE, *fout);
        resample(fin, fout, decoder);

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
