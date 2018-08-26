

#ifndef SIGPROC_BELL_BASE_HPP
#define SIGPROC_BELL_BASE_HPP

#include <vector>
#include <iostream>
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace bell {

enum class TransformKind : char {
    UNKNOWN      = 0,
    FORWARD      = 1, // DFT
    REVERSE      = 2, // Inverse DFT
    MAGNITUDE    = 3, //forward transform, normalized by N/2
    LOGMAGNITUDE = 4, // log of forward transform, normalized by N/2
    DCTII        = 4  //
};

template<typename T>
class TransformBase
{
public:
    TransformBase() {}
    virtual ~TransformBase() {}

    virtual size_t size() const =0;

    virtual T* inputBegin() =0;
    virtual T* inputEnd() =0;
    virtual T* outputBegin() =0;
    virtual T* outputEnd() =0;

    virtual void execute() =0;

};


template <typename T>
TransformBase<T>* newRealTransform(TransformKind kind, size_t Fs, size_t N) {
    SIGPROC_THROW("invalid transform type");
}

template <>
TransformBase<float>* newRealTransform(TransformKind kind, size_t Fs, size_t N);

template <>
TransformBase<double>* newRealTransform(TransformKind kind, size_t Fs, size_t N);

template <typename T>
TransformBase<T>* newCosineTransform(TransformKind kind, size_t N, size_t K) {
    SIGPROC_THROW("invalid transform type");
}

template <>
TransformBase<float>* newCosineTransform(TransformKind kind, size_t N, size_t K);

template <>
TransformBase<double>* newCosineTransform(TransformKind kind, size_t N, size_t K);

    } // bell
} // sigproc

std::ostream& operator << (std::ostream& os, const sigproc::bell::TransformKind& kind);

#endif