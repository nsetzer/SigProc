
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

    virtual void open() {

        // todo: stream synchronization?
        // todo: how to handle irregular stream inputs?
        //Stream<ELEMENT_TYPE>* stream0 = m_input0.stream();
        //Stream<ELEMENT_TYPE>* stream1 = m_input1.stream();

        //m_output.set_interval(stream0->offset(),
        //                      stream0->period(),
        //                      stream0->duration());
    }

    virtual bool compute() {

        size_t index = m_input0.index();
        Stream<ELEMENT_TYPE>* stream0 = m_input0.stream();
        Stream<ELEMENT_TYPE>* stream1 = m_input1.stream();

        size_t n_elements = std::min(stream0->size(), stream1->size());

        // todo: create an element iterator type for streams
        for (size_t i=0; i < n_elements; i++) {
            ELEMENT_TYPE a = stream0->at(index+i);
            ELEMENT_TYPE b = stream1->at(index+i);
            ELEMENT_TYPE result=0;
            switch (m_operation) {
                case binaryoperator::Operation::ADD:
                    result = a + b;
                    break;
                case binaryoperator::Operation::SUBTRACT:
                    result = a + b;
                    break;
                case binaryoperator::Operation::MULTIPLY:
                    result = a * b;
                    break;
                case binaryoperator::Operation::DIVIDE:
                    result = a / b;
                    break;
                default:
                    // open or set param should fail before we get here
                    result = 0;
                    break;

            }
            size_t ts = stream0->units_start(index+i);
            size_t te = stream0->units_end(index+i);

            m_output.push_back(ts, te, result);
        }

        m_input0.advance_index(n_elements);
        m_input1.advance_index(n_elements);

        return n_elements > 0;
    }

    virtual void close() {

        // todo: check to see if one port has any data remaining,
        //       it may indicate an error in computation
    }

    virtual PortBase* create_port(const std::string& name) {
        if (name == "INPUT0") {
            return &m_input0;
        } else if (name == "INPUT1") {
            return &m_input1;
        } else {
            // todo: alternative is to return null?
            SIGPROC_THROW("No Port named " << name);
        }
    }
    virtual StreamBase* create_stream(const std::string& name) {

        if (name != "OUTPUT") {
            // todo: alternative is to return null?
            SIGPROC_THROW("No Stream named " << name);
        }

        return &m_output;
    }

    virtual void set_parameter(const std::string& param, const Composite& value) {
        if (param == "operator") {
            // TODO: add a asString option to composites
            m_operation = binaryoperator::getOperationFromString(value.as_string());

            if (m_operation == binaryoperator::Operation::UNKNOWN) {
                SIGPROC_THROW("Invalid Operation " << value.as_string());
            }
        }

        std::cout << binaryoperator::getOperationTypeName(m_operation) << std::endl;
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

