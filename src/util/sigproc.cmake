include(GenerateExportHeader)

#####################################################
#create_object_library(MyLib ${src})
function(create_object_library lib_name)
    add_library(${lib_name} OBJECT ${ARGN})
    #set_target_properties(${lib_name}
    #    PROPERTIES
    #        POSITION_INDEPENDENT_CODE 1)
    generate_export_header(${lib_name})
endfunction()

#####################################################
#create_object_library(MyLib ${src})
function(create_shared_library lib_name)
    add_library(${lib_name} SHARED ${ARGN})
    generate_export_header(${lib_name})
endfunction()
