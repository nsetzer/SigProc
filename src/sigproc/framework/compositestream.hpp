
#ifndef SIGPROC_FRAMEWORK_COMPOSITESTREAM_HPP
#define SIGPROC_FRAMEWORK_COMPOSITESTREAM_HPP

#include <deque>
#include <vector>

#include "sigproc/framework/composite.hpp"
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace framework {

namespace composite {
    class LineInfo
    {
    public:
        size_t line;
        size_t offset;

        LineInfo(size_t line, size_t offset)
            : line(line)
            , offset(offset)
        {}
        ~LineInfo() {}
    };
}

class CompositeStreamException : public sigproc::common::exception::SigprocException
{
    size_t m_line;
    size_t m_offset;
    std::string m_msg;
public:
    CompositeStreamException( size_t line, size_t offset, const std::string& msg )
        : sigproc::common::exception::SigprocException( msg )
        , m_line(line)
        , m_offset(offset)
    {
        _init();
    }
    CompositeStreamException( composite::LineInfo info, const std::string& msg )
        : sigproc::common::exception::SigprocException( msg )
        , m_line(info.line)
        , m_offset(info.offset)
    {
        _init();
    }
    CompositeStreamException( const std::string& msg )
        : sigproc::common::exception::SigprocException( msg )
        , m_line(0)
        , m_offset(0)
    {
        _init();
    }
    CompositeStreamException( const char* msg )
        : sigproc::common::exception::SigprocException( msg )
        , m_line(0)
        , m_offset(0)
    {
        _init();
    }
    ~CompositeStreamException(){}


    size_t line() { return m_line; }
    size_t offset() { return m_offset; }

    const char* what() const noexcept {
        return m_msg.c_str();
    }

private:
    void _init() {
        std::stringstream ss;
        ss << "Exception as Line " << m_line
           << " Column " << m_offset << ": "
           << sigproc::common::exception::SigprocException::what();
        m_msg = ss.str();
    }
};



class CompositeStream
{
    size_t m_index;
    size_t m_offset;
    size_t m_cursor;
    std::deque<char> m_stream;
    std::vector<Composite*> m_stack;
    std::vector<composite::LineInfo> m_stack_info;
    Composite* m_root;
    Composite* m_mapkey;
    CompositeStreamState m_state;
    CompositeStreamState m_meta_state;
    composite::LineInfo m_info;

    // TODO: rename this
    // when decoding a collection, if a ',' is found, set m_seq_pushed to true
    // whenever a scalar is decoded, set m_seq_pushed to false
    // when decoding a collection, if a ',' is found and m_seq_pushed is true
    // raise an error to indicate an empty sequence was found.
    //   e.g. [ 1, , 3] or {a:0, ,}
    bool m_seq_pushed;

    // when closed, no more bytes can be pushed to the stream;
    bool m_closed;

    // when set true, print out state information about each character decoded
    bool m_diag;

public:
    CompositeStream()
        : m_index(0)
        , m_offset(0)
        , m_cursor(0)
        , m_stream()
        , m_stack()
        , m_stack_info()
        , m_root(nullptr)
        , m_mapkey(nullptr)
        , m_state(CompositeStreamState::UNKNOWN)
        , m_meta_state(CompositeStreamState::UNKNOWN)
        , m_info(0,0)
        , m_seq_pushed(false)
        , m_closed(false)
        , m_diag(false)
    {}
    CompositeStream(bool diag)
        : m_index(0)
        , m_offset(0)
        , m_cursor(0)
        , m_stream()
        , m_stack()
        , m_stack_info()
        , m_root(nullptr)
        , m_mapkey(nullptr)
        , m_state(CompositeStreamState::UNKNOWN)
        , m_meta_state(CompositeStreamState::UNKNOWN)
        , m_info(0,0)
        , m_seq_pushed(false)
        , m_closed(false)
        , m_diag(diag)
    {}
    ~CompositeStream();

    void push(const std::string& bytes);
    void push(const char* bytes, size_t length);
    CompositeStream& operator<<(const std::string& bytes);

    void close();

    // access the document root
    Composite* root();

    // close the stream and transfer ownership of the pointer to the caller
    Composite* release();
private:

    void _decode();
    void _decode_unknown(char c, size_t& index, size_t& offset);
    bool _decode_number(char c, size_t& index, size_t& offset);
    bool _decode_string(char c, size_t& index, size_t& offset);
    bool _decode_token(char c, size_t& index, size_t& offset);
    void _decode_complete(size_t index, size_t offset);

    void _push_scalar(CompositeDataType type, const std::string& str);
    void _push_token(const std::string& str);
    void _push_collection(CompositeDataType collection);
    void _pop_collection();
};

    } // framework
} // sigproc



#endif
