

#ifndef SIGPROC_FRAMEWORK_GRAPH_HPP
#define SIGPROC_FRAMEWORK_GRAPH_HPP

#include <vector>
namespace sigproc {
    namespace framework {

class StreamBase;
class PortBase;
class Processor;
class Node;

class Graph
{
    std::vector<Node*> m_nodes;
    std::vector<Node*> m_owned;

public:
    Graph();
    virtual ~Graph();


    /*
    connect a stream to a port, checking that the element types match
    */
    static void link(StreamBase* stream, PortBase* port);

    // add a node, but do not take ownership of the pointer
    virtual void addNode(Node* node);

    // add a node and take ownership of the pointer
    virtual void takeNode(Node* node);

    virtual void open();
    virtual bool compute();
    virtual void close();

};



    } // framework
} // sigproc

#endif