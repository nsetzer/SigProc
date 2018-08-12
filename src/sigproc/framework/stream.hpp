

#ifndef SIGPROC_FRAMEWORK_STREAM_HPP
#define SIGPROC_FRAMEWORK_STREAM_HPP

#include <iostream>

#include <deque>
#include <vector>
#include <memory>
#include <algorithm>
#include <exception>

#include "sigproc/framework/node.hpp"
#include "sigproc/framework/port.hpp"
#include "sigproc/framework/enum.hpp"

namespace sigproc {
    namespace framework {

template<typename ELEMENT_TYPE> class Stream;

class StreamBase : public Node
{
    Processor* m_processor;

public:
    StreamBase() {}
    virtual ~StreamBase() {}

    virtual ElementType elementType() = 0;
    virtual IntervalType intervalType() = 0;

    template<typename ELEMENT_TYPE>
    Stream<ELEMENT_TYPE>* cast(ElementType  type) {

        if (type != elementType()  && type == getElementType<ELEMENT_TYPE>()) {
            throw std::runtime_error("");
        }

        return static_cast<Stream<ELEMENT_TYPE>*>(this);

    }

    void attachProcessor(Processor* processor) {
        m_processor = processor;
    }

    // capacity
    virtual size_t size() = 0;

};

template<typename ELEMENT_TYPE>
class Stream : public StreamBase
{
public:
    Stream() {}
    virtual ~Stream() {}

    // access
    virtual ELEMENT_TYPE& at(size_t index) = 0;

    // modifiers
    virtual void push_back(size_t ts, size_t te, ELEMENT_TYPE value) = 0;

    // interval specification
    virtual size_t units() = 0;
    virtual size_t offset() = 0;
    virtual size_t period(size_t index) = 0;
    virtual size_t duration(size_t index) = 0;
    virtual size_t units_start(size_t index) = 0;
    virtual size_t units_end(size_t index) = 0;

    virtual void attachPort(Port<ELEMENT_TYPE>* port) = 0;

    virtual ElementType elementType() { return getElementType<ELEMENT_TYPE>(); }
    virtual IntervalType intervalType() { return IntervalType::UNKNOWN; }
};

template<typename ELEMENT_TYPE>
class StreamImpl
{
    size_t m_depth;
    size_t m_index;
private:

    std::deque<ELEMENT_TYPE> m_stream;
    std::vector<PortBase*> m_ports;
    std::shared_ptr<Node> m_parent;

    typedef typename std::vector<ELEMENT_TYPE>::iterator VecIterator;
    typedef typename std::deque<ELEMENT_TYPE>::iterator DeqIterator;

public:
    StreamImpl()
        : m_depth(0)
        , m_index(0)
    {};
    virtual ~StreamImpl() {};

    void attachPort(PortBase* port) {
        m_ports.push_back(port);
    }

    size_t index() {
        return m_index;
    }

    size_t size() {
        return m_stream.size();
    }

    ELEMENT_TYPE& at(size_t index) {
        size_t idx = index - m_index;
        // TODO validate idx in range...
        return m_stream.at(idx);
    }

    void push_back(ELEMENT_TYPE v) {
        m_stream.push_back(v);
    }

    void push_back(const VecIterator begin, const VecIterator end) {
        while(begin != end) {
            m_stream.push_back(*begin);
            ++begin;
        }
    }

    void push_back(const DeqIterator begin, const DeqIterator end) {
        while(begin != end) {
            m_stream.push_back(*begin);
            ++begin;
        }
    }

    size_t slice(size_t index, VecIterator outputIter, VecIterator outputEnd) {

        // TODO: assert index >= m_index

        size_t N = index - m_index;

        DeqIterator iter = m_stream.begin();

        // TODO: fix that N is not the end
        DeqIterator end = m_stream.begin() + N;

        size_t count = 0;
        while (iter != end && outputIter != outputEnd) {

            *outputIter++ = *iter++;
            count++;
        }

        return count;
    }

    virtual void open() {

    }

    virtual bool compute() {
        _free();
        return false;
    }

    virtual void close() {

    }

    size_t calculate_depth() {
        // first time called: calculate the depth, min depth is 1
        if (m_depth == 0) {
            for (auto& ptr : m_ports) {
                m_depth = std::max(m_depth, ptr->calculate_depth());
            }
            m_depth = m_depth + 1;
        }
        return m_depth;
    }

private:
    void _free() {

        // determine the number of elements to free from the deque

        // TODO: if no ports are attached, clear the Stream

        if (m_stream.size()==0) {
            return;
        }

        if (m_ports.size() == 0) {
            std::cout << "no ports attached" << std::endl;
            return;
        }

        size_t minimum = m_ports[0]->index();
        for (const auto& ptr : m_ports) {
            minimum = std::min(minimum, ptr->index());
        }

        if (minimum <= m_index) {
            std::cout << "something is strange " << minimum << "/" << m_index << std::endl;
            return;
        }

        size_t N = minimum - m_index;

        /*
        std::cout << "size: " << m_stream.size()
                   << " " << m_index
                   << " " << minimum
                   << " " << N
                   << std::endl;
        */

        // TODO: assert N is less than or equal to the number of elements

        m_stream.erase(m_stream.begin(), m_stream.begin() + N);
        m_index += N;
    }

};

template<typename ELEMENT_TYPE>
class IrregularElement {

public:
    size_t ts;
    size_t te;
    ELEMENT_TYPE value;

    IrregularElement(size_t ts, size_t te, ELEMENT_TYPE value)
        : ts(ts)
        , te(te)
        , value(value)
    {}
    virtual ~IrregularElement() {}
} ;

template<typename ELEMENT_TYPE>
class IrregularStream : public Stream<ELEMENT_TYPE>
{
    StreamImpl<IrregularElement<ELEMENT_TYPE>> m_impl;
    size_t m_units = 0;

public:
    IrregularStream()
        : m_impl()
    {}
    virtual ~IrregularStream() {}

    void attachPort(Port<ELEMENT_TYPE>* port) {
        m_impl.attachPort(port);
    }

    ELEMENT_TYPE& at(size_t index) {
        return m_impl.at(index).value;
    }

    virtual void push_back(size_t ts, size_t te, ELEMENT_TYPE value) {
        m_impl.push_back({ts, te, value});
    }

    virtual size_t size() {
        return m_impl.size();
    }

    virtual void open() {
        m_impl.open();
    }

    virtual bool compute() {
        return m_impl.compute();
    }

    virtual void close() {
        m_impl.close();

    }

    virtual size_t units() {
        return m_units;
    }

    virtual size_t offset() {
        return 0;
    }

    virtual size_t period(size_t index) {
        return 0;
    }

    virtual size_t duration(size_t index) {
        IrregularElement<ELEMENT_TYPE>& element = m_impl.at(index);
        return element.te - element.ts;
    }

    virtual size_t units_start(size_t index) {
        IrregularElement<ELEMENT_TYPE>& element = m_impl.at(index);
        return element.ts;
    }
    virtual size_t units_end(size_t index) {
        IrregularElement<ELEMENT_TYPE>& element = m_impl.at(index);
        return element.te;
    }

    virtual size_t calculate_depth() final {
        Node::m_depth = m_impl.calculate_depth();
        return Node::m_depth;
    }
};

template<typename ELEMENT_TYPE>
class RegularStream : public Stream<ELEMENT_TYPE>
{
    StreamImpl<ELEMENT_TYPE> m_impl;

    size_t m_units = 0;
    size_t m_offset = 0;
    size_t m_period = 0;
    size_t m_duration = 0;

public:
    RegularStream()
        : m_impl()
    {}
    virtual ~RegularStream() {}

    void attachPort(Port<ELEMENT_TYPE>* port) {
        m_impl.attachPort(port);
    }

    ELEMENT_TYPE& at(size_t index) {
        return m_impl.at(index);
    }

    virtual void push_back(size_t ts, size_t te, ELEMENT_TYPE value) {
        m_impl.push_back(value);
    }

    virtual size_t size() {
        return m_impl.size();
    }

    virtual void open() {
        m_impl.open();
    }

    virtual bool compute() {
        return m_impl.compute();
    }

    virtual void close() {
        m_impl.close();

    }

    virtual size_t units() {
        return m_units;
    }

    virtual size_t offset() {
        return m_offset;
    }

    virtual size_t period(size_t index) {
        return m_period;
    }

    virtual size_t duration(size_t index) {
        return m_duration;
    }

    virtual size_t units_start(size_t index) {
        return m_offset + (index * m_period);
    }

    virtual size_t units_end(size_t index) {
        return m_offset + (index * m_period) + m_duration;
    }

    virtual size_t calculate_depth() final {
        Node::m_depth = m_impl.calculate_depth();
        return Node::m_depth;
    }
};



    } // framework
} // sigproc

#endif

