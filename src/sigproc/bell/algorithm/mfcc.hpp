

#ifndef SIGPROC_BELL_ALGORITHM_MFCC_HPP
#define SIGPROC_BELL_ALGORITHM_MFCC_HPP

#include <vector>
#include <cmath>
#include <iostream>
#include <cstddef>
#include <memory>
#include <numeric>

#include "sigproc/common/exception.hpp"
#include "sigproc/common/format.hpp"
#include "sigproc/bell/base.hpp"
#include "sigproc/bell/algorithm/window.hpp"

using namespace sigproc::common;

namespace sigproc {
    namespace bell {
        namespace algorithm {
            namespace mfcc {

double freq2mel(double freq);
double mel2freq(double mel);

size_t freq2idx(double Fs, size_t N, double freq);
double idx2freq(double Fs, size_t N, size_t idx);

double freq2midi(double freq, int tonesPerOctave, int middleAHz);
double midi2freq(double midi, int tonesPerOctave, int middleAHz);

template <typename T>
class FilterBank
{
    T m_Fs;
    size_t m_nBin;
    size_t m_nFilterBin;

    std::vector<ssize_t> m_table_index;
    std::vector<ssize_t> m_table_length;
    std::vector<T> m_filters;

public:
    FilterBank(T Fs, size_t nBin, size_t nFilterBin, T Fmin, T Fmax, bool equalize)
        : m_Fs(Fs)
        , m_nBin(nBin)
        , m_nFilterBin(nFilterBin)
        , m_table_index()
        , m_table_length()
        , m_filters()
    {
        init_index(Fmin, Fmax);
        init_filters(equalize);
    }
    ~FilterBank()
    {}

    void filter(std::vector<T>& in, std::vector<T>& out) {
        if (in.size() != m_nBin) {
            SIGPROC_THROW("Incorrect Input Size: " << in.size()
                << " expected: " << m_nBin);
        }
        if (out.size() != m_nFilterBin) {
            out.resize(m_nFilterBin);
        }

        const size_t N = m_table_length.back();

        for (size_t i=0; i < m_nFilterBin; i++) {
            size_t k = N*i;
            ssize_t il = m_table_index[i];
            ssize_t ih = m_table_index[i+2];
            out[i] = std::inner_product(in.begin()+il, in.begin()+ih,
                &m_filters[k], 0.0);
        }
    }

private:

    void init_index(T Fmin, T Fmax) {

        T cminf = freq2mel(Fmin);
        T cmaxf = freq2mel(Fmax);
        T delta = cmaxf - cminf;
        size_t N = m_nFilterBin + 1;

        m_table_index.resize(m_nFilterBin+2);
        for (size_t i=0; i<m_nFilterBin+2; i++) {
            T mel = cminf + i*delta/N;
            m_table_index[i] = freq2idx(m_Fs, m_nBin, mel2freq(mel));
        }

        m_table_length.resize(m_nFilterBin);
        for (size_t i=0; i<m_nFilterBin; i++) {

            m_table_length[i] = m_table_index[i+2] - m_table_index[i];
            m_table_length[i] = std::max(static_cast<ssize_t>(1), m_table_length[i]);
        }
    }

    void init_filters(bool equalize) {

        const size_t N = m_table_length.back();
        const size_t n_elements = m_nFilterBin * N;
        m_filters.resize(n_elements);


        for (size_t i=0; i<m_nFilterBin; i++) {

            ssize_t k=N*i;

            // this is a degenerate case when nFilterBin is too large
            // relative to nBin
            if (m_table_length[i]==1) {
                m_filters[k] = 1.0;
                continue;
            }

            ssize_t il = m_table_index[i];
            ssize_t ic = m_table_index[i+1];
            ssize_t ih = m_table_index[i+2];

            double tlm = ((-1.0) / static_cast<double>(il - ic));
            double tlb = - tlm * il;
            double thm = ((1.0) / static_cast<double>(ic - ih));
            double thb = - thm * ih;

            // sample a triangle stretched over il to ih with height 1.0
            // triangles are equi-sized in mel space

            T mag = 0.0, tmp;

            // sample a line from (il,0) to (ic, mag)
            for (ssize_t j=il; j<ic; j++) {
                tmp = tlm * j + tlb;
                m_filters[k++] = tmp;
                mag += tmp;
            }

            // sample a line from (ic,mag) to (ih, 0)
            for (ssize_t j=ic; j<ih; j++) {
                tmp = thm * j + thb;
                m_filters[k++] = tmp;
                mag += tmp;
            }

            // when equalizing, the area of the triangle should sum to one
            if (equalize) {
                for (ssize_t j=0; j<m_table_length[i]; j++) {
                    m_filters[N*i+j] /= mag;
                }
            }

            /*
            T t = 0;
            fmt::osprintf(std::cout, "%4d: %4d %4d %4d k:%4d/%4d ", i, il, ic, ih, k-N*i, m_table_length[i]);
            for (ssize_t j=0; j<m_table_length[i]; j++) {
                t += m_filters[N*i+j];
                fmt::osprintf(std::cout, "%8.6f ", m_filters[N*i+j]);
            }
            fmt::osprintf(std::cout, "<%t>\n", t);
            */
            //m_filters.push_back(std::move(vec));

        }
    }

};

template <typename T>
class PreEmphasisFilter
{
    T m_a;
    T m_last;
public:
    PreEmphasisFilter(T a)
        : m_a(a)
        , m_last(0.0)
    {}
    ~PreEmphasisFilter() {}

    T apply(T v) {
        // naive, replace with a matrix implementation
        T result = v - m_a * m_last;
        m_last = v;
        return result;
    }
};
template <typename T>
class FeatureGenerator
{

    PreEmphasisFilter<T> m_preEmph;
    std::vector<T> m_window;
    std::unique_ptr<FilterBank<T>> m_filterBank;
    std::unique_ptr<TransformBase<T>> m_dft;
    std::unique_ptr<TransformBase<T>> m_dct;
public:

    FeatureGenerator()
    {
        init();
    }
    ~FeatureGenerator() {}


    void init() {
        size_t Fs = 16000;
        size_t nWindow = 448;
        size_t nBin = 512;
        size_t nFilterBin = 40;
        T minF = 133.0;
        T maxF = 6855.0;
        bool equalize = true;

        m_preEmph = PreEmphasisFilter<T>(0.97);

        // create a window function with nWindow, which may be
        // less than the FFT size. then zero pad to the full width.
        m_window = window::hamming<T>(nWindow);
        m_window.resize(nBin);

        m_filterBank = std::unique_ptr<FilterBank<T>>(
            new FilterBank<T>(Fs, nBin, nFilterBin, minF, maxF, equalize));

        m_dft = std::unique_ptr<TransformBase<T>>(
            newRealTransform<T>(TransformKind::FORWARD, Fs, nBin));

        m_dct = std::unique_ptr<TransformBase<T>>(
            newRealTransform<T>(TransformKind::DCTII, Fs, nFilterBin));
    }

    // return the number of elements consumed (0 or nWindow)
    // out will be resize to nFilterBin
    size_t filter(std::vector<T>& in, std::vector<T>& out) {
        return 0;
    }


private:
};

            } // mfcc
        } // algorithm
    } // bell
} // sigproc

#endif