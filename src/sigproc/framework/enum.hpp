

#ifndef SIGPROC_FRAMEWORK_ENUM_HPP
#define SIGPROC_FRAMEWORK_ENUM_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>

namespace sigproc {
    namespace framework {

enum class CompositeStreamState : char {
    UNKNOWN = 0,
    // token states
    TOKEN   = 1,
    NUMBER  = 2,
    STRING  = 3,
    // meta states
    SEQVAL  = 4,
    MAPKEY  = 5,
    MAPVAL  = 6,
};

enum class CompositeDataType : char {
    UNKNOWN = 0,
    INT8    = 1,
    INT16   = 2,
    INT32   = 3,
    INT64   = 4,
    UINT8   = 5,
    UINT16  = 6,
    UINT32  = 7,
    UINT64  = 8,
    FLOAT32 = 9,
    FLOAT64 = 10,
    STRING  = 11,
    BOOL    = 12,
    SEQ     = 13,
    MAP     = 14
};

enum class ElementType: char {
    UNKNOWN    = 0,
    INT8       = 1,
    INT16      = 2,
    INT32      = 3,
    INT64      = 4,
    UINT8      = 5,
    UINT16     = 6,
    UINT32     = 7,
    UINT64     = 8,
    FLOAT32    = 9,
    FLOAT64    = 10,
    STRING     = 11,
    COMPOSITE  = 12,

    FLOAT32VEC = 109,
    FLOAT64VEC = 110,
};

enum class IntervalType: char {
    UNKNOWN   = 0,
    REGULAR   = 1,
    IRREGULAR = 2,
    MOVING    = 3,
};

enum class InputConfigType: char {
    UNKNOWN   = 0,
    ISTREAM   = 1,
    PORT      = 2,
    PORTBANK  = 3
};

enum class OutputConfigType: char {
    UNKNOWN    = 0,
    OSTREAM    = 1,
    STREAM     = 2,
    STREAMBANK = 3,
};

enum class NodeConfigElementType: char {
    UNKNOWN=0,
    UNARY=1,
    COMPOSITE=2
};

template<typename T> CompositeDataType getCompositeDataType();
template<> CompositeDataType getCompositeDataType<int8_t>();
template<> CompositeDataType getCompositeDataType<int16_t>();
template<> CompositeDataType getCompositeDataType<int32_t>();
template<> CompositeDataType getCompositeDataType<int64_t>();
template<> CompositeDataType getCompositeDataType<uint8_t>();
template<> CompositeDataType getCompositeDataType<uint16_t>();
template<> CompositeDataType getCompositeDataType<uint32_t>();
template<> CompositeDataType getCompositeDataType<uint64_t>();
template<> CompositeDataType getCompositeDataType<float>();
template<> CompositeDataType getCompositeDataType<double>();
template<> CompositeDataType getCompositeDataType<std::string>();
template<> CompositeDataType getCompositeDataType<bool>();
class Composite;
template<> CompositeDataType getCompositeDataType<std::vector<std::unique_ptr<Composite>>>();
template<> CompositeDataType getCompositeDataType<std::map<std::string, std::unique_ptr<Composite>>>();


template<typename T> ElementType getElementType();
template<> ElementType getElementType<int8_t>();
template<> ElementType getElementType<int16_t>();
template<> ElementType getElementType<int32_t>();
template<> ElementType getElementType<int64_t>();
template<> ElementType getElementType<uint8_t>();
template<> ElementType getElementType<uint16_t>();
template<> ElementType getElementType<uint32_t>();
template<> ElementType getElementType<uint64_t>();
template<> ElementType getElementType<float>();
template<> ElementType getElementType<double>();
template<> ElementType getElementType<std::string>();
template<> ElementType getElementType<std::vector<float>>();
template<> ElementType getElementType<std::vector<double>>();
class Composite;
template<> ElementType getElementType<Composite>();

const char* getCompositeStreamStateName(const CompositeStreamState& type);
const char* getCompositeDataTypeName(const CompositeDataType& type);
const char* getElementTypeName(const ElementType& type);

template<typename T> std::string getTypeName() {
    return getElementTypeName(getElementType<T>());
}

    } // framework
} // sigproc

std::ostream& operator << (std::ostream& os, const sigproc::framework::CompositeStreamState& type);
std::ostream& operator << (std::ostream& os, const sigproc::framework::CompositeDataType& type);
std::ostream& operator << (std::ostream& os, const sigproc::framework::ElementType& type);


#endif