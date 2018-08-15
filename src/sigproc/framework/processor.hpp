

#ifndef SIGPROC_FRAMEWORK_PROCESSOR_HPP
#define SIGPROC_FRAMEWORK_PROCESSOR_HPP

#include <vector>
#include <map>
#include <memory>

#include "sigproc/framework/node.hpp"
#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"

namespace sigproc {
    namespace framework {

class StreamBase;
class PortBase;

class Processor : public Node
{

    std::vector<StreamBase*> m_output_streams;
    std::vector<PortBase*> m_input_ports;

public:
    Processor()
      : Node("Processor")
    {}
    Processor(const std::string& name)
      : Node(name)
    {}
    virtual ~Processor() {}

    virtual size_t calculate_depth();

protected:
    // to be overridden in a base class
    virtual PortBase* create_port(const std::string& name) { return nullptr; }
    virtual StreamBase* create_stream(const std::string& name) { return nullptr; }

public:

    virtual PortBase* getPort(const std::string& name) final;
    virtual StreamBase* getStream(const std::string& name) final;

    virtual void set_parameter(const std::string& param, const Composite& value) {};

    virtual std::map<std::string, Composite> get_parameters() const {
        std::map<std::string, Composite> params;
        return params;
    }

private:

    virtual void attachPort(PortBase* port) final;
    virtual void attachStream(StreamBase* stream) final;

};


    } // framework
} // sigproc

#endif