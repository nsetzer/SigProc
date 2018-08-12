

#include "sigproc/common/unittest/unittest.hpp"
#include <iostream>
namespace sigproc {
    namespace common {
        namespace unittest {

namespace {

class CaptureStream {

    std::ios* m_stream;
    std::streambuf * m_old;

public:
    CaptureStream( std::ios& buf, std::stringstream& new_buffer )
        : m_stream(&buf)
        , m_old( buf.rdbuf( new_buffer.rdbuf() ) )
    {}

    CaptureStream( std::ios& buf, std::streambuf * new_buffer )
        : m_stream(&buf)
        , m_old( buf.rdbuf( new_buffer ) )
    {}

    ~CaptureStream( ) {
        m_stream->rdbuf( m_old );
    }
};

} // anonymous

std::vector<UnitTest*>* UnitTestRegistry::registry = nullptr;

void UnitTestRegistry::reg(UnitTest* test)
{
    if (UnitTestRegistry::registry == nullptr) {
        UnitTestRegistry::registry = new std::vector<UnitTest*>();
    }
    UnitTestRegistry::registry->push_back(test);
}

int UnitTestRegistry::run_tests()
{
    if (UnitTestRegistry::registry == nullptr || UnitTestRegistry::registry->size()==0) {
        std::cerr << "no tests registered" << std::endl;
        return 1;
    }

    for (UnitTest* test : *UnitTestRegistry::registry) {

        std::cout << test->name() << " ...";

        std::stringstream ss;

        try {

            {
                // todo: unbuffered flag to disable capture
                // todo: quiet flag to disable printing captured output
                CaptureStream capture(std::cout, ss);
                test->run();
            }
            std::cout << "\r [OK ] " << test->name() << std::endl;
            if (!ss.str().empty()) {
                std::cout << ss.str() << std::endl;
            }

        } catch (std::exception& e) {
            std::cout << "\r [ERR] " << test->name() << std::endl;
            std::cout << e.what() << std::endl;
            if (!ss.str().empty()) {
                std::cout << ss.str() << std::endl;
            }
        } catch (...) {
            std::cout << "\r [ERR] " << test->name() << std::endl;
            if (!ss.str().empty()) {
                std::cout << ss.str() << std::endl;
            }
        }
    }

    return 0;
}


        } // unittest
    } // common
} // sigproc
