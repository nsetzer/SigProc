

#include "sigproc/framework/composite.hpp"
#include "sigproc/common/exception.hpp"

namespace {

    void quote(std::ostream& os, const std::string& s) {
        // TODO: some symbols should be escaped
        os << "\"" << s << "\"";
    }

} /* anonymous */


namespace sigproc {
    namespace framework {

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

/*
template<typename T>
T* Composite::value(T** p) {
    void* pValue = nullptr;

    if (getCompositeDataType<T>() == m_type) {
        switch (m_type) {
            case CompositeDataType::INT8:
                pValue = static_cast<void*>(&m_value.i8);
                break;
            case CompositeDataType::INT16:
                pValue = static_cast<void*>(&m_value.i16);
                break;
            case CompositeDataType::INT32:
                pValue = static_cast<void*>(&m_value.i32);
                break;
            case CompositeDataType::INT64:
                pValue = static_cast<void*>(&m_value.i64);
                break;
            case CompositeDataType::UINT8:
                pValue = static_cast<void*>(&m_value.u8);
                break;
            case CompositeDataType::UINT16:
                pValue = static_cast<void*>(&m_value.u16);
                break;
            case CompositeDataType::UINT32:
                pValue = static_cast<void*>(&m_value.u32);
                break;
            case CompositeDataType::UINT64:
                pValue = static_cast<void*>(&m_value.u64);
                break;
            case CompositeDataType::FLOAT32:
                pValue = static_cast<void*>(&m_value.f32);
                break;
            case CompositeDataType::FLOAT64:
                pValue = static_cast<void*>(&m_value.f64);
                break;
            default:
                SIGPROC_THROW("Illegal cast to " << m_type);
        }
    } else {
        SIGPROC_THROW("Illegal cast to " << getCompositeDataType<T>() << " from composite. Type is " << m_type);
    }
    if (p != nullptr) { *p = static_cast<T*>(pValue); }
    return static_cast<T*>(pValue);
}
*/


template<>
int64_t* Composite::value<int64_t>(int64_t** p) {
    int64_t* pValue = nullptr;
    switch (m_type) {
        case CompositeDataType::INT64:
            pValue = &m_value.i64;
            break;
        default:
            SIGPROC_THROW("Illegal cast to string from composite. Type is " <<
                getCompositeDataTypeName(m_type));
    }
    if (p != nullptr) { *p = pValue; }
    return pValue;
}


template<>
char** Composite::value<char*>(char*** p) {
    char** pValue = nullptr;
    switch (m_type) {
        case CompositeDataType::STRING:
            pValue = &m_value.str;
            break;
        default:
            SIGPROC_THROW("Illegal cast to string from composite. Type is " <<
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


bool Composite::as_bool()
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

int64_t Composite::as_int()
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

uint64_t Composite::as_uint()
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

double Composite::as_float()
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
std::string Composite::as_string()
{
    switch (m_type) {
        case CompositeDataType::STRING:
            return m_value.str;
        default:
            SIGPROC_THROW("Illegal cast to double from composite. Type is " <<
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
    int ws;
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
                quote(os, m_value.str);
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
                quote(os, kv.first);
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
