
#ifndef SIGPROC_COMMON_FORMAT_HPP
#define SIGPROC_COMMON_FORMAT_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <type_traits>
/**
 *
 * String Formating
 *
 * printf style formatting with extensions
 *
 * Format Specification:
 * %% -> print a single percent sign
 * %[ALIGN][FILL][WIDTH][.PRECISION][TYPE]
 *
 * ALIGN:
 *      '-'' or '<': align left
 *      '=': align center (using width)
 *      '>': align right (default)
 * FILL:
 *     '0': fill using zeros
 *     ' ': fill using space (default)
 * WIDTH:
 *     integer
 *      specify minimum width of output
 * PRECISION
 *     '.' followed by integer
 *      specify number of decimal places for floats
 * TYPE:
 *      c, C : print character (C: width 2, hex for chars < ' ')
 *      o, O : integer as oct
 *      h, H : integer as hex
 *      d, i : signed
 *      u,   : unsigned
 *      s,   : char* or std::string
 *      t,   : any type implementing friend method:
 *             std::ostream& operator<<(std::ostream& os, const Type& inst)
 *      Most of these types are handled automatically by the stream operator
 *      implementation, with the exception of 'C' for chars.
 */

namespace sigproc {
    namespace common {
        namespace fmt {


constexpr int FMT_PERCENT   = 0x0001;
constexpr int FMT_LEFT      = 0x0002;
constexpr int FMT_CENTER    = 0x0004;
constexpr int FMT_RIGHT     = 0x0008;
constexpr int FMT_CHR       = 0x0010;
constexpr int FMT_INT       = 0x0020;
constexpr int FMT_OCT       = 0x0040;
constexpr int FMT_HEX       = 0x0080;
constexpr int FMT_UPPERCASE = 0x0100;
constexpr int FMT_lOWERCASE = 0x0200;
constexpr int FMT_FLT_EXP   = 0x1000;
constexpr int FMT_UNUSED2   = 0x2000;
constexpr int FMT_UNUSED4   = 0x4000;
constexpr int FMT_POINTER   = 0x8000;

/**
 * parses string format options
 *
 * %% -> {FMT_PERCENT, 0, 0}
 * %[CHAR][-+][NUMBER][CHAR] -> fill, width, type
 */
class Format
{
    int m_flags;
    int m_width;
    int m_precision;
    char m_fill;
public:
    Format(int flags, int64_t width, int64_t precision, char fill)
        : m_flags(flags)
        , m_width(width)
        , m_precision(precision)
        , m_fill(fill)
    {}
    Format()
        : m_flags(0)
        , m_width(0)
        , m_precision(0)
        , m_fill(0)
    {}
    ~Format() {}

    static Format read(const char* str, int* read) {

        int64_t width=0;
        int64_t precision=0;
        char fill = 0;
        char* end;
        int flags = 0;

        *read = 0;

        if (*str == '%') {
            return {FMT_PERCENT, 0, 0, 0};
        }

        // read the alignment character
        switch (*str) {
            case '-':
            case '<':
                flags |= FMT_LEFT;
                *read += 1;
                str += 1;
                break;
            case '=':
                flags |= FMT_CENTER;
                *read += 1;
                str += 1;
                break;
            case '>':
                flags |= FMT_RIGHT;
                *read += 1;
                str += 1;
                break;

            default:
                break;
        }

        /*
            case '+':
                // TODO: force printing numeric sign
            case '#':
                // TODO: preceed with 0, 0x or OX
                *read += 1;
                str += 1;
                break;
        */

        // read the fill character
        switch (*str) {
            case '0':
            case ' ':
                fill = *str;
                *read += 1;
                str += 1;
                break;

            default:
                fill = '\0';
                break;
        }

        // read width from format string
        // width is set to zero if string does not contain an integer
        width = strtoll(str, &end, 10);
        *read += end - str;
        str += end - str;
        // read floating point precision
        if (*str == '.') {
            *read += 1;
            str += 1;
            precision = strtoll(str, &end, 10);
            *read += end - str;
            str += end - str;
        }

        // read the format specifier
        // http://www.cplusplus.com/reference/cstdio/printf/
        switch (*str) {
            case 'x':
                flags |= FMT_HEX|FMT_lOWERCASE;
                break;
            case 'X':
                flags |= FMT_HEX|FMT_UPPERCASE;
                break;
            case 'o':
                flags |= FMT_OCT|FMT_lOWERCASE;
                break;
            case 'O':
                flags |= FMT_OCT|FMT_UPPERCASE;
                break;
            case 'd':
            case 'i':
                flags |= FMT_INT;
                break;
            case 'C':
                flags |= FMT_UPPERCASE;
                // fall thru
            case 'c':
                flags |= FMT_CHR;
                break;
            case 'u': // unsigned integer (default)
                break;
            case 'f': // floating decimal point (default)
            case 'F':
                break;
            case 'e': // floats, not implemented
            case 'E':
                flags |= FMT_FLT_EXP;
                break;
            case 'g':
            case 'G':
                flags |= FMT_FLT_EXP; // TODO: what should this be?
                break;
            case 'p': // pointers, (not implemented)
                flags |= FMT_POINTER;
                break;
            case 's': // strings,
            case 't': // any valid type (default)
                break;
            default:
                break;
        }
        *read += 1;

        return {flags, width, precision, fill};
    }

    int flags() const { return m_flags; }
    int width() const { return m_width; }
    void set_width(int width) { m_width = width; }

    void set(std::ostream& os) const {

        if (m_flags&FMT_LEFT) {
            os << std::left;
        }
        if (m_flags&FMT_RIGHT) {
            os << std::right;
        }
        if (m_flags&FMT_HEX) {
            os << std::hex;
        }
        if (m_flags&FMT_OCT) {
            os << std::oct;
        }
        if (m_flags&FMT_UPPERCASE) {
            os << std::uppercase;
        }
        if (m_flags&FMT_FLT_EXP) {
            os << std::scientific;
        }

        if (m_fill) {
            os << std::setfill(m_fill);
        } else {
            os << std::setfill(' ');
        }

        if (!(m_flags&FMT_CENTER)) {
            os << std::setw(m_width);
        }

        if (m_precision) {
            os << std::fixed
               << std::setprecision(static_cast<int>(m_precision));
        }


    }
};

class FormatSaver
{
    std::ios_base* m_ios;
    std::ios_base::fmtflags m_flags;
public:
    FormatSaver(std::ios_base& ios)
        : m_ios(&ios)
        , m_flags(ios.flags(std::ios_base::fmtflags()))
    {}
    ~FormatSaver()
    {
        m_ios->flags(m_flags);
    }
};


template<typename T>
void tprintf_i(std::ostream& os, const Format& fmt, T value) {
    // todo: hide this behind debug builds...
    if (fmt.flags()&FMT_POINTER && !std::is_pointer<T>::value) {
        std::cerr << "format specifier expected a pointer"<< std::endl;
    }
    os << value;
}

template<>
void tprintf_i<char>(std::ostream& os, const Format& fmt, char value);

template<typename T>
void tprintf(std::ostream& os, const Format& fmt, const T value)
{
    FormatSaver saver(os);
    if(fmt.flags()&FMT_CENTER) {
        std::stringstream ss;
        fmt.set(ss);
        tprintf_i<T>(ss, fmt, value);
        std::string s = ss.str();
        int w1 = fmt.width() - static_cast<int>(s.size());
        int w2 = (w1 + 1) / 2;
        if (w1 > 0) {
            w1 = w1/2;
            os << std::string(w1, ' ') << s << std::string(w2, ' ');
        } else {
            os << s;
        }
    } else {
        fmt.set(os);
        tprintf_i<T>(os, fmt, value);
    }

}

void osprintf(std::ostream& os, size_t index, const std::string& fmtstr);

template<typename T, typename... U>
void osprintf(std::ostream& os, size_t index, const std::string& fmtstr, const T head, const U... rest)
{
    Format fmt;
    int read;
    size_t offset = index;

    bool consumes = false;
    for (; index < fmtstr.size(); index++) {

        char c = fmtstr[index];

        switch (c) {
            case '%':
                os << fmtstr.substr(offset, index-offset);
                fmt = Format::read(fmtstr.c_str() + index + 1, &read);
                index += 1 + read;
                offset = index;
                if (!(fmt.flags()&FMT_PERCENT)){
                    tprintf(os, fmt, head);
                    consumes = true;
                }
                break;
            default:
                break;
        }

        if (consumes) {
            break;
        }
    }

    if (!consumes) {
        // this is a user error
        // more arguments were provided than the format string specified.
        osprintf(os, offset, fmtstr);
    } else {
        osprintf(os, index, fmtstr, rest...);
    }
}

std::string sprintf(const std::string& fmtstr);

template<typename T, typename... U>
std::string sprintf(const std::string& fmtstr, const T head, const U... rest)
{
    std::stringstream ss;
    osprintf(ss, 0, fmtstr, head, rest...);
    return ss.str();
}

/*
template<typename... U>
class PrintFormat
{
    std::string m_fmtstr;
    std::tuple<U...> m_args;

public:
    PrintFormat(const std::string& fmtstr, U&&... args)
        : m_fmtstr(fmtstr)
        , m_args(std::forward<U>(args)...)
    {}
    ~PrintFormat() {}

    template<typename... Uf>
    friend std::ostream& operator << (std::ostream& os, PrintFormat<Uf...>& obj);

};

template<typename... U>
PrintFormat<U...> printf(const std::string& fmtstr, const U... args) {
    return PrintFormat<U...>(fmtstr, args...);
}
*/



        } // fmt
    } // common
} // sigproc

/*template<typename... U>
std::ostream& operator << (std::ostream& os, const sigproc::common::fmt::PrintFormat<U...>& obj)
{
    // TODO:
    os << "todo";
    return os;
}*/

#endif