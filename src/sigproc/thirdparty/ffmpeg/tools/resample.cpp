

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "sigproc/thirdparty/ffmpeg/decode.hpp"

using namespace sigproc::thirdparty;

int main(int argc, char* argv[])
{
    constexpr size_t data_capacity = 2048;
    uint8_t data[data_capacity];
    size_t data_length;
    std::istream* fin = &std::cin;
    std::ostream* fout = &std::cout;
    ffmpeg::Decoder decoder(0, 16000, 1);

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

    while (fin->good()) {
        fin->read(reinterpret_cast<char*>(data), data_capacity);
        decoder.push_data(data, fin->gcount());

        if (decoder.output_size()>0) {
            fout->write(reinterpret_cast<const char*>(decoder.output_data()),
                        decoder.output_size());
            decoder.output_erase(decoder.output_size());
        }
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