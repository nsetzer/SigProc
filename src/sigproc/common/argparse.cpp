

#include "sigproc/common/argparse.hpp"


namespace sigproc {
    namespace common {

std::string ArgParseItem::description() const {
    (void) m_type;
    (void) m_shortname;
    (void) m_consumes;

    return m_description;
}

void ArgParser::parse(const std::vector<std::string>& args, const ArgParseSpec& spec) {



    for (const std::string& str: args) {
        std::cout << str << std::endl;
    }

    for (const ArgParseItem& item: spec) {
        std::cout << item.description() << std::endl;
    }
}


    } // common
} // sigproc

