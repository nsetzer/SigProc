

#include "sigproc/common/unittest/unittest.hpp"

#include "sigproc/framework/node.hpp"
#include "sigproc/framework/stream.hpp"
#include "sigproc/framework/port.hpp"
#include "sigproc/framework/processor.hpp"
#include "sigproc/framework/registry.hpp"
#include "sigproc/framework/factory.hpp"
#include "sigproc/framework/graph.hpp"
#include "sigproc/nodes/adder.hpp"

using namespace sigproc::framework;
/*
template<typename T>
Processor* create_graph(Stream<T>* stream0, Stream<T>* stream1)
{
    ProcessorFactory factory(&ProcessorRegistry::processor_registry);

    Processor* proc = factory.create("Adder<INT32,Irregular>");

    StreamBase* stream2 = proc->getStream("OUTPUT");
    PortBase* port0 = proc->getPort("INPUT0");
    PortBase* port1 = proc->getPort("INPUT1");

    Graph::link(stream0, port0);
    Graph::link(stream1, port1);

    return proc;
}
*/
SIGPROC_TEST(Adder_Irregular_Int32) {

    Graph graph;

    Stream<int32_t>* stream0 = new IrregularStream<int32_t>();
    Stream<int32_t>* stream1 = new IrregularStream<int32_t>();

    ProcessorFactory factory(&ProcessorRegistry::processor_registry);

    Processor* proc = factory.create("Adder<INT32,Irregular>");

    StreamBase* stream2 = proc->getStream("OUTPUT");
    PortBase* port0 = proc->getPort("INPUT0");
    PortBase* port1 = proc->getPort("INPUT1");

    graph.takeNode(proc);
    graph.takeNode(stream0);
    graph.takeNode(stream1);

    graph.addNode(stream2);
    graph.addNode(port0);
    graph.addNode(port1);

    Graph::link(stream0, port0);
    Graph::link(stream1, port1);

    stream0->push_back(0, 1, 1);
    stream0->push_back(1, 2, 2);
    stream0->push_back(2, 3, 3);

    stream1->push_back(0, 1, 10);
    stream1->push_back(1, 2, 20);
    stream1->push_back(2, 3, 30);

    graph.open();
    graph.compute();
    graph.close();

    std::cout << "steam 0 size: " << stream0->size() << std::endl;
    std::cout << "steam 1 size: " << stream1->size() << std::endl;
    std::cout << "steam 2 size: " << stream2->size() << std::endl;
}