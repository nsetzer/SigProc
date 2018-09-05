

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstring>

#include "sigproc/bell/algorithm/mfcc.hpp"
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

#define OUTPUT_TYPE double

size_t n_samples=0;
size_t n_frames=0;

void generate_frames(
    std::ostream* fout,
    AudioDecoderBase<OUTPUT_TYPE>& decoder,
    algorithm::mfcc::FeatureGenerator<OUTPUT_TYPE>& frameGen,
    std::vector<OUTPUT_TYPE> frame,
    bool final)
{

    while (decoder.output_size(0) >= frameGen.windowSize()) {
        const double* data = decoder.output_data(0);
        frameGen.filter(decoder.output_data(0), frame);
        decoder.output_erase(0, frameGen.frameStep());

        n_samples+=frameGen.frameStep();
        n_frames ++;

        fout->write(reinterpret_cast<const char*>(&frame[0]),
                    sizeof(double)*frame.size());
    }

    if (final) {
        // zero pad data to produce the final few frames
        while (decoder.output_size(0) > 0) {
            size_t consumed = decoder.output_size(0) >= frameGen.frameStep() ? \
                frameGen.frameStep() : decoder.output_size(0);

            std::vector<OUTPUT_TYPE> data;
            data.reserve(frameGen.windowSize());
            std::copy (decoder.output_data(0),
                decoder.output_data(0) + consumed,
                back_inserter(data));
            data.resize(frameGen.windowSize());

            fmt::osprintf(std::cerr, "data size %d %d %d\n", decoder.output_size(0), consumed, data.size());
            frameGen.filter(decoder.output_data(0), frame);
            decoder.output_erase(0, consumed);

            n_samples += consumed;
            n_frames ++;

            fout->write(reinterpret_cast<const char*>(&frame[0]),
                        sizeof(double)*frame.size());

            decoder.output_erase(0, consumed);

        }
    }

}

void resample(std::istream* fin, std::ostream* fout,
    AudioDecoderBase<OUTPUT_TYPE>& decoder,
    algorithm::mfcc::FeatureGenerator<OUTPUT_TYPE>& frameGen)
{
    constexpr size_t data_capacity = 20480;
    uint8_t data[data_capacity];
    size_t data_length;

    std::vector<double> frame;

    int32_t kSampleRate = 16000;
    int32_t kFrameSize = 40;
    int32_t kFrameStep = 160;
    int32_t kUnused = 0;
    fout->write(reinterpret_cast<const char*>(&kSampleRate), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kFrameSize), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kFrameStep), sizeof(int32_t));
    fout->write(reinterpret_cast<const char*>(&kUnused), sizeof(int32_t));

    n_samples=0;
    n_frames=0;

    while (fin->good()) {
        fin->read(reinterpret_cast<char*>(data), data_capacity);
        decoder.push_data(data, fin->gcount());
        generate_frames(fout, decoder, frameGen, frame, false);
    }
    // push one extra empty packet to flush the decoder
    {
        decoder.push_data(data, 0);
        generate_frames(fout, decoder, frameGen, frame, true);
    }

    fmt::osprintf(std::cout, "%.2f + %d unused samples\n", n_samples/16000.0, decoder.output_size(0));
    fmt::osprintf(std::cout, "produced %d frames\n", n_frames);
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

    try {
        AudioDecoderBase<OUTPUT_TYPE>* pDecoder = newAudioDecoderFromPath<OUTPUT_TYPE>(args[1], 16000, 1);
        algorithm::mfcc::FeatureGenerator<OUTPUT_TYPE> frameGen;

        resample(fin, fout, *pDecoder, frameGen);

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


