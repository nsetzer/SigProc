

#include "sigproc/framework/composite.hpp"
#include "sigproc/common/exception.hpp"


namespace sigproc {
    namespace framework {


namespace composite {

    void quote(std::ostream& os, const std::string& s) {
        // TODO: some symbols should be escaped
        os << "\"" << s << "\"";
    }

    std::string unquote(const std::string& s) {
        // TODO: some escape symbols should be handled
        return s.substr(1, s.size()-2);
    }

    bool parse_int64(const std::string& s, int64_t* i) {
        char * p ;

        if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) {
            return false;
        }

        *i = strtoll(s.c_str(), &p, 10);

        return (*p == 0) ;
    }

    bool parse_double(const std::string& s, double* d) {
        char * p ;

        if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) {
            return false;
        }

        *d = strtold(s.c_str(), &p);

        return (*p == 0) ;
    }

    Composite* parse(const std::string& s) {

        int64_t i;
        double d;
        if(parse_int64(s, &i)) {
            return new Composite(i);
        }

        if(parse_double(s, &d)) {
            return new Composite(d);
        }

        return new Composite(s);

    }

} /* composite */

void* Composite::value(CompositeDataType type) {

    if (type != m_type) {
        // todo throw;
        return nullptr;
    }

    switch(m_type) {
        case CompositeDataType::BOOL:
        case CompositeDataType::INT8:
            return &m_value.i8;
        case CompositeDataType::INT16:
            return &m_value.i16;
        case CompositeDataType::INT32:
            return &m_value.i32;
        case CompositeDataType::INT64:
            return &m_value.i64;
        case CompositeDataType::UINT8:
            return &m_value.u8;
        case CompositeDataType::UINT16:
            return &m_value.u16;
        case CompositeDataType::UINT32:
            return &m_value.u32;
        case CompositeDataType::UINT64:
            return &m_value.u64;
        case CompositeDataType::FLOAT32:
            return &m_value.f32;
        case CompositeDataType::FLOAT64:
            return &m_value.f64;
        case CompositeDataType::STRING:
            return m_value.str;
        case CompositeDataType::SEQ:
            return m_value.vec;
        case CompositeDataType::MAP:
            return m_value.map;
        default:
            return  nullptr;
    }
}

template<typename T>
T* Composite::value(T** p) {
    if (p != nullptr) { *p = nullptr; }
    return nullptr;
}

#define DEFINE_VALUE_ACCESSOR(TYPE, ENUM, ATTR) template<> \
    TYPE* Composite::value<TYPE>(TYPE** p) { \
        TYPE* pValue = nullptr; \
        switch (m_type) { \
            case ENUM: \
                pValue = &ATTR; \
                break; \
            default: \
                SIGPROC_THROW("Illegal cast to " \
                    << getCompositeDataType<TYPE>() \
                    << " from composite. Type is " << \
                    getCompositeDataTypeName(m_type)); \
        } \
        if (p != nullptr) { *p = pValue; } \
        return pValue; \
    }

DEFINE_VALUE_ACCESSOR(int16_t, CompositeDataType::INT16, m_value.i16);
DEFINE_VALUE_ACCESSOR(int32_t, CompositeDataType::INT32, m_value.i32);
DEFINE_VALUE_ACCESSOR(int64_t, CompositeDataType::INT64, m_value.i64);
DEFINE_VALUE_ACCESSOR(uint8_t, CompositeDataType::UINT8, m_value.u8);
DEFINE_VALUE_ACCESSOR(uint16_t, CompositeDataType::UINT16, m_value.u16);
DEFINE_VALUE_ACCESSOR(uint32_t, CompositeDataType::UINT32, m_value.u32);
DEFINE_VALUE_ACCESSOR(uint64_t, CompositeDataType::UINT64, m_value.u64);
DEFINE_VALUE_ACCESSOR(float, CompositeDataType::FLOAT32, m_value.f32);
DEFINE_VALUE_ACCESSOR(double, CompositeDataType::FLOAT64, m_value.f64);
DEFINE_VALUE_ACCESSOR(char*, CompositeDataType::STRING, m_value.str);

template<>
int8_t* Composite::value<int8_t>(int8_t** p) {
    int8_t* pValue = nullptr;
    switch (m_type) {
        case CompositeDataType::BOOL:
        case CompositeDataType::INT8:
            pValue = &m_value.i8;
            break;
        default:
            SIGPROC_THROW("Illegal cast to "
                << getCompositeDataType<int8_t>()
                << " from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
    if (p != nullptr) { *p = pValue; }
    return pValue;
}

template<>
CompositeVector* Composite::value<CompositeVector>(CompositeVector** p) {
    CompositeVector* pValue = nullptr;
    switch (m_type) {
        case CompositeDataType::SEQ:
            pValue = m_value.vec;
            break;
        default:
            SIGPROC_THROW("Illegal cast to sequence from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
    if (p != nullptr) { *p = pValue; }
    return pValue;
}

template<>
CompositeMap* Composite::value<CompositeMap>(CompositeMap** p) {
    CompositeMap* pValue = nullptr;
    switch (m_type) {
        case CompositeDataType::MAP:
            pValue = m_value.map;
            break;
        default:
            SIGPROC_THROW("Illegal cast to map from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
    if (p != nullptr) { *p = pValue; }
    return pValue;
}


bool Composite::as_bool() const
{
    switch (m_type) {
        case CompositeDataType::BOOL:
        case CompositeDataType::INT8:
            return m_value.i8;
        case CompositeDataType::INT16:
            return m_value.i16;
        case CompositeDataType::INT32:
            return m_value.i32;
        case CompositeDataType::INT64:
            return m_value.i64;
        case CompositeDataType::UINT8:
            return m_value.u8;
        case CompositeDataType::UINT16:
            return m_value.u16;
        case CompositeDataType::UINT32:
            return m_value.u32;
        case CompositeDataType::UINT64:
            return m_value.u64;
        default:
            SIGPROC_THROW("Illegal cast to bool from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

int64_t Composite::as_int() const
{
    switch (m_type) {
        case CompositeDataType::BOOL:
        case CompositeDataType::INT8:
            return static_cast<int64_t>(m_value.i8);
        case CompositeDataType::INT16:
            return static_cast<int64_t>(m_value.i16);
        case CompositeDataType::INT32:
            return static_cast<int64_t>(m_value.i32);
        case CompositeDataType::INT64:
            return m_value.i64;
        default:
            SIGPROC_THROW("Illegal cast to int from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

uint64_t Composite::as_uint() const
{
    switch (m_type) {
        case CompositeDataType::BOOL:
            return m_value.i8?1:0;
        case CompositeDataType::UINT8:
            return static_cast<uint64_t>(m_value.u8);
        case CompositeDataType::UINT16:
            return static_cast<uint64_t>(m_value.u16);
        case CompositeDataType::UINT32:
            return static_cast<uint64_t>(m_value.u32);
        case CompositeDataType::UINT64:
            return m_value.u64;
        default:
            SIGPROC_THROW("Illegal cast to unsigned int from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

double Composite::as_float() const
{
    switch (m_type) {
        case CompositeDataType::FLOAT32:
            return static_cast<double>(m_value.f32);
        case CompositeDataType::FLOAT64:
            return m_value.f64;
        default:
            SIGPROC_THROW("Illegal cast to double from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}
std::string Composite::as_string() const
{
    switch (m_type) {
        case CompositeDataType::STRING:
            return m_value.str;
        default:
            SIGPROC_THROW("Illegal cast to double from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

CompositeVector& Composite::as_vector()
{
    switch (m_type) {
        case CompositeDataType::SEQ:
            return *m_value.vec;
        default:
            SIGPROC_THROW("Illegal cast to vector from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

CompositeVector& Composite::as_vector() const
{
    switch (m_type) {
        case CompositeDataType::SEQ:
            return *m_value.vec;
        default:
            SIGPROC_THROW("Illegal cast to vector from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

CompositeMap& Composite::as_map()
{
    switch (m_type) {
        case CompositeDataType::MAP:
            return *m_value.map;
        default:
            SIGPROC_THROW("Illegal cast to map from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}

const CompositeMap& Composite::as_map() const
{
    switch (m_type) {
        case CompositeDataType::MAP:
            return *m_value.map;
        default:
            SIGPROC_THROW("Illegal cast to map from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
}


std::ostream& operator << (std::ostream& os, const CompositeDataType& obj)
{
   os << static_cast<std::underlying_type<CompositeDataType>::type>(obj);
   return os;
}

void Composite::print(std::ostream& os, size_t depth, size_t tab_width, bool pretty) const
{
    bool first = true; // used to control pretty printing
    size_t ws;
    switch(m_type) {
        case CompositeDataType::BOOL:
            os << ((m_value.i8)?"true":"false");
            break;
        case CompositeDataType::INT8:
            os << m_value.i8;
            break;
        case CompositeDataType::INT16:
            os << m_value.i16;
            break;
        case CompositeDataType::INT32:
            os << m_value.i32;
            break;
        case CompositeDataType::INT64:
            os << m_value.i64;
            break;
        case CompositeDataType::UINT8:
            os << m_value.u8;
            break;
        case CompositeDataType::UINT16:
            os << m_value.u16;
            break;
        case CompositeDataType::UINT32:
            os << m_value.u32;
            break;
        case CompositeDataType::UINT64:
            os << m_value.u64;
            break;
        case CompositeDataType::FLOAT32:
            os << m_value.f32;
            break;
        case CompositeDataType::FLOAT64:
            os << m_value.f64;
            break;
        case CompositeDataType::STRING:
            if (m_value.str != nullptr) {
                composite::quote(os, m_value.str);
            } else {
                os << "null";
            }
            break;
        case CompositeDataType::SEQ:
            // serialize a vector as json
            // no comma after the final value
            os << "[";
            if (pretty && m_value.map->size()>0) {
                os << std::endl;
                for (ws=0; ws < (1+depth)*(tab_width); ws++) { os << " "; }
            }
            for (const auto& p : *m_value.vec) {

                if (first==false) {
                    os << ", ";
                    if (pretty) {
                       os << std::endl;
                       for (ws=0; ws < (1+depth)*(tab_width); ws++) { os << " "; }
                    }
                }

                if (!p) {
                    os << "null";
                } else {
                    p->print(os, depth+1, tab_width, pretty);
                }

                first = false;
            }
            if (pretty && m_value.vec->size()>0) {
                os << std::endl;
                for (ws=0; ws < depth*tab_width; ws++) { os << " "; }
            }
            os << "]";
            //if (pretty) {
            //    os << std::endl;
            //    for (ws=0; ws < depth*tab_width; ws++) { os << " "; }
            //}
            break;
        case CompositeDataType::MAP:
            // serialize a map as json
            // no comma after the final value
            os << "{";
            if (pretty && m_value.map->size()>0) {
                os << std::endl;
                for (ws=0; ws < (1+depth)*(tab_width); ws++) { os << " "; }
            }
            for (const auto& kv : *m_value.map) {
                if (first==false) {
                    os << ", ";
                    if (pretty) {
                       os << std::endl;
                       for (ws=0; ws < (1+depth)*(tab_width); ws++) { os << " "; }
                    }
                }
                composite::quote(os, kv.first);
                os << ": ";
                if (!kv.second) {
                    os << "null";
                } else {
                    kv.second->print(os, depth+1, tab_width, pretty);
                }
                first = false;
            }
            if (pretty && m_value.map->size()>0) {
                os << std::endl;
                for (ws=0; ws < depth*tab_width; ws++) { os << " "; }
            }
            os << "}";
            //if (pretty) {
            //    os << std::endl;
            //    for (ws=0; ws < depth*tab_width; ws++) { os << " "; }
            //}
            break;
        default:
            SIGPROC_THROW("invalid type" << Val(m_type));
            break;
    }

}

/**
 * JSON encoder for Composite
 */
std::ostream& operator << (std::ostream& os, const Composite& obj)
{
    obj.print(os, 0, 0, false);
    return os;
}

    } // framework
} // sigproc
