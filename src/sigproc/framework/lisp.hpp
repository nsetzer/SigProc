
#ifndef SIGPROC_FRAMEWORK_LISP_HPP
#define SIGPROC_FRAMEWORK_LISP_HPP


#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"
#include "sigproc/framework/compositestream.hpp"

namespace sigproc {
    namespace framework {


class LispStream : public CompositeStream {


public:
    LispStream()
        : CompositeStream()
    {}

    LispStream(bool diag)
        : CompositeStream(diag)
    {}

    ~LispStream() {}

    virtual void _decode();
    virtual void _decode_unknown(char c, size_t& index, size_t& offset);
    virtual bool _decode_number(char c, size_t& index, size_t& offset);
    virtual void _decode_complete(size_t index, size_t offset);

    void _push_list();
    void _push_scalar(CompositeDataType type, const std::string& str);
    void _pop_list();

};

/*

 (token number string float)

*/


    } // framework
} // sigproc



#endif
