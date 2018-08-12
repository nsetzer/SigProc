
#####################################################
#create_object_library(MyLib ${src})
function(create_object_library lib_name)
    add_library(${lib_name} OBJECT ${ARGN})
    set_target_properties(${lib_name}
        PROPERTIES
            CXX_STANDARD 11
            POSITION_INDEPENDENT_CODE 1)
endfunction()

#####################################################
#create_object_library(MyLib ${src})
function(create_shared_library lib_name)
    add_library(${lib_name} SHARED ${ARGN})
    set_target_properties(${lib_name}
        PROPERTIES
            CXX_STANDARD 11)
endfunction()
