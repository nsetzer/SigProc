

#include "sigproc/framework/graph.hpp"
#include "sigproc/framework/node.hpp"
#include "sigproc/framework/stream.hpp"
#include "sigproc/framework/port.hpp"

namespace sigproc {
    namespace framework {

namespace {
    template<typename T>
    void link_impl(StreamBase* s, PortBase* p)
    {
        ElementType type = getElementType<T>();
        Port<T>* port = p->cast<T>(type);
        Stream<T>* stream = s->cast<T>(type);
        port->attachStream(stream);
        stream->attachPort(port);
    }
} // anonymous

void Graph::link(StreamBase* stream, PortBase* port)
{
    if (port->elementType() != stream->elementType()) {
        throw std::runtime_error("...");
    }

    switch(port->elementType()) {
        case ElementType::INT8:
            link_impl<int8_t>(stream, port);
            break;
        case ElementType::INT16:
            link_impl<int16_t>(stream, port);
            break;
        case ElementType::INT32:
            link_impl<int32_t>(stream, port);
            break;
        case ElementType::INT64:
            link_impl<int64_t>(stream, port);
            break;
        case ElementType::UINT8:
            link_impl<uint8_t>(stream, port);
            break;
        case ElementType::UINT16:
            link_impl<uint16_t>(stream, port);
            break;
        case ElementType::UINT32:
            link_impl<uint32_t>(stream, port);
            break;
        case ElementType::UINT64:
            link_impl<uint64_t>(stream, port);
            break;
        case ElementType::FLOAT32:
            link_impl<float>(stream, port);
            break;
        case ElementType::FLOAT64:
            link_impl<double>(stream, port);
            break;
        case ElementType::STRING:
            link_impl<std::string>(stream, port);
            break;
        case ElementType::COMPOSITE:
            link_impl<Composite>(stream, port);
            break;
        case ElementType::FLOAT32VEC:
            link_impl<std::vector<float>>(stream, port);
            break;
        case ElementType::FLOAT64VEC:
            link_impl<std::vector<double>>(stream, port);
            break;
        default:
            // names of nodes!
            throw std::runtime_error("unable to link...");
            break;
    }
}

Graph::Graph()
    : m_nodes()
    , m_owned()
{

}

Graph::~Graph()
{
    while (m_owned.size() > 0) {
        Node* node = m_owned.back();
        delete node;
        m_owned.pop_back();
    }
}

void Graph::addNode(Node* node)
{
    m_nodes.push_back(node);
}

void Graph::takeNode(Node* node) {
    m_owned.push_back(node);
    addNode(node);
}

void Graph::open()
{
    std::sort(m_nodes.begin(), m_nodes.end(),
        [](Node* a, Node* b) -> bool {
            return a->calculate_depth() < b->calculate_depth();
    });

    for (Node* node : m_nodes) {
        node->open();
    }
}

bool Graph::compute()
{
    for (Node* node : m_nodes) {
        node->compute();
    }

    return true;
}

void Graph::close()
{
    for (Node* node : m_nodes) {
        node->close();
    }
}


    } // framework
} // sigproc

