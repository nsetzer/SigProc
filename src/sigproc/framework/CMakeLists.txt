

include(sigproc)

set(FRAMEWORK_PATH "${PROJECT_SOURCE_DIR}/sigproc/framework")

set(src_framework_cpp
    ${FRAMEWORK_PATH}/composite.cpp
    ${FRAMEWORK_PATH}/compositestream.cpp
    ${FRAMEWORK_PATH}/config.cpp
    ${FRAMEWORK_PATH}/enum.cpp
    ${FRAMEWORK_PATH}/factory.cpp
    ${FRAMEWORK_PATH}/graph.cpp
    ${FRAMEWORK_PATH}/graphdef.cpp
    ${FRAMEWORK_PATH}/lisp.cpp
    ${FRAMEWORK_PATH}/node.cpp
    ${FRAMEWORK_PATH}/port.cpp
    ${FRAMEWORK_PATH}/processor.cpp
    ${FRAMEWORK_PATH}/registry.cpp
    ${FRAMEWORK_PATH}/stream.cpp
)

set(src_framework_hpp
    ${FRAMEWORK_PATH}/composite.hpp
    ${FRAMEWORK_PATH}/compositestream.hpp
    ${FRAMEWORK_PATH}/config.hpp
    ${FRAMEWORK_PATH}/enum.hpp
    ${FRAMEWORK_PATH}/factory.hpp
    ${FRAMEWORK_PATH}/graph.hpp
    ${FRAMEWORK_PATH}/graphdef.hpp
    ${FRAMEWORK_PATH}/lisp.hpp
    ${FRAMEWORK_PATH}/node.hpp
    ${FRAMEWORK_PATH}/port.hpp
    ${FRAMEWORK_PATH}/processor.hpp
    ${FRAMEWORK_PATH}/registry.hpp
    ${FRAMEWORK_PATH}/stream.hpp
)

set(src_framework_all
        ${src_framework_cpp}
        ${src_framework_hpp}
    )

create_object_library(SigprocFramework_object ${src_framework_all})
# $<TARGET_OBJECTS:SigprocFramework_object>

create_shared_library(SigprocFrameworkTest
    $<TARGET_OBJECTS:SigprocFramework_object>
    $<TARGET_OBJECTS:SigprocUnitTest_object>
)

function(create_framework_test base)

    add_executable(${base}
        ${FRAMEWORK_PATH}/test/${base}.cpp
    )

    TARGET_LINK_LIBRARIES(${base} SigprocFrameworkTest)

    set_target_properties(${base}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

endfunction()

create_framework_test(composite_test)
create_framework_test(compositestream_test)
create_framework_test(lisp_test)