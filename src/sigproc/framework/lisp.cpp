
#include <iterator>
#include <algorithm>

#include "sigproc/framework/lisp.hpp"

namespace sigproc {
    namespace framework {

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

/**

eval is basically:
    call eval on the root list
    iterate through the list, for any list in the list, call eval
    eval will produce a result, delete the list and substitute the value
    finally call eval_main on the list

*/


        } // framework
} // sigproc


