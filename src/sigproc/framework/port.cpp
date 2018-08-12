

#include "sigproc/framework/node.hpp"
#include "sigproc/framework/processor.hpp"
#include "sigproc/framework/stream.hpp"


namespace sigproc {
    namespace framework {

size_t PortBase::calculate_depth() {
    if (m_depth==0) {
        if (m_processor != nullptr) {
            m_depth = m_processor->calculate_depth();
        }
        m_depth += 1;
    }
    return m_depth;
}

    } // framework
} // sigproc