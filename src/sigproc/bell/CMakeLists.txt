
include(sigproc)

set(BELL_PATH "${PROJECT_SOURCE_DIR}/sigproc/bell")
set(FFMPEG_PATH "${PROJECT_SOURCE_DIR}/alien/ffmpeg")
set(FFTW_PATH "${PROJECT_SOURCE_DIR}/alien/fftw")

###############################################################################
# FFTW

set(src_fftw_cpp
    ${BELL_PATH}/fftw/rfft.cpp
)

set(src_fftw_hpp
    ${BELL_PATH}/fftw/rfft.hpp
)

set(src_fftw_all
        ${src_fftw_cpp}
        ${src_fftw_hpp}
    )

find_library(FFTW3_PATH fftw3
    PATHS "${FFTW_PATH}/.libs")
message(warning " FFTW3_PATH: ${FFTW3_PATH}")

include_directories("${FFTW_PATH}/api")

create_static_library(SigprocBellFFTW ${src_fftw_all})
set_target_properties(SigprocBellFFTW PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

###############################################################################
# FFmpeg

set(src_ffmpeg_hpp
    ${BELL_PATH}/ffmpeg/decode.hpp
)

set(src_ffmpeg_cpp
    ${BELL_PATH}/ffmpeg/decode.cpp
)

set(src_ffmpeg_hpp
    ${BELL_PATH}/ffmpeg/decode.hpp
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

create_static_library(SigprocBellFFmpeg ${src_ffmpeg_all})
set_target_properties(SigprocBellFFmpeg PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast")

###############################################################################
# Algorithm

set(src_algorithm_cpp
    ${BELL_PATH}/algorithm/directform.cpp
    ${BELL_PATH}/algorithm/dct.cpp
    ${BELL_PATH}/algorithm/mfcc.cpp
    ${BELL_PATH}/algorithm/window.cpp
    ${BELL_PATH}/algorithm/zbfilter.cpp
)

set(src_algorithm_hpp
    ${BELL_PATH}/algorithm/directform.hpp
    ${BELL_PATH}/algorithm/dct.hpp
    ${BELL_PATH}/algorithm/mfcc.hpp
    ${BELL_PATH}/algorithm/window.hpp
    ${BELL_PATH}/algorithm/zbfilter.hpp
)

set(src_algorithm_all
        ${src_algorithm_cpp}
        ${src_algorithm_hpp}
    )

###############################################################################
# BELL

set(src_bell_cpp
    ${BELL_PATH}/base.cpp
)

set(src_bell_hpp
    ${BELL_PATH}/base.hpp
)

set(src_bell_all
        ${src_bell_cpp}
        ${src_bell_hpp}
    )

create_object_library(SigprocBell_object
    ${src_bell_all}
    ${src_algorithm_all})
# $<TARGET_OBJECTS:SigprocBell_object>

create_shared_library(SigprocBell
    $<TARGET_OBJECTS:SigprocBell_object>
    $<TARGET_OBJECTS:SigprocCommon_object>
)

target_link_libraries(SigprocBell SigprocBellFFTW)
target_link_libraries(SigprocBell ${FFTW3_PATH})
target_link_libraries(SigprocBell SigprocBellFFmpeg)
target_link_libraries(SigprocBell ${FFMPEG_LIBSWRESAMPLE_PATH})
target_link_libraries(SigprocBell ${FFMPEG_LIBAVFORMAT_PATH})
target_link_libraries(SigprocBell ${FFMPEG_LIBAVCODEC_PATH})
target_link_libraries(SigprocBell ${FFMPEG_LIBAVUTIL_PATH})
target_link_libraries(SigprocBell pthread)

function(create_bell_tool name source)

    add_executable("${name}"
        ${source}
    )

    target_link_libraries("${name}" SigprocBell)

    set_target_properties("${name}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endfunction()

function(create_bell_test name source)

    add_executable("${name}"
        ${source}
        $<TARGET_OBJECTS:SigprocUnitTest_object>
    )

    target_link_libraries("${name}" SigprocBell)

    set_target_properties("${name}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endfunction()

create_bell_test(rfft_test ${BELL_PATH}/fftw/test/rfft_test.cpp)
create_bell_test(directform_test ${BELL_PATH}/algorithm/test/directform_test.cpp)
create_bell_test(zbfilter_test ${BELL_PATH}/algorithm/test/zbfilter_test.cpp)
create_bell_test(mfcc_test ${BELL_PATH}/algorithm/test/mfcc_test.cpp)
create_bell_test(dct_test ${BELL_PATH}/algorithm/test/dct_test.cpp)

create_bell_tool(resample ${BELL_PATH}/ffmpeg/tools/resample.cpp)
create_bell_tool(framegen ${BELL_PATH}/ffmpeg/tools/framegen.cpp)




