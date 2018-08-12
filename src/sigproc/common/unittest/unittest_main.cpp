


#include "sigproc/common/unittest/unittest.hpp"

using namespace sigproc::common::unittest;

int main(int argc, char* argv[]) {

    std::cout << "running tests" << std::endl;
    int retval = UnitTestRegistry::run_tests();

    if (UnitTestRegistry::registry != nullptr) {
        delete UnitTestRegistry::registry;
    }

    return retval;
}