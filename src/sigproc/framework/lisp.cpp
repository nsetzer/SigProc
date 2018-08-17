
#include <iterator>
#include <algorithm>
#include <map>
#include <functional>

#include "sigproc/framework/lisp.hpp"

namespace sigproc {
    namespace framework {

namespace {

typedef std::unique_ptr<Composite> upComposite;
typedef std::function<upComposite(CompositeVector&)> LispFunction;
typedef std::map<std::string, LispFunction> LispFunctionMap;

// addition truth table time
//  v1| v2|res ::  v1| v2|res ::  v1| v2|res ::  v1| v2|res
// int|int|int :: uns|int|int :: flt|int|flt :: str|int|str
// int|uns|int :: uns|uns|uns :: flt|uns|flt :: str|uns|str
// int|flt|flt :: uns|flt|flt :: flt|flt|flt :: str|flt|str
// int|str|err :: uns|str|err :: flt|str|err :: str|str|str
// int|seq|err :: uns|seq|err :: flt|seq|err :: str|seq|str
// int|map|err :: uns|map|err :: flt|map|err :: str|map|str

// subtraction truth table time
//  v1| v2|res ::  v1| v2|res ::  v1| v2|res ::  v1| v2|res
// int|int|int :: uns|int|int :: flt|int|flt :: str|int|err
// int|uns|int :: uns|uns|uns :: flt|uns|flt :: str|uns|err
// int|flt|flt :: uns|flt|flt :: flt|flt|flt :: str|flt|err
// int|str|err :: uns|str|err :: flt|str|err :: str|str|err
// int|seq|err :: uns|seq|err :: flt|seq|err :: str|seq|str
// int|map|err :: uns|map|err :: flt|map|err :: str|map|str

// multiplication truth table time
//  v1| v2|res ::  v1| v2|res ::  v1| v2|res ::  v1| v2|res
// int|int|int :: uns|int|int :: flt|int|flt :: str|int|str
// int|uns|int :: uns|uns|uns :: flt|uns|flt :: str|uns|str
// int|flt|flt :: uns|flt|flt :: flt|flt|flt :: str|flt|err
// int|str|err :: uns|str|err :: flt|str|err :: str|str|err
// int|seq|err :: uns|seq|err :: flt|seq|err :: str|seq|str
// int|map|err :: uns|map|err :: flt|map|err :: str|map|str

// division truth table time
//  v1| v2|res ::  v1| v2|res ::  v1| v2|res ::  v1| v2|res
// int|int|int :: uns|int|int :: flt|int|flt :: str|int|err
// int|uns|int :: uns|uns|uns :: flt|uns|flt :: str|uns|err
// int|flt|flt :: uns|flt|flt :: flt|flt|flt :: str|flt|err
// int|str|err :: uns|str|err :: flt|str|err :: str|str|err
// int|seq|err :: uns|seq|err :: flt|seq|err :: str|seq|str
// int|map|err :: uns|map|err :: flt|map|err :: str|map|str

template<typename T>
Composite lisp_binary_plus_impl(T v1, upComposite& v2) {
    return Composite(CompositeDataType::UNKNOWN);
}

template<>
Composite lisp_binary_plus_impl<int64_t>(int64_t v1, upComposite& v2) {
    switch (v2->type()) {
        case CompositeDataType::INT8:
        case CompositeDataType::INT16:
        case CompositeDataType::INT32:
        case CompositeDataType::INT64:
            return v1 + v2->as_int();
        default:
            return 0;
    }
}

Composite lisp_binary_plus(Composite& v1, upComposite& v2) {

    return lisp_binary_plus_impl<int64_t>(v1.as_int(), v2);
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
    Composite result = lisp_binary_plus(*vec[1], vec[2]);
    for (int i=3; i<vec.size(); i++) {
        result = lisp_binary_plus(result, vec[3]);
    }
    return std::unique_ptr<Composite>(new Composite(result));
}


LispFunctionMap lisp_map = {
    {"+", lisp_plus},
};

}
void LispStream::_decode()
{
    size_t index = m_index - m_cursor;
    size_t offset = m_offset - m_cursor;
    CompositeStreamState saved_state = CompositeStreamState::UNKNOWN;
    CompositeStreamState saved_meta_state = CompositeStreamState::UNKNOWN;
    //size_t saved_index = index;
    //size_t saved_offset = offset;

    while (index < m_stream.size()) {
        char c = m_stream.at(index);

        saved_state = m_state;
        saved_meta_state = m_meta_state;
        //std::string str_state = std::string(m_stream.begin(),m_stream.end());
        size_t saved_index = index;
        switch (m_state) {
            case CompositeStreamState::UNKNOWN:
                m_info.offset ++;
                _decode_unknown(c, index, offset);
                break;
            case CompositeStreamState::NUMBER:
                if (_decode_number(c, index, offset)) {
                    index -= 1;
                    _decode_complete(index, offset);
                    offset = index;
                    m_state = CompositeStreamState::UNKNOWN;
                }
                break;
            case CompositeStreamState::STRING:
                if (_decode_string(c, index, offset)) {
                    //index -= 1;
                    _decode_complete(index, offset);
                    offset = index;
                    m_state = CompositeStreamState::UNKNOWN;
                }
                break;
            case CompositeStreamState::TOKEN:
                if (_decode_token(c, index, offset)) {
                    index -= 1;
                    _decode_complete(index, offset);
                    offset = index;
                    m_state = CompositeStreamState::UNKNOWN;
                }
                break;
            default:
                break;
        }

        if (m_diag) {
            if (saved_index == (index+1)) {
                std::cout << "[" << c << "] ";
            } else {
                std::cout << " " << c << "  ";
            }
            std::cout << saved_state << "/"
                      << saved_meta_state << " "
                      << m_state << "/"
                      << m_meta_state << std::endl;

        }

        index++;
    }

    m_index = m_cursor + index;
    m_offset = m_cursor + offset;

    //if (m_diag) {
    //    std::cout << "index: " << saved_index << " -> " << index
    //              << " offset: " << saved_offset << " -> " << offset << std::endl;
    //}

    // erase data which has already been consumed
    if (offset > 0) {
        m_cursor += offset;
        m_stream.erase(m_stream.begin(), m_stream.begin() + offset);
    }
}

void LispStream::_decode_unknown(char c, size_t& index, size_t& offset)
{

    switch (c) {

        //-----------------------------------------------------------------
        case '"':
            offset = index;
            m_state = CompositeStreamState::STRING;
            break;

        //-----------------------------------------------------------------
        case '(':
            offset = index;
            _push_list();
            m_meta_state = CompositeStreamState::SEQVAL;
            break;

        //-----------------------------------------------------------------
        case ')':
            offset = index;
            _pop_list();
            break;

        //-----------------------------------------------------------------
        case '+':
        case '-':
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            offset = index;
            m_state = CompositeStreamState::NUMBER;
            break;

        //-----------------------------------------------------------------
        // skip whitespace

        case '\n':
            m_info.line++;
            m_info.offset=0;
        case '\r':
        case '\t':
        case ' ':
            offset = index;
            // TODO: count newlines for useful error reporting
            // AND count number of characters since the last newline
            break;

        //-----------------------------------------------------------------
        // unkown character is probably an error
        default:
            offset = index;
            m_state = CompositeStreamState::TOKEN;
            break;
    }
}

bool LispStream::_decode_number(char c, size_t& index, size_t& offset)
{
    switch (c) {

        //-----------------------------------------------------------------
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            break;

        //-----------------------------------------------------------------
        // skip whitespace
        case ' ':
        case '\t':
        case '\n':
        case ')':
            return true;

        //-----------------------------------------------------------------
        // unkown character is probably an error
        default:
            // unexpected newline ?
            break;
    }

    return false;
}

void LispStream::_decode_complete(size_t index, size_t offset)
{
    std::string str;
    switch (m_state) {
        case CompositeStreamState::UNKNOWN:
            break;
        case CompositeStreamState::TOKEN:
            str = std::string(m_stream.begin()+offset,m_stream.begin() + index + 1);
            _push_scalar(CompositeDataType::STRING, str);
            break;
        case CompositeStreamState::NUMBER:
            str = std::string(m_stream.begin()+offset,m_stream.begin() + index + 1);
            _push_scalar(CompositeDataType::INT64, str);
            break;
        case CompositeStreamState::STRING:
            str = composite::unquote(std::string(m_stream.begin()+offset,m_stream.begin() + index + 1));
            _push_scalar(CompositeDataType::STRING, str);
            break;
        default:
            break;
    }
    m_seq_pushed = false;
}

void LispStream::_push_list() {

    if (m_root == nullptr) {
        m_root = new Composite(CompositeDataType::SEQ);
        m_stack.push_back(m_root);
    } else {
        // this unique ptr is interesting and needs to be revisited
        // the stack should probably contain pointers to unique ptrs
        CompositeVector& vec = m_stack.back()->as_vector();
        Composite* ptr = new Composite(CompositeDataType::SEQ);
        vec.push_back(std::unique_ptr<Composite>(ptr));
        m_stack.push_back(ptr);
    }
}

void LispStream::_push_scalar(CompositeDataType type, const std::string& str) {

    Composite* ptr;

    if (m_root == nullptr) {
        _push_list();
    }

    if (m_stack.size()==0) {
        SIGPROC_THROW("stack empty");
    }


    CompositeVector& vec = m_stack.back()->as_vector();
    if (type == CompositeDataType::STRING) {
        ptr = new Composite(str);
    } else {
        ptr = composite::parse(str);
    }
    vec.push_back(std::unique_ptr<Composite>(ptr));
}

void LispStream::_pop_list() {

    if (m_stack.size()>0) {
        m_stack.pop_back();
    } else {
        SIGPROC_THROW("stack empty");
    }
}


std::unique_ptr<Composite> LispStream::eval() {

    if (m_root == nullptr) {
        SIGPROC_THROW("error: null root");
    }
    return eval_r(m_root->as_vector());
}

std::unique_ptr<Composite> LispStream::eval_r(CompositeVector& vec) {
    for (int i=0; i<vec.size(); i++) {
        auto& ptr = vec[i];
        // todo I need two different types of SEQ, lisp seq, data seq
        // ... or evaluate on _pop_list and avoid this recursion
        // or re-use UNKNOWN
        if (ptr->type() == CompositeDataType::SEQ) {
            vec[i] = eval_r(ptr->as_vector());
        }
    }
    return eval_f(vec);
}

/**
 * constraint: vec is a sequence of scalars with at least one element
 *             the first element is a string which will be used
 *             as a function label
 *             the function returns a Composite
 *
 */
std::unique_ptr<Composite> LispStream::eval_f(CompositeVector& vec) {
    return lisp_map[vec[0]->as_string()](vec);
}



        } // framework
} // sigproc


