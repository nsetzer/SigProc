
#include <cmath>
#include "sigproc/bell/algorithm/dct.hpp"
#include <numeric>
namespace sigproc {
    namespace bell {
        namespace algorithm {

/*

naive implementation based off of this page:
https://en.wikipedia.org/wiki/Discrete_cosine_transform

rsplitfft, splitdct algorithm:
https://arxiv.org/pdf/cs/0703150.pdf

*/
template<typename T>
std::vector<T> dctmat_impl(size_t N, size_t k, T freqstep)
{
    std::vector<T> mat;
    mat.resize(N*k);
    for (size_t j=0; j < k; j++) {
        for (size_t i=0; i < N; i++) {
            mat[j*N + i] = cos(freqstep * (i+0.5) * j);
        }
    }
    return std::move(mat);
}

// for dct1, dct2
template<typename T>
std::vector<T> dct2mat(size_t N, size_t k) {
    T freqstep = M_PI / N;
    return dctmat_impl<T>(N, k, freqstep);
}

template<typename T>
DCTII<T>::DCTII(size_t N, size_t k)
    : m_input()
    , m_output()
    , m_cosmat(dct2mat<T>(N,k))
{
    m_input.resize(N);
    m_output.resize(k);

}

template<typename T>
DCTII<T>::~DCTII()
{

}

template<typename T>
size_t DCTII<T>::size() const {
    return m_output.size();
}

template<typename T>
T* DCTII<T>::inputBegin() {
    return &m_input[0];
}

template<typename T>
T* DCTII<T>::inputEnd() {
    return &m_input[0] + m_input.size();
}

template<typename T>
T* DCTII<T>::outputBegin() {
    return &m_output[0];
}

template<typename T>
T* DCTII<T>::outputEnd() {
    return &m_output[0] + m_output.size();
}

template<typename T>
void DCTII<T>::execute()
{
    size_t N = m_input.size();
    size_t k = m_output.size();
    for (size_t j=0; j < k; j++) {
        m_output[j] = std::inner_product(m_input.begin(), m_input.end(),
            &m_cosmat[j*N], 0.0);
    }
}

template<typename T>
bool DCTII<T>::supports(sigproc::bell::TransformKind kind) {
    return false;
}

template class DCTII<float>;
template class DCTII<double>;

       } // algorithm
    } // bell
} // sigproc

