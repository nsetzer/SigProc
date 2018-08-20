

include(sigproc)

set(COMMON_PATH "${PROJECT_SOURCE_DIR}/sigproc/common")

set(src_common_cpp
    ${COMMON_PATH}/argparse.cpp
    ${COMMON_PATH}/logging.cpp
    ${COMMON_PATH}/format.cpp
)

set(src_common_hpp
    ${COMMON_PATH}/argparse.hpp
    ${COMMON_PATH}/exception.hpp
    ${COMMON_PATH}/logging.hpp
    ${COMMON_PATH}/format.hpp
)

set(src_unittest_all
        ${src_common_cpp}
        ${src_common_hpp}
    )

create_object_library(SigprocCommon_object
    ${src_unittest_all})
# $<TARGET_OBJECTS:SigprocCommon_object>


set(UNITTEST_PATH "${PROJECT_SOURCE_DIR}/sigproc/common/unittest")

set(src_unittest_cpp
    ${UNITTEST_PATH}/unittest.cpp
    ${UNITTEST_PATH}/unittest_main.cpp
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
    ${src_unittest_all})
# $<TARGET_OBJECTS:SigprocUnitTest_object>

