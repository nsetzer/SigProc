
#include <utility>

#include "sigproc/common/exception.hpp"
#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/lispexpr.hpp"

namespace sigproc {
    namespace framework {


namespace {

// truth table
// like kinds produce like kind,
// while mixed types promote to float
//  v1| v2|res ::  v1| v2|res ::  v1| v2|res ::  v1| v2|res
// int|int|int :: uns|int|flt :: flt|int|flt :: str|int|err
// int|uns|flt :: uns|uns|uns :: flt|uns|flt :: str|uns|err
// int|flt|flt :: uns|flt|flt :: flt|flt|flt :: str|flt|err
// int|str|err :: uns|str|err :: flt|str|err :: str|str|err

template<typename T>
Composite normalize_rhs(const Composite& v) {
    return Composite(CompositeDataType::UNKNOWN);
}

template<>
Composite normalize_rhs<int64_t>(const Composite& v) {

    switch (v.type()) {
        case CompositeDataType::INT8:
        case CompositeDataType::INT16:
        case CompositeDataType::INT32:
        case CompositeDataType::INT64:
            return v.as_int();
        case CompositeDataType::UINT8:
        case CompositeDataType::UINT16:
        case CompositeDataType::UINT32:
        case CompositeDataType::UINT64:
            return static_cast<double>(v.as_uint());
        case CompositeDataType::FLOAT32:
        case CompositeDataType::FLOAT64:
            return v.as_float();
        default:
            SIGPROC_THROW("illegal type conversion lhs: " <<
                getElementType<int64_t>() <<
                "rhs: " << v.type());
    }
}

template<>
Composite normalize_rhs<uint64_t>(const Composite& v) {

    switch (v.type()) {
        case CompositeDataType::INT8:
        case CompositeDataType::INT16:
        case CompositeDataType::INT32:
        case CompositeDataType::INT64:
            return static_cast<double>(v.as_int());
        case CompositeDataType::UINT8:
        case CompositeDataType::UINT16:
        case CompositeDataType::UINT32:
        case CompositeDataType::UINT64:
            return v.as_uint();
        case CompositeDataType::FLOAT32:
        case CompositeDataType::FLOAT64:
            return v.as_float();
        default:
            SIGPROC_THROW("illegal type conversion lhs: " <<
                getElementType<int64_t>() <<
                "rhs: " << v.type());
    }
}

template<>
Composite normalize_rhs<double>(const Composite& v) {

    switch (v.type()) {
        case CompositeDataType::INT8:
        case CompositeDataType::INT16:
        case CompositeDataType::INT32:
        case CompositeDataType::INT64:
            return static_cast<double>(v.as_int());
        case CompositeDataType::UINT8:
        case CompositeDataType::UINT16:
        case CompositeDataType::UINT32:
        case CompositeDataType::UINT64:
            return static_cast<double>(v.as_uint());
        case CompositeDataType::FLOAT32:
        case CompositeDataType::FLOAT64:
            return v.as_float();
        default:
            SIGPROC_THROW("illegal type conversion lhs: " <<
                getElementType<int64_t>() <<
                "rhs: " << v.type());
    }
}

// returns two composite values of the same type
// throws an exception if the input types cannot be coerced into the same type
std::pair<Composite,Composite> normalize_types(const Composite& v1, const Composite& v2) {
    Composite v2_norm;
    switch (v1.type()) {
        case CompositeDataType::INT8:
        case CompositeDataType::INT16:
        case CompositeDataType::INT32:
        case CompositeDataType::INT64:
            v2_norm = normalize_rhs<int64_t>(v2);
            if (v2_norm.type() == CompositeDataType::INT64) {
                return std::pair<Composite,Composite>(v1.as_int(), v2_norm);
            }
            return std::pair<Composite,Composite>(
                static_cast<double>(v1.as_int()), v2_norm);
        case CompositeDataType::UINT8:
        case CompositeDataType::UINT16:
        case CompositeDataType::UINT32:
        case CompositeDataType::UINT64:
            v2_norm = normalize_rhs<uint64_t>(v2);
            if (v2_norm.type() == CompositeDataType::UINT64) {
                return std::pair<Composite,Composite>(v1.as_uint(), v2_norm);
            }
            return std::pair<Composite,Composite>(
                static_cast<double>(v1.as_uint()), v2_norm);
        case CompositeDataType::FLOAT32:
        case CompositeDataType::FLOAT64:
            v2_norm = normalize_rhs<double>(v2);
            return std::pair<Composite,Composite>(v1.as_float(), v2_norm);
        default:
            SIGPROC_THROW("illegal type conversion lhs: " <<
                v1.type() << "rhs: " << v2.type());
    }
    return std::pair<Composite,Composite>(0,0);
}

} // anonymous

Composite lisp_binary_add(const Composite& v1, const Composite& v2) {
    if (v1.type() != v2.type()) {
        SIGPROC_THROW("illegal operation (+) lhs: " <<
            v1.type() << "rhs: " << v2.type());
    }

    switch (v1.type()) {
        case CompositeDataType::INT64:
            return v1.as_int() + v2.as_int();
        case CompositeDataType::UINT64:
            return v1.as_uint() + v2.as_uint();
        case CompositeDataType::FLOAT64:
            return v1.as_float() + v2.as_float();
        default:
            SIGPROC_THROW("illegal operation (+) lhs: " <<
                v1.type() << "rhs: " << v2.type());
    }
}

Composite lisp_binary_subtract(const Composite& v1, const Composite& v2) {
    if (v1.type() != v2.type()) {
        SIGPROC_THROW("illegal operation (+) lhs: " <<
            v1.type() << "rhs: " << v2.type());
    }

    switch (v1.type()) {
        case CompositeDataType::INT64:
            return v1.as_int() - v2.as_int();
        case CompositeDataType::UINT64:
            return v1.as_uint() - v2.as_uint();
        case CompositeDataType::FLOAT64:
            return v1.as_float() - v2.as_float();
        default:
            SIGPROC_THROW("illegal operation (+) lhs: " <<
                v1.type() << "rhs: " << v2.type());
    }
}

Composite lisp_binary_multiply(const Composite& v1, const Composite& v2) {
    if (v1.type() != v2.type()) {
        SIGPROC_THROW("illegal operation (+) lhs: " <<
            v1.type() << "rhs: " << v2.type());
    }

    switch (v1.type()) {
        case CompositeDataType::INT64:
            return v1.as_int() * v2.as_int();
        case CompositeDataType::UINT64:
            return v1.as_uint() * v2.as_uint();
        case CompositeDataType::FLOAT64:
            return v1.as_float() * v2.as_float();
        default:
            SIGPROC_THROW("illegal operation (+) lhs: " <<
                v1.type() << "rhs: " << v2.type());
    }
}

Composite lisp_binary_divide(const Composite& v1, const Composite& v2) {
    if (v1.type() != v2.type()) {
        SIGPROC_THROW("illegal operation (+) lhs: " <<
            v1.type() << "rhs: " << v2.type());
    }

    switch (v1.type()) {
        case CompositeDataType::INT64:
            return v1.as_int() / v2.as_int();
        case CompositeDataType::UINT64:
            return v1.as_uint() / v2.as_uint();
        case CompositeDataType::FLOAT64:
            return v1.as_float() / v2.as_float();
        default:
            SIGPROC_THROW("illegal operation (+) lhs: " <<
                v1.type() << "rhs: " << v2.type());
    }
}

upComposite lisp_plus(CompositeVector& vec) {

    // TODO: types are uint64_t > int64_t > double
    // allow mixed types and promote in that order
    // also allow combining strings

    if (vec.size() == 2) {
        auto& ptr = vec[1];
        return std::unique_ptr<Composite>(new Composite(*ptr));
    }
    // vec size must be 3 or more

    std::pair<Composite,Composite> pair;
    Composite result;

    pair = normalize_types(*vec[1], *vec[2]);
    result = lisp_binary_add(pair.first, pair.second);

    for (size_t i=3; i<vec.size(); i++) {
        pair = normalize_types(result, *vec[i]);
        result = lisp_binary_add(pair.first, pair.second);
    }

    return std::unique_ptr<Composite>(new Composite(result));
}

upComposite lisp_subtract(CompositeVector& vec) {

    if (vec.size() == 2) {
        auto& ptr = vec[1];
        return std::unique_ptr<Composite>(new Composite(*ptr));
    }
    // vec size must be 3 or more

    std::pair<Composite,Composite> pair;
    Composite result;

    pair = normalize_types(*vec[1], *vec[2]);
    result = lisp_binary_subtract(pair.first, pair.second);

    for (size_t i=3; i<vec.size(); i++) {
        pair = normalize_types(result, *vec[i]);
        result = lisp_binary_subtract(pair.first, pair.second);
    }

    return std::unique_ptr<Composite>(new Composite(result));

}

upComposite lisp_multiply(CompositeVector& vec) {

    if (vec.size() == 2) {
        auto& ptr = vec[1];
        return std::unique_ptr<Composite>(new Composite(*ptr));
    }
    // vec size must be 3 or more

    std::pair<Composite,Composite> pair;
    Composite result;

    pair = normalize_types(*vec[1], *vec[2]);
    result = lisp_binary_multiply(pair.first, pair.second);

    for (size_t i=3; i<vec.size(); i++) {
        pair = normalize_types(result, *vec[i]);
        result = lisp_binary_multiply(pair.first, pair.second);
    }

    return std::unique_ptr<Composite>(new Composite(result));

}

upComposite lisp_divide(CompositeVector& vec) {

    if (vec.size() == 2) {
        auto& ptr = vec[1];
        return std::unique_ptr<Composite>(new Composite(*ptr));
    }
    // vec size must be 3 or more

    std::pair<Composite,Composite> pair;
    Composite result;

    pair = normalize_types(*vec[1], *vec[2]);
    result = lisp_binary_divide(pair.first, pair.second);

    for (size_t i=3; i<vec.size(); i++) {
        pair = normalize_types(result, *vec[i]);
        result = lisp_binary_divide(pair.first, pair.second);
    }

    return std::unique_ptr<Composite>(new Composite(result));

}


   } // framework
} // sigproc


