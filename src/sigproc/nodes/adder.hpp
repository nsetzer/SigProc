
#ifndef SIGPROC_NODES_ADDER_HPP
#define SIGPROC_NODES_ADDER_HPP


namespace sigproc {
    namespace framework {

using namespace sigproc::framework;

namespace binaryoperator {

    enum class Operation: char {
        UNKNOWN = 0,
        ADD = 1,
        SUBTRACT = 2,
        MULTIPLY = 3,
        DIVIDE = 4,
    };

    std::string getOperationTypeName(Operation type);
    Operation getOperationFromString(const std::string& str);
}

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

    binaryoperator::Operation m_operation;

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

    virtual void set_parameter(const std::string& param, const Composite& value) {
        if (param == "operator") {
            // TODO: add a asString option to composites
            m_operation = binaryoperator::Operation::ADD;
        }
    };


    virtual ParamDef parameters() const {
        ParamDef def;

        def["operator"] = "ADD";

        return def;
    }

    virtual PortDef ports() const {
        PortDef def;

        def["INPUT0"] = InputConfigType::PORT;
        def["INPUT1"] = InputConfigType::PORT;

        return def;
    }

    virtual StreamDef streams() const {
        StreamDef def;

        def["OUTPUT"] = OutputConfigType::STREAM;

        return def;
    }



};


    } // nodes
} // sigproc

#endif

