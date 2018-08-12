
#ifndef SIGPROC_NODES_ADDER_HPP
#define SIGPROC_NODES_ADDER_HPP


namespace sigproc {
    namespace framework {

using namespace sigproc::framework;

template<typename ELEMENT_TYPE, typename STREAM_TYPE>
class Adder : public Processor
{


public:
    Adder()
      : Processor() {}
    ~Adder() {}

    STREAM_TYPE m_output;
    Port<ELEMENT_TYPE> m_input0;
    Port<ELEMENT_TYPE> m_input1;

    virtual void open() {}

    virtual bool compute() {

        size_t count = 0;

        while (count < m_input0.stream()->size() &&
               count < m_input1.stream()->size()) {
            ELEMENT_TYPE a = m_input0.stream()->at(count);
            ELEMENT_TYPE b = m_input1.stream()->at(count);

            m_output.push_back(0, 0, a + b);

            count++;
        }

        m_input0.advance_index(count);
        m_input1.advance_index(count);

        return count > 0;
    }

    virtual void close() {

    }

/*
    virtual void attach_port(const std::string& name, PortBase* port) {
        if (name == "INPUT0") {
            m_input0 = port->cast<ELEMENT_TYPE>(getElementType<ELEMENT_TYPE>());
        } else if (name == "INPUT1") {
            m_input1 = port->cast<ELEMENT_TYPE>(getElementType<ELEMENT_TYPE>());
        } else {
            throw std::runtime_error("");
        }
    }

    virtual void attach_stream(const std::string& name, StreamBase* stream) {
        if (name == "OUTPUT") {
            m_output = stream->cast<ELEMENT_TYPE>(getElementType<ELEMENT_TYPE>());
        } else {
            throw std::runtime_error("");
        }
    }
*/

    virtual PortBase* create_port(const std::string& name) {
        if (name == "INPUT0") {
            return &m_input0;
        } else if (name == "INPUT1") {
            return &m_input1;
        } else {
            throw std::runtime_error("");
        }
    }
    virtual StreamBase* create_stream(const std::string& name) {

        if (name != "OUTPUT") {
            throw std::runtime_error("");
        }

        return &m_output;
    }


};


    } // nodes
} // sigproc

#endif

