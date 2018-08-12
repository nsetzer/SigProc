

#include <iostream>

/*

todo: figure out the depth calulation problem
      encoder / decoder processors
      root / terminal node singletons for attaching encoders, decoders
*/
#include "sigproc/framework/node.hpp"
#include "sigproc/framework/stream.hpp"
#include "sigproc/framework/port.hpp"
#include "sigproc/framework/processor.hpp"
#include "sigproc/framework/registry.hpp"
#include "sigproc/framework/factory.hpp"
#include "sigproc/framework/graph.hpp"
#include "sigproc/nodes/adder.hpp"

using namespace sigproc::framework;

int main() {

    std::cout << "registered nodes" << std::endl;

    for (const auto& kv : ProcessorRegistry::processor_registry) {
        std::cout << kv.first << std::endl;
    }
    std::cout << "----------------" << std::endl;

    ProcessorFactory factory(&ProcessorRegistry::processor_registry);

    Stream<int32_t>* stream0 = new IrregularStream<int32_t>();
    Stream<int32_t>* stream1 = new IrregularStream<int32_t>();
    Processor* proc = factory.create("Adder<INT32,Irregular>");

    StreamBase* stream2 = proc->getStream("OUTPUT");
    PortBase* port0 = proc->getPort("INPUT0");
    PortBase* port1 = proc->getPort("INPUT1");

    Graph::link(stream0, port0);
    Graph::link(stream1, port1);

    stream0->push_back(0, 1, 1);
    stream0->push_back(1, 2, 2);
    stream0->push_back(2, 3, 3);

    stream1->push_back(0, 1, 10);
    stream1->push_back(1, 2, 20);
    stream1->push_back(2, 3, 30);

    std::vector<Node*> nodes;
    nodes.push_back(stream0);
    nodes.push_back(stream1);
    nodes.push_back(stream2);
    nodes.push_back(port0);
    nodes.push_back(port1);
    nodes.push_back(proc);

    // sort:: <: smallest to largest
    //        >: largest to smallest
    // which is optimal?
    // TODO: depth is calculated by total graph traversal only every call

    // sort must be done with non-const pointers, to calculate depths
    std::sort(nodes.begin(), nodes.end(),
        [](Node* a, Node* b) -> bool {
            return a->calculate_depth() < b->calculate_depth();
    });

    for (Node* node  : nodes) {
        std::cout << node->name() << " " << node->depth() << std::endl;
        node->compute();
    }

    std::cout << "steam 0: " << stream0->depth() << std::endl;
    std::cout << "steam 1: " << stream1->depth() << std::endl;
    std::cout << "port  0: " << port0->depth() << std::endl;
    std::cout << "port  1: " << port1->depth() << std::endl;
    std::cout << "proc  0: " << proc->depth() << std::endl;
    std::cout << "steam 2: " << stream2->depth() << std::endl;

    std::cout << "steam 0 size: " << stream0->size() << std::endl;
    std::cout << "steam 1 size: " << stream1->size() << std::endl;
    std::cout << "steam 2 size: " << stream2->size() << std::endl;

    return 0;
}