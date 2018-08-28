

#include "sigproc/common/string.hpp"
#include "sigproc/common/format.hpp"

#include <cstring>

namespace sigproc {
    namespace common {
        namespace fmt {

template<>
void tprintf_i<char>(std::ostream& os, const Format& fmt, char value)
{
    if (fmt.flags()&FMT_UPPERCASE) {
        os  << std::setw(2);
        if (value < ' ')
            os << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(value);
        else
            os << std::setfill(' ') << value;
    } else {
        os << value;
    }
}

void osprintf(std::ostream& os, size_t index, const std::string& fmtstr) {
    os << replace(fmtstr.substr(index, fmtstr.size()-index), "%%", "%");
}

std::string sprintf(const std::string& fmtstr)
{
    return replace(fmtstr, "%%", "%");
}

void printf(const std::string& fmtstr)
{
    osprintf(std::cout, 0, fmtstr);
}

template<>
size_t osprintb<char>(std::ostream& os, const char* value)
{
    size_t len = strlen(value) + 1;
    os.write(value, len);
    return len;
}

size_t osprintb(std::ostream& os, const std::string& value)
{
    os.write(value.c_str(), value.size() + 1);
    return value.size() + 1;
}


        } // fmt
    } // common
} // sigproc
