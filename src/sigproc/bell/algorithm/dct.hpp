

#ifndef SIGPROC_BELL_ALGORITHM_DCT_HPP
#define SIGPROC_BELL_ALGORITHM_DCT_HPP

#include <vector>
#include "sigproc/bell/base.hpp"

namespace sigproc {
    namespace bell {
        namespace algorithm {


template<typename T>
class DCTII : public sigproc::bell::TransformBase<T>
{
    std::vector<T> m_input;
    std::vector<T> m_output;
    std::vector<T> m_cosmat;

public:
    DCTII(size_t N, size_t k);
    virtual ~DCTII();

    DCTII(const DCTII& ) = delete; // copy constructor
    DCTII(const DCTII&& ) = delete; // move constructor
    DCTII &operator=( const DCTII& ) = delete; // copy assignment operator
    DCTII &operator=( const DCTII&&  ) = delete; // move assignment operator

    virtual size_t size() const;

    virtual T* inputBegin();
    virtual T* inputEnd();
    virtual T* outputBegin();
    virtual T* outputEnd();

    void execute();

    static bool supports(sigproc::bell::TransformKind kind);

};
        } // algorithm
    } // bell
} // sigproc

#endif