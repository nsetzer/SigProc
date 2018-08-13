
#include "sigproc/framework/graphdef.hpp"
#include "sigproc/framework/graph.hpp"
#include "sigproc/common/exception.hpp"

namespace sigproc {
    namespace framework {

Graph* GraphDef::create(std::vector<std::unique_ptr<Composite>> nodes)
{
    _validate();
    _create();
    _connect();
    _set_param();
    return nullptr;
}

void GraphDef::_validate()
{

}

void GraphDef::_create()
{

}

void GraphDef::_connect()
{

}

void GraphDef::_set_param()
{

}


    } // framework
} // sigproc