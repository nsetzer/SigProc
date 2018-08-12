
#include "sigproc/framework/node.hpp"
#include "sigproc/framework/stream.hpp"
#include "sigproc/framework/port.hpp"
#include "sigproc/framework/processor.hpp"


namespace sigproc {
    namespace framework {

size_t Processor::calculate_depth() {

    if (m_depth == 0) {
        for (auto& ptr : m_output_streams) {
            m_depth = std::max(m_depth, ptr->calculate_depth());
        }
        m_depth = m_depth + 1;
    }

    return m_depth;
}

PortBase* Processor::getPort(const std::string& name) {
    PortBase* port = create_port(name);
    if (port != nullptr) {
        // todo: check if allready attached
        this->attachPort(port);
        port->attachProcessor(this);
        port->set_name(name);
    }
    return port;
}

StreamBase* Processor::getStream(const std::string& name) {
    StreamBase* stream = create_stream(name);
    if (stream != nullptr) {
        // todo: check if allready attached
        this->attachStream(stream);
        stream->attachProcessor(this);
        stream->set_name(name);
    }
    return stream;
}

void Processor::attachPort(PortBase* port) {
    m_input_ports.push_back(port);
    //attach_port(name, port);
    //port->attachProcessor(this);
}

void Processor::attachStream(StreamBase* stream) {
    m_output_streams.push_back(stream);
    //attach_stream(name, stream);
}


    } // framework
} // sigproc
