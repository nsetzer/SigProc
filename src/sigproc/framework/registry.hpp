#ifndef SIGPROC_FRAMEWORK_REGISTRY_HPP
#define SIGPROC_FRAMEWORK_REGISTRY_HPP

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <cstddef>

#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"
#include "sigproc/framework/stream.hpp"
#include "sigproc/framework/processor.hpp"

namespace sigproc {
    namespace framework {

class ProcessorRegistry
{
public:
    ProcessorRegistry() {}
    ~ProcessorRegistry() {}

    static std::map<std::string, std::function<Processor*()>> processor_registry;

    static void reg(const std::string& name, std::function<Processor*()> fptr);
    static void reg(const std::string& name, const std::string& element, std::function<Processor*()> fptr);
    static void reg(const std::string& name, const std::string& element, const std::string& stream, std::function<Processor*()> fptr);

};

#define REGISTER_BEGIN_IMPL(x) struct x{x(){

#define REGISTER_END_IMPL(regcls) }}; regcls regcls ## _i;

// to register a new type, use these three macros
// or use the convenience methods defined below
#define REGISTER_BEGIN(uid) REGISTER_BEGIN_IMPL(Register_ ## uid)
#define REGISTER_PROC(proc) ProcessorRegistry::reg(#proc, []() -> Processor* {return new proc();});
#define REGISTER_END(uid) REGISTER_END_IMPL(Register_ ## uid)

#define REGISTER_ELEMENT_IMPL(name, e) ProcessorRegistry::reg(#name, getTypeName<e>(), []() -> Processor* {return new name<e>();});
#define REGISTER_STREAM_IMPL(name, e, s) ProcessorRegistry::reg(#name, getTypeName<e>(), #s, []() -> Processor* {return new name<e,s<e>>();});

// register a processor which does not take template arguments
#define REGISTER_PROCESSOR(x) \
    REGISTER_BEGIN(x) \
    REGISTER_PROC(x) \
    REGISTER_END(x)

// register a processor with all default element types
#define REGISTER_NUMERIC_ELEMENT_PROCESSOR(x) \
    REGISTER_BEGIN(x) \
    REGISTER_ELEMENT_IMPL(x, int8_t) \
    REGISTER_ELEMENT_IMPL(x, int16_t) \
    REGISTER_ELEMENT_IMPL(x, int32_t) \
    REGISTER_ELEMENT_IMPL(x, int64_t) \
    REGISTER_ELEMENT_IMPL(x, uint8_t) \
    REGISTER_ELEMENT_IMPL(x, uint16_t) \
    REGISTER_ELEMENT_IMPL(x, uint32_t) \
    REGISTER_ELEMENT_IMPL(x, uint64_t) \
    REGISTER_ELEMENT_IMPL(x, float) \
    REGISTER_ELEMENT_IMPL(x, double) \
    REGISTER_END(x)

#define REGISTER_ELEMENT_PROCESSOR(x) \
    REGISTER_BEGIN(x) \
    REGISTER_ELEMENT_IMPL(x, int8_t) \
    REGISTER_ELEMENT_IMPL(x, int16_t) \
    REGISTER_ELEMENT_IMPL(x, int32_t) \
    REGISTER_ELEMENT_IMPL(x, int64_t) \
    REGISTER_ELEMENT_IMPL(x, uint8_t) \
    REGISTER_ELEMENT_IMPL(x, uint16_t) \
    REGISTER_ELEMENT_IMPL(x, uint32_t) \
    REGISTER_ELEMENT_IMPL(x, uint64_t) \
    REGISTER_ELEMENT_IMPL(x, float) \
    REGISTER_ELEMENT_IMPL(x, double) \
    REGISTER_ELEMENT_IMPL(x, std::string) \
    REGISTER_ELEMENT_IMPL(x, Composite) \
    REGISTER_ELEMENT_IMPL(x, std::vector<float>) \
    REGISTER_ELEMENT_IMPL(x, std::vector<double>) \
    REGISTER_END(x)

// register a processor with all default element and stream types
#define REGISTER_NUMERIC_STREAM_PROCESSOR(x) \
    REGISTER_BEGIN(x) \
    REGISTER_STREAM_IMPL(x, int8_t,   IrregularStream) \
    REGISTER_STREAM_IMPL(x, int16_t,  IrregularStream) \
    REGISTER_STREAM_IMPL(x, int32_t,  IrregularStream) \
    REGISTER_STREAM_IMPL(x, int64_t,  IrregularStream) \
    REGISTER_STREAM_IMPL(x, uint8_t,  IrregularStream) \
    REGISTER_STREAM_IMPL(x, uint16_t, IrregularStream) \
    REGISTER_STREAM_IMPL(x, uint32_t, IrregularStream) \
    REGISTER_STREAM_IMPL(x, uint64_t, IrregularStream) \
    REGISTER_STREAM_IMPL(x, float,    IrregularStream) \
    REGISTER_STREAM_IMPL(x, double,   IrregularStream) \
    REGISTER_STREAM_IMPL(x, int8_t,   RegularStream) \
    REGISTER_STREAM_IMPL(x, int16_t,  RegularStream) \
    REGISTER_STREAM_IMPL(x, int32_t,  RegularStream) \
    REGISTER_STREAM_IMPL(x, int64_t,  RegularStream) \
    REGISTER_STREAM_IMPL(x, uint8_t,  RegularStream) \
    REGISTER_STREAM_IMPL(x, uint16_t, RegularStream) \
    REGISTER_STREAM_IMPL(x, uint32_t, RegularStream) \
    REGISTER_STREAM_IMPL(x, uint64_t, RegularStream) \
    REGISTER_STREAM_IMPL(x, float,    RegularStream) \
    REGISTER_STREAM_IMPL(x, double,   RegularStream) \
    REGISTER_END(x)

    } // framework
} // sigproc

#endif