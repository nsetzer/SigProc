

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstring>

#include "sigproc/bell/ffmpeg/decode.hpp"
#include "sigproc/bell/algorithm/mfcc.hpp"

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

void do_framegen(std::istream* fin, std::ostream* fout,
    ffmpeg::Decoder& decoder, algorithm::mfcc::FeatureGenerator<double>& frameGen)
{
    constexpr size_t data_capacity = 2048;
    uint8_t data[data_capacity];
    size_t data_length;

    std::vector<double> frame;
    std::vector<double> fdata;
    fdata.resize(frameGen.windowSize());

    int32_t kSampleRate = 16000;
    int32_t kFrameSize = 40;
    int32_t kFrameStep = 160;
    int32_t kUnused = 0;
    fout->write(reinterpret_cast<const char*>(&kSampleRate), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kFrameSize), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kFrameStep), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kUnused), sizeof(int32_t));

    size_t count = 0;
    while (fin->good()) {
        fin->read(reinterpret_cast<char*>(data), data_capacity);
        decoder.push_data(data, fin->gcount());

        while (decoder.output_size()/2 >= frameGen.windowSize()) {
            const int16_t* idata = reinterpret_cast<const int16_t*>(decoder.output_data());
            for (size_t i=0; i<frameGen.windowSize(); i++) {
                fdata[i] = idata[i]/32768.0;
            }

            frameGen.filter(fdata, frame);

            decoder.output_erase(frameGen.frameStep()*2);
            count ++;

            fout->write(reinterpret_cast<const char*>(&frame[0]),
                        sizeof(double)*frame.size());
        }
    }

    std::cout << "produce " << count << " frames" << std::endl;
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
        ffmpeg::Decoder decoder(0, 16000, 1);
        algorithm::mfcc::FeatureGenerator<double> frameGen;

        do_framegen(fin, fout, decoder, frameGen);

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
