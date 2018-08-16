
set(BIN_PATH "${PROJECT_SOURCE_DIR}/sigproc/bin")

add_executable(demo ${BIN_PATH}/demo.cpp
                    $<TARGET_OBJECTS:SigprocFramework_object>
                    $<TARGET_OBJECTS:SigprocNodes_object>
                    $<TARGET_OBJECTS:SigprocCommon_object>
)

add_executable(validate_json ${BIN_PATH}/validate_json.cpp
                    $<TARGET_OBJECTS:SigprocFramework_object>
                    $<TARGET_OBJECTS:SigprocNodes_object>
                    $<TARGET_OBJECTS:SigprocCommon_object>
)