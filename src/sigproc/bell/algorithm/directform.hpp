

#ifndef SIGPROC_BELL_ALGORITHM_DIRECTFORM_HPP
#define SIGPROC_BELL_ALGORITHM_DIRECTFORM_HPP

#include <vector>
#include <deque>
#include <cmath>
#include <iostream>

namespace sigproc {
    namespace bell {
        namespace algorithm {

template<typename T>
class DirectForm
{
public:
    DirectForm() {}
    ~DirectForm() {}

    virtual void filt(T* x, size_t len) = 0;

};

// https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html

template<typename T>
class DirectFormII : public DirectForm<T>
{
    std::vector<T> B;
    std::vector<T> A;
    std::vector<T> V;
    ssize_t head=0;

public:
    DirectFormII(std::vector<T>& B_, std::vector<T>& A_)
        : B(B_), A(A_)
    {
        V.resize(B.size(), 0);
    }
    ~DirectFormII() {}

    virtual void filt(T* x, size_t len) {

        const size_t s = V.size();
        for (size_t i=0; i < len; i++) {
            T v = x[i];
            T y = 0.0;
            for (ssize_t j = 1; j < s; j++) {
                // V is a queue, where increasing the index (in a circular
                // fashion) goes backwards in time
                const ssize_t idx = (head + j) % s;
                v -= A[j] * V[idx];
                y += B[j] * V[idx];
            }
            V[head] = v;
            x[i] = B[0]*v + y;

            head -= 1;
            if (head < 0) {
                head += s;
            }
        }
    }

};
        } // algorithm
    } // bell
} // sigproc

#endif