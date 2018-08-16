
#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"

#include <iostream>
#include <memory>

namespace sigproc {
    namespace framework {

template<typename T> CompositeDataType getCompositeDataType() { return CompositeDataType::UNKNOWN; }

template<> CompositeDataType getCompositeDataType<int8_t>() { return CompositeDataType::INT8; }
template<> CompositeDataType getCompositeDataType<int16_t>() { return CompositeDataType::INT16; }
template<> CompositeDataType getCompositeDataType<int32_t>() { return CompositeDataType::INT32; }
template<> CompositeDataType getCompositeDataType<int64_t>() { return CompositeDataType::INT64; }

template<> CompositeDataType getCompositeDataType<uint8_t>() { return CompositeDataType::UINT8; }
template<> CompositeDataType getCompositeDataType<uint16_t>() { return CompositeDataType::UINT16; }
template<> CompositeDataType getCompositeDataType<uint32_t>() { return CompositeDataType::UINT32; }
template<> CompositeDataType getCompositeDataType<uint64_t>() { return CompositeDataType::UINT64; }

template<> CompositeDataType getCompositeDataType<float>() { return CompositeDataType::FLOAT32; }
template<> CompositeDataType getCompositeDataType<double>() { return CompositeDataType::FLOAT64; }

template<> CompositeDataType getCompositeDataType<std::string>() { return CompositeDataType::STRING; }

template<> CompositeDataType getCompositeDataType<bool>() { return CompositeDataType::BOOL; }

template<> CompositeDataType getCompositeDataType<std::vector<std::unique_ptr<Composite>>>() { return CompositeDataType::SEQ; }
template<> CompositeDataType getCompositeDataType<std::map<std::string, std::unique_ptr<Composite>>>() { return CompositeDataType::MAP; }



template<typename T> ElementType getElementType() { return ElementType::UNKNOWN; }

template<> ElementType getElementType<int8_t>() { return ElementType::INT8; }
template<> ElementType getElementType<int16_t>() { return ElementType::INT16; }
template<> ElementType getElementType<int32_t>() { return ElementType::INT32; }
template<> ElementType getElementType<int64_t>() { return ElementType::INT64; }

template<> ElementType getElementType<uint8_t>() { return ElementType::UINT8; }
template<> ElementType getElementType<uint16_t>() { return ElementType::UINT16; }
template<> ElementType getElementType<uint32_t>() { return ElementType::UINT32; }
template<> ElementType getElementType<uint64_t>() { return ElementType::UINT64; }

template<> ElementType getElementType<float>() { return ElementType::FLOAT32; }
template<> ElementType getElementType<double>() { return ElementType::FLOAT64; }

template<> ElementType getElementType<std::string>() { return ElementType::STRING; }

template<> ElementType getElementType<std::vector<float>>() { return ElementType::FLOAT32VEC; }
template<> ElementType getElementType<std::vector<double>>() { return ElementType::FLOAT64VEC; }

template<> ElementType getElementType<Composite>() { return ElementType::COMPOSITE; }

const char* getCompositeStreamStateName(const CompositeStreamState& type) {

    switch(type) {
        case CompositeStreamState::TOKEN:
            return "TOKEN";
        case CompositeStreamState::NUMBER:
            return "NUMBER";
        case CompositeStreamState::STRING:
            return "STRING";
        case CompositeStreamState::SEQVAL:
            return "SEQVAL";
        case CompositeStreamState::MAPKEY:
            return "MAPKEY";
        case CompositeStreamState::MAPVAL:
            return "MAPVAL";
        default:
            return "UNKNOWN";
    }
}

const char* getCompositeDataTypeName(const CompositeDataType& type) {

    switch(type) {
        case CompositeDataType::INT8:
            return "INT8";
        case CompositeDataType::INT16:
            return "INT16";
        case CompositeDataType::INT32:
            return "INT32";
        case CompositeDataType::INT64:
            return "INT64";
        case CompositeDataType::UINT8:
            return "UINT8";
        case CompositeDataType::UINT16:
            return "UINT16";
        case CompositeDataType::UINT32:
            return "UINT32";
        case CompositeDataType::UINT64:
            return "UINT64";
        case CompositeDataType::FLOAT32:
            return "FLOAT32";
        case CompositeDataType::FLOAT64:
            return "FLOAT64";
        case CompositeDataType::STRING:
            return "STRING";
        case CompositeDataType::BOOL:
            return "BOOL";
        case CompositeDataType::SEQ:
            return "SEQ";
        case CompositeDataType::MAP:
            return "MAP";
        default:
            return "UNKNOWN";
    }
}

const char* getElementTypeName(const ElementType& type) {

    switch(type) {
        case ElementType::INT8:
            return "INT8";
        case ElementType::INT16:
            return "INT16";
        case ElementType::INT32:
            return "INT32";
        case ElementType::INT64:
            return "INT64";
        case ElementType::UINT8:
            return "UINT8";
        case ElementType::UINT16:
            return "UINT16";
        case ElementType::UINT32:
            return "UINT32";
        case ElementType::UINT64:
            return "UINT64";
        case ElementType::FLOAT32:
            return "FLOAT32";
        case ElementType::FLOAT64:
            return "FLOAT64";
        case ElementType::STRING:
            return "STRING";
        case ElementType::COMPOSITE:
            return "COMPOSITE";
        case ElementType::FLOAT32VEC:
            return "FLOAT32VEC";
        case ElementType::FLOAT64VEC:
            return "FLOAT64VEC";
        default:
            return "UNKNOWN";
    }
}

    } // framework
} // sigproc

std::ostream& operator << (std::ostream& os, const sigproc::framework::CompositeStreamState& type)
{
    os << sigproc::framework::getCompositeStreamStateName(type);
    return os;
}

std::ostream& operator << (std::ostream& os, const sigproc::framework::ElementType& type)
{
    os << sigproc::framework::getElementTypeName(type);
    return os;
}


std::ostream& operator << (std::ostream& os, const sigproc::framework::CompositeDataType& type)
{
    os << sigproc::framework::getCompositeDataTypeName(type);
    return os;
}