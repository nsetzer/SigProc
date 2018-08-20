
#include "sigproc/common/string.hpp"

namespace sigproc {
    namespace common {

std::string replace(const std::string& str, const std::string& find, const std::string& replace) {
    std::string out(str);
    size_t start = 0;
    while ((start=out.find(find, start)) != std::string::npos) {
        out.replace(start, find.size(), replace);
        start += replace.size();
    }
    return out;
}

    } // common
} // sigproc