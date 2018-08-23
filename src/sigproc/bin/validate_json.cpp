

#include "sigproc/common/argparse.hpp"
#include "sigproc/framework/composite.hpp"
#include "sigproc/framework/compositestream.hpp"

#include <iostream>
#include <fstream>

using namespace sigproc::common;
using namespace sigproc::framework;

ArgParseSpec spec = {
    {"validate_json"},
    {"verbose", 'v', 0, "enable pretty printing"},
    {"diag", 'd', 0, "enable diagnostics"},
    {"in_file", true, "json input (- for stdin)"},
    {"out_file", true, "json output (- for stdout)"},
};

int main(int argc, char* argv[]) {

    char buffer[1024];
    std::istream* fin = &std::cin;
    std::ostream* fout = &std::cout;

    ArgParser parser(argc, argv, spec);

    bool diag = false;
    bool pretty = false;

    std::vector<std::string> args;
    for (int i=0; i<argc; i++) {
        args.push_back(argv[i]);
    }

    while (args.size() > 1 && args[1].size() == 2) {
        // enable json pretty printing
        if (args[1] == "-v") {
            pretty = true;
            args.erase(args.begin()+1);
        // enable decoder diagnostics
        } else if (args[1] == "-d") {
            diag = true;
            args.erase(args.begin()+1);
        } else {
            break;
        }
    }

    if (args.size() > 1 && args[1] != "-") {
        fin = new std::ifstream(args[1].c_str());
    }

    if (args.size() > 2 && args[2] != "-") {
        fout = new std::ofstream(args[2].c_str());
    }

    try {
        CompositeStream stream(diag);

        // read from the input file and push the bytes into the
        // composite stream, building a composite object
        while (fin->good()) {
            fin->read(buffer, sizeof(buffer));
            stream.push(buffer, fin->gcount());
        }

        stream.close();

        // if no exceptions were thrown, serialize the composite
        // object back to json
        if (stream.root() != nullptr) {
            //(*fout) << *stream.root() << std::endl;
            stream.root()->print(*fout, 0, 2, pretty);
            *fout << std::endl;
        } else {
            std::cerr << "no root object decoded" << std::endl;
        }

    } catch (CompositeStreamException& e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    if (fin != &std::cin) {
        static_cast<std::ifstream*>(fin)->close();
        delete fin;
    }

    if (fout != &std::cout) {
        static_cast<std::ofstream*>(fout)->close();
        delete fout;
    }
}