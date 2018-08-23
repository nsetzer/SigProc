



include(sigproc)

set(THIRDPARTY_PATH "${PROJECT_SOURCE_DIR}/sigproc/thirdparty")
set(FFMPEG_PATH "${PROJECT_SOURCE_DIR}/alien/ffmpeg")
set(FFTW_PATH "${PROJECT_SOURCE_DIR}/alien/fftw")

set(src_fftw_cpp
    ${THIRDPARTY_PATH}/fftw/rfft.cpp
)

set(src_fftw_hpp
    ${THIRDPARTY_PATH}/fftw/rfft.hpp
)

set(src_fftw_all
        ${src_fftw_cpp}
        ${src_fftw_hpp}
    )

find_library(FFTW3_PATH fftw3
    PATHS "${FFTW_PATH}/.libs")
message(warning " FFTW3_PATH: ${FFTW3_PATH}")

include_directories("${FFTW_PATH}/api")

function(create_thirdparty_fftw_test module base)

    add_executable("${module}_${base}"
        ${THIRDPARTY_PATH}/${module}/test/${base}.cpp
        ${src_fftw_all}
        $<TARGET_OBJECTS:SigprocCommon_object>
        $<TARGET_OBJECTS:SigprocUnitTest_object>
    )

    target_link_libraries("${module}_${base}" ${FFTW3_PATH})
    set_target_properties("${module}_${base}" PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")
    set_target_properties("${module}_${base}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

endfunction()

create_thirdparty_fftw_test(fftw rfft_test)

set(src_ffmpeg_hpp
    ${THIRDPARTY_PATH}/ffmpeg/decode.hpp
)

set(src_ffmpeg_cpp
    ${THIRDPARTY_PATH}/ffmpeg/decode.cpp
)

set(src_ffmpeg_hpp
    ${THIRDPARTY_PATH}/ffmpeg/decode.hpp
)

set(src_ffmpeg_all
        ${src_ffmpeg_cpp}
        ${src_ffmpeg_hpp}
    )


find_library(FFMPEG_LIBAVUTIL_PATH avutil
    PATHS "${FFMPEG_PATH}/libavutil")
message(warning " FFMPEG_LIBAVUTIL_PATH: ${FFMPEG_LIBAVUTIL_PATH}")

find_library(FFMPEG_LIBAVCODEC_PATH avcodec
    PATHS "${FFMPEG_PATH}/libavcodec")
message(warning " FFMPEG_LIBAVCODEC_PATH: ${FFMPEG_LIBAVCODEC_PATH}")

find_library(FFMPEG_LIBAVFORMAT_PATH avformat
    PATHS "${FFMPEG_PATH}/libavformat")
message(warning " FFMPEG_LIBAVFORMAT_PATH: ${FFMPEG_LIBAVFORMAT_PATH}")

find_library(FFMPEG_LIBSWRESAMPLE_PATH swresample
    PATHS "${FFMPEG_PATH}/libswresample")
message(warning " FFMPEG_LIBSWRESAMPLE_PATH: ${FFMPEG_LIBSWRESAMPLE_PATH}")

find_library(FFMPEG_LIBSWSCALE_PATH swscale
    PATHS "${FFMPEG_PATH}/libswscale")
message(warning " FFMPEG_LIBSWSCALE_PATH: ${FFMPEG_LIBSWSCALE_PATH}")

include_directories("${FFMPEG_PATH}")

#create_static_library(SigprocThirdpartyFFmpeg ${src_ffmpeg_all})

#set_target_properties(SigprocThirdpartyFFmpeg PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

#create_object_library(SigprocThirdparty_object ${src_framework_all})
# $<TARGET_OBJECTS:SigprocFramework_object>

#create_shared_library(SigprocThirdpartyTest
#    # $<TARGET_OBJECTS:SigprocThirdparty_object>
#    $<TARGET_OBJECTS:SigprocCommon_object>
#    $<TARGET_OBJECTS:SigprocUnitTest_object>
#)

#target_link_libraries(SigprocThirdpartyTest SigprocThirdpartyFFmpeg)
#target_link_libraries(SigprocThirdpartyTest ${FFMPEG_LIBSWRESAMPLE_PATH})
#target_link_libraries(SigprocThirdpartyTest ${FFMPEG_LIBAVFORMAT_PATH})
#target_link_libraries(SigprocThirdpartyTest ${FFMPEG_LIBAVCODEC_PATH})
#target_link_libraries(SigprocThirdpartyTest ${FFMPEG_LIBAVUTIL_PATH})
#target_link_libraries(SigprocThirdpartyTest ${FFMPEG_LIBSWSCALE_PATH})

function(create_thirdparty_test module base)

    add_executable("${module}_${base}"
        ${THIRDPARTY_PATH}/${module}/test/${base}.cpp
        ${src_ffmpeg_all}
        $<TARGET_OBJECTS:SigprocCommon_object>
        $<TARGET_OBJECTS:SigprocUnitTest_object>
    )

    #TARGET_LINK_LIBRARIES("${module}_${base}" SigprocThirdpartyTest)

    target_link_libraries("${module}_${base}" ${FFMPEG_LIBSWRESAMPLE_PATH})
    target_link_libraries("${module}_${base}" ${FFMPEG_LIBAVFORMAT_PATH})
    target_link_libraries("${module}_${base}" ${FFMPEG_LIBAVCODEC_PATH})
    target_link_libraries("${module}_${base}" ${FFMPEG_LIBAVUTIL_PATH})
    #target_link_libraries("${module}_${base}" ${FFMPEG_LIBSWSCALE_PATH})
    target_link_libraries("${module}_${base}" rt)
    target_link_libraries("${module}_${base}" pthread)
    target_link_libraries("${module}_${base}" m)

    set_target_properties("${module}_${base}" PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

    set_target_properties("${module}_${base}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

endfunction()

function(create_thirdparty_tool name source)

    add_executable("${name}"
        ${source}
        ${src_ffmpeg_all}
        $<TARGET_OBJECTS:SigprocCommon_object>
    )

    #TARGET_LINK_LIBRARIES("${module}_${base}" SigprocThirdpartyTest)

    target_link_libraries("${name}" ${FFMPEG_LIBSWRESAMPLE_PATH})
    target_link_libraries("${name}" ${FFMPEG_LIBAVFORMAT_PATH})
    target_link_libraries("${name}" ${FFMPEG_LIBAVCODEC_PATH})
    target_link_libraries("${name}" ${FFMPEG_LIBAVUTIL_PATH})
    #target_link_libraries("${module}_${base}" ${FFMPEG_LIBSWSCALE_PATH})
    # target_link_libraries("${name}" rt)
    target_link_libraries("${name}" pthread)
    target_link_libraries("${name}" m)

    set_target_properties("${name}" PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

    set_target_properties("${name}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

endfunction()

create_thirdparty_tool(resample ${THIRDPARTY_PATH}/ffmpeg/tools/resample.cpp)