

include(sigproc)

set(UNITTEST_PATH "${PROJECT_SOURCE_DIR}/sigproc/common/unittest")

set(src_unittest_cpp
    ${UNITTEST_PATH}/unittest.cpp
)

set(src_unittest_hpp
    ${UNITTEST_PATH}/unittest.hpp
    ${UNITTEST_PATH}/unittest_assert.hpp
)

set(src_unittest_all
        ${src_unittest_cpp}
        ${src_unittest_hpp}
    )

create_object_library(SigprocUnitTest_object
    ${src_unittest_all}
    ${UNITTEST_PATH}/unittest_main.cpp)
# $<TARGET_OBJECTS:SigprocUnitTest_object>

