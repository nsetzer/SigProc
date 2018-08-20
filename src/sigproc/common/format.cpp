

#include "sigproc/common/format.hpp"


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
    os << fmtstr.substr(index, fmtstr.size()-index);
}

std::string sprintf(const std::string& fmtstr)
{
    return fmtstr;
}



        } // fmt
    } // common
} // sigproc
