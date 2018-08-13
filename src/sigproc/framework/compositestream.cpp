

#include <iterator>
#include <algorithm>

#include "sigproc/framework/compositestream.hpp"
#include "sigproc/framework/enum.hpp"
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace framework {

#define COMPOSITE_EXCEPTION(x) do { \
    std::stringstream _ex_ss; \
    _ex_ss << x; \
    throw CompositeStreamException(_ex_ss.str()); \
} while(0)

namespace {

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

} /* anonymous */

CompositeStream::~CompositeStream()
{
    if (m_root != nullptr) {
        delete m_root;
    }

    if (m_mapkey != nullptr) {
        delete m_mapkey;
    }
}

void CompositeStream::push(const std::string& bytes)
{
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(m_stream));
    _decode();
}

void CompositeStream::push(const char* bytes, size_t length)
{
    std::copy(bytes, bytes + length, std::back_inserter(m_stream));
    _decode();
}

CompositeStream& CompositeStream::operator<<(const std::string& bytes)
{
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(m_stream));
    _decode();
    return *this;
}


void CompositeStream::close()
{
    // TODO: check for unterminated string state

    m_closed = true;

    size_t index = m_index - m_cursor;
    size_t offset = m_offset - m_cursor;

    /*std::cout
        << " cursor: " << m_cursor
        << " offset: " << m_offset << "(" << offset << ")"
        << " index: " << m_index << "(" << index << ")"
        << " size: " << m_stream.size()
        << std::endl;*/\
    if (m_state == CompositeStreamState::STRING) {
        throw CompositeStreamException(m_info, "unterminated string");
    }

    if (m_stack_info.size() > 0) {
        throw CompositeStreamException(m_stack_info.back(), "unterminated collection");
    }

    _decode_complete(index-1, offset);

    /*std::cout
        << " cursor: " << m_cursor
        << " offset: " << m_offset << "(" << offset << ")"
        << " index: " << m_index << "(" << index << ")"
        << " size: " << m_stream.size()
        << std::endl;*/

}

Composite* CompositeStream::root()
{
    return m_root;
}

Composite* CompositeStream::release()
{
    if (!m_closed) {
        close();
    }

    Composite* ptr = m_root;
    m_root = nullptr;
    return ptr;
}

void CompositeStream::_decode()
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
        saved_meta_state = saved_meta_state;
        std::string str_state = std::string(m_stream.begin(),m_stream.end());
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
            /*std::cout << saved_state << "/"
                      << saved_meta_state << " "
                      << m_state << "/"
                      << m_meta_state << " " << str_state << std::endl;
			*/
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

void CompositeStream::_decode_unknown(char c, size_t& index, size_t& offset)
{

    switch (c) {

        //-----------------------------------------------------------------
        case '"':
            offset = index;
            m_state = CompositeStreamState::STRING;
            break;

        //-----------------------------------------------------------------
        case '[':
            offset = index;
            _push_collection(CompositeDataType::SEQ);
            m_meta_state = CompositeStreamState::SEQVAL;
            break;

        //-----------------------------------------------------------------
        case ']':
            offset = index;
            _pop_collection();

            break;

        //-----------------------------------------------------------------
        case '{':
            offset = index;
            _push_collection(CompositeDataType::MAP);
            m_meta_state = CompositeStreamState::MAPKEY;
            break;

        //-----------------------------------------------------------------
        case '}':
            offset = index;
            _pop_collection();
            break;

        //-----------------------------------------------------------------
        case ',':
            offset = index;
            if (m_seq_pushed) {
                COMPOSITE_EXCEPTION("empty seq");
            }
            if (m_meta_state == CompositeStreamState::SEQVAL ) {
                m_seq_pushed = true;
                break;
            } else if (m_meta_state == CompositeStreamState::MAPVAL ) {
                m_seq_pushed = true;
                m_meta_state = CompositeStreamState::MAPKEY;
                break;
            }
            // error
            break;

        case ':':
            offset = index;
            if (m_meta_state == CompositeStreamState::MAPKEY ) {
                m_meta_state = CompositeStreamState::MAPVAL;
                break;
            }
            // error
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

bool CompositeStream::_decode_number(char c, size_t& index, size_t& offset)
{
    switch (c) {

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
            break;

        //-----------------------------------------------------------------
        // skip whitespace
        case ' ':
        case '\t':
        case '\n':
            return true;

        case ',':
            if (m_meta_state == CompositeStreamState::MAPVAL) {
                return true;
            }
            // fall through
        case ']':
            if (m_meta_state == CompositeStreamState::SEQVAL) {
                return true;
            }

            // error?
            break;
        case '}':
            if (m_meta_state == CompositeStreamState::MAPKEY ||
                m_meta_state == CompositeStreamState::MAPVAL) {
                return true;
            }

            // error?
            break;

        //-----------------------------------------------------------------
        // unkown character is probably an error
        default:
            // unexpected newline ?
            break;
    }

    return false;
}

bool CompositeStream::_decode_string(char c, size_t& index, size_t& offset)
{
    switch (c) {

        //-----------------------------------------------------------------
        case '\\':
            index += 1;
            break;

        case '"':
            return true;
    }

    return false;
}

bool CompositeStream::_decode_token(char c, size_t& index, size_t& offset)
{
    switch (c) {

        //-----------------------------------------------------------------
        case ',':
        case ':':
        case '[':
        case ']':
        case '{':
        case '}':
        case '"':
            return true;

        case ' ':
        case '\t':
        case '\n':
            return true;
    }

    return false;
}

void CompositeStream::_decode_complete(size_t index, size_t offset)
{
    std::string str;
    switch (m_state) {
        case CompositeStreamState::UNKNOWN:
            break;
        case CompositeStreamState::TOKEN:
            str = std::string(m_stream.begin()+offset,m_stream.begin() + index + 1);
            _push_token(str);
            break;
        case CompositeStreamState::NUMBER:
            str = std::string(m_stream.begin()+offset,m_stream.begin() + index + 1);
            _push_scalar(CompositeDataType::INT64, str);
            break;
        case CompositeStreamState::STRING:
            str = unquote(std::string(m_stream.begin()+offset,m_stream.begin() + index + 1));
            _push_scalar(CompositeDataType::STRING, str);
            break;
        default:
            break;
    }
    m_seq_pushed = false;
}

void CompositeStream::_push_scalar(CompositeDataType type, const std::string& str)
{
    // TODO: only parse if the state is a number...

    //std::cout << m_stack.size() << "/(" << static_cast<int>(m_meta_state) << ") " << type <<  "`" << str << "`" << std::endl;

    Composite* ptr = nullptr;
    Composite* current = nullptr;
    char** key = nullptr;
    CompositeVector* vec = nullptr;
    CompositeMap* map = nullptr;

    if (m_root == nullptr) {
        m_root = parse(str);
    } else {
        switch (m_meta_state) {
            case CompositeStreamState::MAPKEY:

                if (type == CompositeDataType::INT64) {
                    COMPOSITE_EXCEPTION("map keys must be strings" << Val(type) << Val(str));
                }
                m_mapkey = new Composite(str);
                break;
            case CompositeStreamState::MAPVAL:
                current = m_stack.back();
                // ---------------------------------------------------
                // todo: these can throw and bad things will happen
                current->value<CompositeMap>(&map);
                m_mapkey->value<char*>(&key);
                // ---------------------------------------------------
                //std::cout << "{} " << *key << ":" << str << std::endl;
                if (type != CompositeDataType::STRING) {
                    ptr = parse(str);
                } else {
                    ptr = new Composite(str);
                }
                (*map)[*key] = std::unique_ptr<Composite>(ptr);
                delete m_mapkey;
                m_mapkey = nullptr;
                break;
            case CompositeStreamState::SEQVAL:
                current = m_stack.back();
                // ---------------------------------------------------
                // todo: these can throw and bad things will happen
                current->value<CompositeVector>(&vec);
                // ---------------------------------------------------
                //std::cout << "[] " << str << std::endl;
                if (type != CompositeDataType::STRING) {
                    ptr = parse(str);
                } else {
                    ptr = new Composite(str);
                }
                vec->push_back(std::unique_ptr<Composite>(ptr));
                break;
            default:
                break;
        }
    }

}

void CompositeStream::_push_token(const std::string& str)
{
    if (str == "null") {
        _push_scalar(CompositeDataType::STRING, "");
    } else if (str == "true") {
        _push_scalar(CompositeDataType::INT64, "1");
    } else if (str == "false") {
        _push_scalar(CompositeDataType::INT64, "0");
    } else {
        _push_scalar(CompositeDataType::STRING, str);
    }
}

void CompositeStream::_push_collection(CompositeDataType collection)
{
    // validate the top of the stack is either seq or map or null
    // if seq, push onto that sequence, and the stack
    // if map, push onto that map, and the stack
    Composite* current = nullptr;
    char** key = nullptr;
    CompositeVector* vec = nullptr;
    CompositeMap* map = nullptr;

    //std::cout << "push " << collection << std::endl;

    if (m_root != nullptr && (m_root->type() != CompositeDataType::SEQ &&
                              m_root->type() != CompositeDataType::MAP)) {
        m_closed = true;
        COMPOSITE_EXCEPTION("something about multi roots");
    }

    if (m_stack.size() > 0) {
        current = m_stack.back();
    }

    m_stack.push_back(new Composite(collection));
    m_stack_info.push_back(m_info);

    if (m_root == nullptr) {
        //std::cout << "push " << collection << " to root " << std::endl;
        m_root = m_stack.back();
    } else if (current != nullptr) {

        //std::cout << "push update " << current->type() <<std::endl;
        switch (current->type())  {
            case CompositeDataType::SEQ:
                // ---------------------------------------------------
                // todo: these can throw and bad things will happen
                try {
                    current->value<CompositeVector>(&vec);
                } catch (sigproc::common::exception::SigprocException& e) {
					(void)e;
                    m_closed = true;
                    COMPOSITE_EXCEPTION("illegal vec operation");
                }
                // ---------------------------------------------------
                //std::cout << "[] []" << std::endl;
                vec->push_back(std::unique_ptr<Composite>(m_stack.back()));
                break;
            case CompositeDataType::MAP:
                // ---------------------------------------------------
                // todo: these can throw and bad things will happen
                try {
                    current->value<CompositeMap>(&map);
                } catch (sigproc::common::exception::SigprocException& e) {
					(void)e;
                    m_closed = true;
                    COMPOSITE_EXCEPTION("illegal map operation");
                }
                try {
                    m_mapkey->value<char*>(&key);
                } catch (sigproc::common::exception::SigprocException& e) {
					(void)e;
                    m_closed = true;
                    COMPOSITE_EXCEPTION("illegal map key operation");
                }
                // ---------------------------------------------------
                //std::cout << "{} " << *key << ": []" << std::endl;
                (*map)[*key] = std::unique_ptr<Composite>(m_stack.back());
                delete m_mapkey;
                m_mapkey = nullptr;
                break;
            default:
                //std::cout << "push " << collection << " to ERROR " << std::endl;
                break;
        }
    } else {
        m_closed = true;
        COMPOSITE_EXCEPTION("something is wrong with the stack");
    }

}


void CompositeStream::_pop_collection()
{
    // validate that the top of the stack is a seq
    // pop that element from the stack

    m_stack.pop_back();
    m_stack_info.pop_back();

    if (m_stack.size() > 0) {
        Composite* current = m_stack.back();
        switch (current->type())  {
            case CompositeDataType::SEQ:
                //std::cout << "pop ??? -> SEQ" << std::endl;
                m_meta_state = CompositeStreamState::SEQVAL;
                break;
            case CompositeDataType::MAP:
                // UNSURE but looks right
                //std::cout << "pop ??? -> MAP" << std::endl;
                m_meta_state = CompositeStreamState::MAPKEY;
                break;
            default:
                m_meta_state = CompositeStreamState::UNKNOWN;
                break;
        }
    } else {
        m_meta_state = CompositeStreamState::UNKNOWN;
    }
}

        } // framework
} // sigproc


