

#include "sigproc/framework/composite.hpp"
#include "sigproc/framework/compositestream.hpp"

#include <iostream>
#include <fstream>

using namespace sigproc::framework;

int main(int argc, char* argv[]) {

    char buffer[1024];
    std::istream* fin = &std::cin;
    std::ostream* fout = &std::cout;

    if (argc > 1 && std::string(argv[1]) != "-") {
        fin = new std::ifstream(argv[1]);
    }

    if (argc > 2 && std::string(argv[2]) != "-") {
        fout = new std::ofstream(argv[2]);
    }

    try {
        CompositeStream stream;

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
            (*fout) << *stream.root() << std::endl;
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