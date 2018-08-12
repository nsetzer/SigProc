

#ifndef SIGPROC_FRAMEWORK_NODE_HPP
#define SIGPROC_FRAMEWORK_NODE_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"
#include "sigproc/common/exception.hpp"
namespace sigproc {
    namespace framework {

class Node
{
    std::string m_node_name;
protected:
    size_t m_depth=0;
public:
    Node()
      : m_node_name("Node")
    {};
    Node(const std::string& name)
      : m_node_name(name)
    {};
    virtual ~Node() {};

    const std::string& name() const { return m_node_name; }
    void set_name(const std::string& name) { m_node_name = name; }

    virtual size_t calculate_depth() = 0;
    size_t depth() const { return m_depth; }

    virtual void open() = 0;
    virtual bool compute() = 0;
    virtual void close() = 0;

};

    } // framework
} // sigproc

#endif