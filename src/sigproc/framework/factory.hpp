
#ifndef SIGPROC_FRAMEWORK_FACTORY_HPP
#define SIGPROC_FRAMEWORK_FACTORY_HPP

#include <map>
#include <functional>

#include "sigproc/framework/processor.hpp"

namespace sigproc {
    namespace framework {

class ProcessorFactory
{
    std::map<std::string, std::function<Processor*()>>* m_pRegistry;

public:
    ProcessorFactory(std::map<std::string, std::function<Processor*()>>* registry)
        : m_pRegistry(registry)
    {}
    ~ProcessorFactory() {}

    Processor* create(const std::string& procName);

};

    } // framework
} // sigproc

#endif