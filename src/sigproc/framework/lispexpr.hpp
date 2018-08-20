#ifndef SIGPROC_FRAMEWORK_LISPEXPR_HPP
#define SIGPROC_FRAMEWORK_LISPEXPR_HPP


#include "sigproc/framework/enum.hpp"
#include "sigproc/framework/composite.hpp"
#include "sigproc/framework/compositestream.hpp"

namespace sigproc {
    namespace framework {

typedef std::function<upComposite(CompositeVector&)> LispFunction;
typedef std::map<std::string, LispFunction> LispFunctionMap;

upComposite lisp_plus(CompositeVector& vec);
upComposite lisp_subtract(CompositeVector& vec);
upComposite lisp_multiply(CompositeVector& vec);
upComposite lisp_divide(CompositeVector& vec);


    } // framework
} // sigproc

#endif