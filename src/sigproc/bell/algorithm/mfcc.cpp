
#include <cmath>


#include "sigproc/bell/algorithm/mfcc.hpp"

namespace sigproc {
    namespace bell {
        namespace algorithm {
            namespace mfcc {

double freq2mel(double freq) {
    return 1127 * log(1.0 + freq/700.0);
}

double mel2freq(double mel) {
    return 700.0*(exp((mel/1127.0))-1);
}

size_t freq2idx(double Fs, size_t N, double freq) {
    // ensure that the returned integer is less than N
    return static_cast<size_t>(freq/Fs*N);
}

double idx2freq(double Fs, size_t N, size_t idx) {
    return static_cast<size_t>(idx)/N*Fs;
}

double freq2midi(double freq, int tonesPerOctave, int middleAHz) {
    return tonesPerOctave*(5.75 + log2(freq/middleAHz));
}

double midi2freq(double midi, int tonesPerOctave, int middleAHz) {
    return middleAHz*pow(2.0,midi/tonesPerOctave - 5.75);
}



            } // mfcc
        } // algorithm
    } // bell
} // sigproc