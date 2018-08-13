


#ifndef SIGPROC_FRAMEWORK_GRAPHDEF_HPP
#define SIGPROC_FRAMEWORK_GRAPHDEF_HPP

#include <vector>
#include <memory>

#include "sigproc/framework/factory.hpp"

namespace sigproc {
    namespace framework {

/*
accept a list of node-configs
validate that each object matches the schema
create each node, giving it a name
set the base sample rate for all streams
use the node-config to connect ports / streams
use the node-config to set parameters

schema:

NodeDef:
    type: str
    name: str
    params: map<str, Composite>
    inputs: map<str, PortDef>
    outputs: map<str, StreamDef>

PortDef:
    - str
    - map<str, str>

StreamDef:
    - str
    - map<str, str>

todo:
    given a list of nodes (by name) determine the default
        inputs, outputs, parameters (with values)
        generate a json formatted document
        create an api for querying available nodes
        create a binary for querying available nodes

*/

class Graph;

class GraphDef
{
    std::shared_ptr<ProcessorFactory> m_factory;

public:
    GraphDef(std::shared_ptr<ProcessorFactory>& factory)
        : m_factory(factory)
    {}
    ~GraphDef(){}

    Graph* create(std::vector<std::unique_ptr<Composite>> nodes);

private:
    void _validate();
    void _create();
    void _connect();
    void _set_param();

    // set the factory



};


    } // framework
} // sigproc

#endif