

#ifndef SIGPROC_BELL_ALGORITHM_ZBFILTER_HPP
#define SIGPROC_BELL_ALGORITHM_ZBFILTER_HPP

#include <vector>
#include <cmath>

namespace sigproc {
    namespace bell {
        namespace algorithm {

/*
    Zölzer-Boltze peak/noth parametric EQ
    http://strauss.pas.nu/zb-peq.html

    zbpeqfilter creates a 2 pole IIR band pass filter
    in this configuration the filter applies a gain to a selected
    frquency range.

    B,A : empty vectors to place filter coefficients into
    fc : center frequency of band pass filter
    fs : sample frequency (44100 Hz)
    bw : bandwidth of the band pass filter (Hz)
    gdb : gain coefficient, in DB.
*/
template<typename T>
T zbpeqfilter(std::vector<T>& B, std::vector<T>& A, T fc, T fs, T bw, T gdb)
{
    T v0 = pow(10.0,gdb/20.0);

    T h0 = v0 - 1.0;

    T ohmc = (2.0*M_PI*fc)/fs;
    T ohmw = (2.0*M_PI*bw)/fs;

    T d = -cos(ohmc);

    T ax;
    T tohm = tan(ohmw/2.0);
    if (v0 >= 1.0)
        ax = (tohm-1.0) / (tohm+1.0);
    else
        ax = (tohm-v0) / (tohm+v0);

    B.resize(3);
    A.resize(3);

    B[0] = -ax;
    B[1] = d * ( 1.0 - ax );
    B[2] = 1.0;

    A[0] = 1.0;
    // note A1 and A2 are inverted
    A[1] = -d * ( 1.0 - ax );
    A[2] = ax;

    return h0/2.0;
}


        } // algorithm
    } // bell
} // sigproc

#endif