

#ifndef SIGPROC_FRAMEWORK_PORT_HPP
#define SIGPROC_FRAMEWORK_PORT_HPP

#include <memory>

#include "sigproc/framework/node.hpp"

namespace sigproc {
    namespace framework {

template<typename ELEMENT_TYPE>
class Stream;

template<typename ELEMENT_TYPE>
class Port;

class Processor;

class PortBase : public Node
{
    Processor* m_processor;

protected:
    size_t m_index=0;

public:
    PortBase()
      : Node("Port")
      , m_processor(nullptr)
    {}
    PortBase(const std::string& name)
      : Node(name)
      , m_processor(nullptr)
    {}
    ~PortBase() {}

    virtual ElementType elementType() = 0;

    template<typename ELEMENT_TYPE>
    Port<ELEMENT_TYPE>* cast(ElementType  type) {

        if (type != elementType() && type == getElementType<ELEMENT_TYPE>()) {
            throw std::runtime_error("");
        }

        return static_cast<Port<ELEMENT_TYPE>*>(this);

    }

    void attachProcessor(Processor* processor) {
        m_processor = processor;
    }

    virtual size_t calculate_depth();

    size_t index() {
        return m_index;
    }

};

template<typename ELEMENT_TYPE>
class Port : public PortBase
{
    Stream<ELEMENT_TYPE>* m_stream;

public:
    Port()
      : PortBase()
      , m_stream(nullptr)
    {}
    Port(const std::string& name)
      : PortBase(name)
      , m_stream(nullptr)
    {}
    virtual ~Port() {};

    void attachStream(Stream<ELEMENT_TYPE>* stream) {
        m_stream = stream;
    }

    virtual ElementType elementType() { return getElementType<ELEMENT_TYPE>(); }

    Stream<ELEMENT_TYPE>* stream() {
        return &(*m_stream);
    }

    void advance_index(size_t delta) {
        m_index += delta;
    }

    virtual void open() {}
    virtual bool compute() { return false; }
    virtual void close() {}

};

    } // framework
} // sigproc

#endif