#ifndef SIGPROC_COMMON_ARGPARSE_HPP
#define SIGPROC_COMMON_ARGPARSE_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace sigproc {
    namespace common {

enum class ArgParseItemType
{
    UNKOWN     = 0,
    POSITIONAL = 1, // a positional argument
    SWITCH     = 2, // a optional switch, e.g. -v, --verbose
    PARAGRAPH  = 3,
};

class ArgParseItem
{
    ArgParseItemType m_type;
    std::string m_name;
    char m_shortname;
    std::string m_description;
    int m_consumes;

public:
    ArgParseItem(const std::string& description)
        : m_type(ArgParseItemType::PARAGRAPH)
        , m_name()
        , m_shortname()
        , m_description(description)
        , m_consumes(0)
    {}
    ArgParseItem(const std::string& name, const std::string& description)
        : m_type(ArgParseItemType::POSITIONAL)
        , m_name(name)
        , m_shortname()
        , m_description(description)
        , m_consumes(1)
    {}
    ArgParseItem(const std::string& name, bool optional, const std::string& description)
        : m_type(ArgParseItemType::POSITIONAL)
        , m_name(name)
        , m_shortname()
        , m_description(description)
        , m_consumes(optional?0:1)
    {}
    ArgParseItem(const std::string& name, int consumes, const std::string& description)
        : m_type(ArgParseItemType::SWITCH)
        , m_name(name)
        , m_shortname()
        , m_description(description)
        , m_consumes(consumes)
    {}
    ArgParseItem(const std::string& name, char shortname, int consumes, const std::string& description)
        : m_type(ArgParseItemType::SWITCH)
        , m_name(name)
        , m_shortname(shortname)
        , m_description(description)
        , m_consumes(consumes)
    {}
    ~ArgParseItem() {}

    std::string description() const;

};

typedef std::vector<ArgParseItem> ArgParseSpec;

class ArgParser
{
public:
    ArgParser(int argc, char* argv[], const ArgParseSpec& spec) {
        std::vector<std::string> args;
        for (int i=0; i<argc; i++) {
            args.push_back(argv[i]);
        }
        parse(args, spec);
    }
    ArgParser(const std::vector<std::string>& args, const ArgParseSpec& spec);
    ~ArgParser() {}

private:
    void parse(const std::vector<std::string>& args, const ArgParseSpec& spec);



};


    } // common
} // sigproc


#endif