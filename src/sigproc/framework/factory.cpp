

#include "sigproc/framework/factory.hpp"
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace framework {

Processor* ProcessorFactory::create(const std::string& procName)
{
    if (m_pRegistry->count(procName)==0) {
        SIGPROC_THROW( "no node named: " << Val(procName) );
    }
    Processor* proc = (*m_pRegistry)[procName]();
    proc->set_name(procName);
    return proc;
}


    } // framework
} // sigproc