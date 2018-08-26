

#ifndef SIGPROC_BELL_ALGORITHM_WINDOW_HPP
#define SIGPROC_BELL_ALGORITHM_WINDOW_HPP

#include <vector>
#include <cmath>
#include <cstddef>

namespace sigproc {
    namespace bell {
        namespace algorithm {
            namespace window {

template<typename T>
std::vector<T> sine(size_t N) {
    std::vector<T> v;
    v.reserve(N);

    for (size_t i=0; i < N; i++) {
        T t = sin(M_PI * i / (N - 1));
        v.push_back(t);
    }
    return v;
}

template<typename T>
std::vector<T> hanning(size_t N) {
    std::vector<T> v;
    v.reserve(N);

    const T a0 = 0.5;
    for (size_t i=0; i < N; i++) {
        T t = a0 - (1.0 - a0) * cos(2 * M_PI * i / (N - 1));
        v.push_back(t);
    }
    return v;
}

template<typename T>
std::vector<T> hamming(size_t N) {
    std::vector<T> v;
    v.reserve(N);

    const T a0 = 25.0/46.0;
    for (size_t i=0; i < N; i++) {
        T t = a0 - (1.0 - a0) * cos(2 * M_PI * i / (N - 1));
        v.push_back(t);
    }
    return v;
}

template<typename T>
std::vector<T> blackman(size_t N) {
    std::vector<T> v;
    v.reserve(N);

    constexpr T a = 0.16;
    constexpr T a0 = (1.0 - a) / 2.0;
    constexpr T a1 = 0.5;
    constexpr T a2 = a / 2.0;

    for (size_t i=0; i < N; i++) {
        T t1 = a1 * cos( 2.0 * M_PI * i / (N - 1));
        T t2 = a2 * cos( 4.0 * M_PI * i / (N - 1));
        T t = a0 - t1 + t2;
        v.push_back(t);
    }
    return v;
}


            } // window
        } // algorithm
    } // bell
} // sigproc

#endif