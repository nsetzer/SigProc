
#include "sigproc/framework/registry.hpp"

namespace sigproc {
    namespace framework {


std::map<std::string, std::function<Processor*()>>* ProcessorRegistry::processor_registry = nullptr;

void ProcessorRegistry::reg(const std::string& name, std::function<Processor*()> fptr) {
    if (ProcessorRegistry::processor_registry == nullptr) {
        ProcessorRegistry::processor_registry = new std::map<std::string, std::function<Processor*()>>();
    }
    (*processor_registry)[name] = fptr;
}

void ProcessorRegistry::reg(const std::string& name, const std::string& element, std::function<Processor*()> fptr) {
    if (ProcessorRegistry::processor_registry == nullptr) {
        ProcessorRegistry::processor_registry = new std::map<std::string, std::function<Processor*()>>();
    }
    std::string type_name = name + "<" + element + ">";
    (*processor_registry)[type_name] = fptr;
}

void ProcessorRegistry::reg(const std::string& name, const std::string& element, const std::string& stream, std::function<Processor*()> fptr) {
    if (ProcessorRegistry::processor_registry == nullptr) {
        ProcessorRegistry::processor_registry = new std::map<std::string, std::function<Processor*()>>();
    }
    std::string stream_name(stream);
    size_t index = stream_name.find("Stream");
    if (index != std::string::npos) {
        stream_name = stream_name.substr(0, index);
    }
    std::string type_name = name + "<" + element + "," + stream_name + ">";
    (*processor_registry)[type_name] = fptr;
}

    }
}