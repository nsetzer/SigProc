
set(BIN_PATH "${PROJECT_SOURCE_DIR}/sigproc/bin")

add_executable(demo ${BIN_PATH}/demo.cpp
                    $<TARGET_OBJECTS:SigprocFramework_object>
                    $<TARGET_OBJECTS:SigprocNodes_object>
)