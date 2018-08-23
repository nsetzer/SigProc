include(GenerateExportHeader)

#####################################################
#create_object_library(MyLib ${src})
function(create_object_library lib_name)
    add_library(${lib_name} OBJECT ${ARGN})
    #set_target_properties(${lib_name}
    #    PROPERTIES
    #        POSITION_INDEPENDENT_CODE 1)
    generate_export_header(${lib_name})
    target_compile_definitions(${lib_name} PUBLIC SIPROC_EXPORT=1)
endfunction()

#####################################################
#create_shared_library(MyLib ${src})
function(create_shared_library lib_name)
    add_library(${lib_name} SHARED ${ARGN})
    generate_export_header(${lib_name})
    target_compile_definitions(${lib_name} PUBLIC SIPROC_EXPORT=1)
endfunction()

#####################################################
#create_static_library(MyLib ${src})
function(create_static_library lib_name)
    add_library(${lib_name} STATIC ${ARGN})
    generate_export_header(${lib_name})
    target_compile_definitions(${lib_name} PUBLIC SIPROC_EXPORT=1)
endfunction()
