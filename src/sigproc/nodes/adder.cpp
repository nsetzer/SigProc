
#include "sigproc/framework/registry.hpp"
#include "sigproc/nodes/adder.hpp"
#include <algorithm>

namespace sigproc {
    namespace framework {

namespace binaryoperator {

std::string getOperationTypeName(Operation type)
{
    #define enum_case(t,e) case t::e: return #e;
    switch(type) {
        enum_case(Operation, ADD);
        enum_case(Operation, SUBTRACT);
        enum_case(Operation, MULTIPLY);
        enum_case(Operation, DIVIDE);
        default:
            return "UNKNOWN";
    }
}

std::string uppercase(const std::string& str)
{
    std::string out;
    std::transform(str.begin(), str.end(),
        std::back_inserter(out), ::toupper);
    return out;
}

Operation getOperationFromString(const std::string& str)
{
    std::string s = uppercase(str);
    #define enum_equals(t,e) if(s == #e) { return t::e; }
    enum_equals(Operation, ADD);
    enum_equals(Operation, SUBTRACT);
    enum_equals(Operation, MULTIPLY);
    enum_equals(Operation, DIVIDE);
    return Operation::UNKNOWN;
}


}

REGISTER_NUMERIC_STREAM_PROCESSOR(Adder);

    }
}