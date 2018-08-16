
#ifndef SIGPROC_FRAMEWORK_COMPOSITE_HPP
#define SIGPROC_FRAMEWORK_COMPOSITE_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <memory>
#include <exception>

#include "sigproc/framework/enum.hpp"

namespace sigproc {
    namespace framework {

/**
 * CompositeDataValue represents a node in a document
 */
class Composite;

typedef std::vector<std::unique_ptr<Composite>> CompositeVector;
typedef std::map<std::string, std::unique_ptr<Composite>> CompositeMap;


// todo: allow booleans as int8_t
//       allow automatic promotion to larger integer or float types
//       allow bool to be promoted to any integer type
//       allow demotion of any integer to a bool
//       allow demotion of a string to a bool (null: false otherwise true)
//       are empty strings, seqs, or maps also false?

union CompositeDataValue
{
    int8_t      i8;
    int16_t     i16;
    int32_t     i32;
    int64_t     i64;
    uint8_t     u8;
    uint16_t    u16;
    uint32_t    u32;
    uint64_t    u64;
    float       f32;
    double      f64;
    char*       str;
    CompositeVector* vec;
    CompositeMap* map;

    // union constructor to null initialize
    CompositeDataValue() { memset( this, 0, sizeof( CompositeDataValue ) ); }
};

/**
 * Composite is the implementation for a signal value representing a document
 * an Composite can be passed around the graph in the same way as any other
 * basic type
 */

class Composite
{
    CompositeDataType m_type;
    CompositeDataValue m_value;
public:

    Composite()
        : m_type(CompositeDataType::UNKNOWN)
        , m_value()
    {}

    // this constructor is useful for initializing composite types.
    Composite(CompositeDataType type)
        : m_type(type)
        , m_value()
    {
        switch(m_type) {
            case CompositeDataType::STRING:
                //this is not a useful option with no no way to update/replace
                m_value.str = new char[1];
                m_value.str[0] = '\0';
            case CompositeDataType::SEQ:
                m_value.vec = new CompositeVector;
                break;
            case CompositeDataType::MAP:
                m_value.map = new CompositeMap;
                break;
            default:
                break;
        }
    }

    Composite(bool data) {
            m_type = CompositeDataType::BOOL;
            m_value.i8 = data;
        }
    Composite(int8_t data) {
            m_type = CompositeDataType::INT8;
            m_value.i8 = data;
        }
    Composite(int16_t data) {
            m_type = CompositeDataType::INT16;
            m_value.i16 = data;
        }
    Composite(int32_t data) {
            m_type = CompositeDataType::INT32;
            m_value.i32 = data;
        }
    Composite(int64_t data) {
            m_type = CompositeDataType::INT64;
            m_value.i64 = data;
        }
    Composite(uint8_t data) {
            m_type = CompositeDataType::UINT8;
            m_value.u8 = data;
        }
    Composite(uint16_t data) {
            m_type = CompositeDataType::UINT16;
            m_value.u16 = data;
        }
    Composite(uint32_t data) {
            m_type = CompositeDataType::UINT32;
            m_value.u32 = data;
        }
    Composite(uint64_t data) {
            m_type = CompositeDataType::INT64;
            m_value.u64 = data;
        }
    Composite(float data) {
            m_type = CompositeDataType::FLOAT32;
            m_value.f32 = data;
        }
    Composite(double data) {
            m_type = CompositeDataType::FLOAT64;
            m_value.f64 = data;
        }
    Composite(const char* data)  {
            m_type = CompositeDataType::STRING;
            if (data != nullptr) {
                _copy_str(data);
            } else {
                m_value.str = nullptr;
            }
        }
    Composite(const std::string& data)  {
            m_type = CompositeDataType::STRING;
            _copy_str(data);
        }
    // constructor which performs a deep copy of the arguments
    Composite(const CompositeVector* data) {
            m_type = CompositeDataType::SEQ;
            _copy_seq(*data);
        }
    // constructor which performs a deep copy of the arguments
    Composite(const CompositeVector& data) {
            m_type = CompositeDataType::SEQ;
            _copy_seq(data);
        }
    // constructor which performs a deep copy of the arguments
    Composite(const CompositeMap* data) {
            m_type = CompositeDataType::MAP;
            _copy_map(*data);
        }
    // constructor which performs a deep copy of the arguments
    Composite(const CompositeMap& data) {
            m_type = CompositeDataType::MAP;
            _copy_map(data);
        }

    Composite(const std::unique_ptr<Composite>& other)
        : m_type(other->m_type) {
            switch(m_type) {
                case CompositeDataType::STRING:
                    if (other->m_value.str==nullptr) {
                        m_value.str = nullptr;
                    } else {
                        _copy_str(other->m_value.str);
                    }
                    break;
                case CompositeDataType::SEQ:
                    _copy_seq(*other->m_value.vec);
                    break;
                case CompositeDataType::MAP:
                    _copy_map(*other->m_value.map);
                    break;
                default:
                    m_value = other->m_value;
                    break;
            }
        }
    Composite(const Composite& other) {
            m_type = other.m_type;

            switch(m_type) {
                case CompositeDataType::STRING:
                    if (m_value.str!=nullptr) {
                        delete[] m_value.str;
                    }
                    if (other.m_value.str == nullptr) {
                        m_value.str = nullptr;
                    } else {
                        _copy_str(other.m_value.str);
                    }
                    break;
                case CompositeDataType::SEQ:
                    if (m_value.vec!=nullptr) {
                        delete m_value.vec;
                    }
                    _copy_seq(*other.m_value.vec);
                    break;
                case CompositeDataType::MAP:
                    if (m_value.map!=nullptr) {
                        delete m_value.map;
                    }
                    _copy_map(*other.m_value.map);
                    break;
                default:
                    m_value = other.m_value;
                    break;
            }
        }

    ~Composite() {
        switch(m_type) {
            case CompositeDataType::STRING:
                if (m_value.str!=nullptr) {
                    delete[] m_value.str;
                }
                break;
            case CompositeDataType::SEQ:
                delete m_value.vec;
                break;
            case CompositeDataType::MAP:
                delete m_value.map;
                break;
            default:
                break;
        }
    }

    Composite& operator=(const Composite& other) {
            m_type = other.m_type;

            switch(m_type) {
                case CompositeDataType::STRING:
                    if (m_value.str!=NULL) {
                        delete[] m_value.str;
                    }
                    _copy_str(other.m_value.str);
                    break;
                case CompositeDataType::SEQ:
                    _copy_seq(*other.m_value.vec);
                    break;
                case CompositeDataType::MAP:
                    _copy_map(*other.m_value.map);
                    break;
                default:
                    m_value = other.m_value;
                    break;
            }
            return *this;
        }

    CompositeDataType type() const { return m_type; }

    // access a raw pointer to the underlying type
    // throws if the given type does not match the actual type
    void* value(CompositeDataType type);

    // mix of compile and runtime type checking
    // for accessing the underlying value of the payload
    template<typename T> T* value(T** p);

    // basic type conversions
    bool as_bool() const;
    int64_t as_int() const;
    uint64_t as_uint() const;
    double as_float() const;
    std::string as_string() const;
    CompositeVector& as_vector();
    CompositeVector& as_vector() const;
    CompositeMap& as_map();
    CompositeMap& as_map() const;

    void print(std::ostream& os, size_t depth, size_t tab_width, bool pretty) const;

    friend std::ostream& operator << (std::ostream& os, const Composite& obj);

    template<typename T>
    static std::unique_ptr<Composite> makeUnique(T data) {
        return std::unique_ptr<Composite>(new Composite(data));
    }

private:

    void _copy_str(const std::string& data) {
        size_t len = data.size() + 1;
        m_value.str = new char[len];
        memcpy(m_value.str, data.c_str(), len);
    }

    void _copy_seq(const CompositeVector& data) {
        // this is a deep copy...
        m_value.vec = new CompositeVector;
        for (auto& p: data) {
            m_value.vec->push_back(
                std::unique_ptr<Composite>(new Composite(p))
                );
        }
    }

    void _copy_map(const CompositeMap& data) {
        // this is a deep copy...
        m_value.map = new CompositeMap;
        for (const auto& kv: data) {
            m_value.map->insert(m_value.map->begin(),
                    std::pair<std::string, std::unique_ptr<Composite>>(
                        kv.first,
                        std::unique_ptr<Composite>(new Composite(kv.second))
                    ));
        }
    }

};

    } // framework
} // sigproc



#endif
