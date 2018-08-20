

include(sigproc)

set(NODES_PATH "${PROJECT_SOURCE_DIR}/sigproc/nodes")

set(src_nodes_cpp
    ${NODES_PATH}/adder.cpp
)

set(src_nodes_hpp
    ${NODES_PATH}/adder.hpp
)

set(src_nodes_all
        ${src_nodes_cpp}
        ${src_nodes_hpp}
    )

create_object_library(SigprocNodes_object ${src_nodes_all})
# $<TARGET_OBJECTS:SigprocNodes_object>

create_shared_library(SigprocNodeTest
    $<TARGET_OBJECTS:SigprocFramework_object>
    $<TARGET_OBJECTS:SigprocCommon_object>
    $<TARGET_OBJECTS:SigprocNodes_object>
    $<TARGET_OBJECTS:SigprocUnitTest_object>
)


function(create_node_test base)

    add_executable(${base}
        ${NODES_PATH}/test/${base}.cpp
    )

    TARGET_LINK_LIBRARIES(${base} SigprocNodeTest)

    set_target_properties(${base}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

endfunction()

create_node_test(adder_test)