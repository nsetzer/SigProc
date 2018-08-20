
#ifndef SIGPROC_COMMON_FORMAT_HPP
#define SIGPROC_COMMON_FORMAT_HPP

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace sigproc {
    namespace common {
        namespace fmt {

/**
 *
 *
 */
constexpr int FMT_PERCENT   = 0x001;
constexpr int FMT_LEFT      = 0x002;
constexpr int FMT_CENTER    = 0x004;
constexpr int FMT_RIGHT     = 0x008;
constexpr int FMT_CHR       = 0x010;
constexpr int FMT_INT       = 0x020;
constexpr int FMT_OCT       = 0x040;
constexpr int FMT_HEX       = 0x080;
constexpr int FMT_UPPERCASE = 0x100;
constexpr int FMT_lOWERCASE = 0x200;

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
    char m_fill;
public:
    Format(int flags, int64_t width, char fill)
        : m_flags(flags)
        , m_width(width)
        , m_fill(fill)
    {}
    Format(int flags, int64_t width)
        : m_flags(flags)
        , m_width(width)
        , m_fill(0)
    {}
    Format()
        : m_flags(0)
        , m_width(0)
        , m_fill(0)
    {}
    ~Format() {}

    /**
     * A format string has the form:
     *   %[ALIGN][WIDTH][TYPE]
     *
     * ALIGN: optional; any character in the set:
     *      -: left align
     *      <: left align
     *      =: center align (not yet implemented)
     *      >: right align
     * WIDTH: optiona; an integer
     * TYPE: required: any character in the set:
     *      o: oct
     *      O: OCT
     *      x: hex
     *      X: HEX
     *      d: decimal
     *      c: char
     *      C: char, (HEX for c < ' ', width is always 2)
     */
    static Format read(const char* str, int* read) {

        int64_t width;
        char fill = 0;
        char* end;
        int flags = 0;

        *read = 0;

        if (*str == '%') {
            return {FMT_PERCENT, 0, 0};
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
                break;
        }

        // read width from format string
        // width is set to zero if string does not contain an integer
        width = strtoll(str, &end, 10);
        *read += end - str;
        str += end - str;

        //TODO:
        // precision, if the next character is a '.'
        // skip the character and read another integer
        // std::cout << std::setprecision(n)
        // std::cout << std::scientific
        // std::cout << std::fixed

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
            case 'c':
                flags |= FMT_CHR;
                break;
            case 'C':
                flags |= FMT_CHR|FMT_UPPERCASE;
                break;
            case 'u': // unsigned integer (default)
            case 'f': // floating decimal point (default)
            case 'F':
            case 'e': // floats, not implemented
            case 'E':
            case 'g':
            case 'G':
                break;
            case 'p': // pointers, (not implemented)
            case 's': // strings,
            case 't': // any valid type (default)
                break;
            default:
                break;
        }
        *read += 1;

        return {flags, width, fill};
    }

    int flags() const { return m_flags; }
    int width() const { return m_width; }

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
        if (m_fill) {

            os << std::setfill(m_fill);
        } else {
            os << std::setfill(' ');
        }

        os << std::setw(m_width);
    }
};

class FormatSaver
{
    std::ios_base* m_ios;
    std::ios_base::fmtflags m_flags;
public:
    FormatSaver(std::ios_base& ios)
        : m_ios(&ios)
        , m_flags(ios.flags())
    {}
    ~FormatSaver()
    {
        m_ios->flags(m_flags);
    }
};


template<typename T>
void tprintf_i(std::ostream& os, const Format& fmt, T value) {
    os << value;
}

template<>
void tprintf_i<char>(std::ostream& os, const Format& fmt, char value);

template<typename T>
void tprintf(std::ostream& os, const Format& fmt, const T value)
{
    FormatSaver saver(os);
    fmt.set(os);
    tprintf_i<T>(os, fmt, value);
}

void osprintf(std::ostream& os, size_t index, const std::string& fmtstr);

template<typename T, typename... U>
void osprintf(std::ostream& os, size_t index, const std::string& fmtstr, const T head, const U... rest)
{
    Format fmt;
    int read;
    size_t offset = index;
    // TODO: this implementation is not finished
    // check for cases where the fmt runs out before
    // all values are consumed, and warn
    for (; index < fmtstr.size(); index++) {

        char c = fmtstr[index];
        bool consumes = false;

        switch (c) {
            case '%':
                os << fmtstr.substr(offset, index-offset);
                fmt = Format::read(fmtstr.c_str() + index + 1, &read);
                if (!(fmt.flags()&FMT_PERCENT)){
                    tprintf(os, fmt, head);
                    index += 1 + read;
                    offset = index;
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

    osprintf(os, index, fmtstr, rest...);
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