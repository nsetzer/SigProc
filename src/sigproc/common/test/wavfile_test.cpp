#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/common/format.hpp"
#include "sigproc/common/wavfile.hpp"

using namespace sigproc::common;

SIGPROC_TEST(wavfile_write_stream_001) {
    {
        fmt::ovectorstream<char> ostream;
        WavStreamWriter wwstream(&ostream, 8000, 2);

        std::cout << ostream.vector().size() << std::endl;

        fmt::ivectorstream<char> istream(ostream.vector());
        WavStreamReader wrstream(&istream);
    }
}

SIGPROC_TEST(wavfile_read_file_001) {
    {
        WavFileReader wav("./cnn.wav");

        int16_t buf[1024];
        size_t count=0;
        size_t n=0;

        while( (n=wav.read(buf, sizeof(buf)/sizeof(int16_t)))>0 ){
            count += n;
        }

        std::cout << wav.sample_rate() << std::endl;
        std::cout << wav.num_channels() << std::endl;
        std::cout << count << std::endl;
        std::cout << (static_cast<double>(count)/wav.sample_rate()/wav.num_channels()) << std::endl;
    }
}
