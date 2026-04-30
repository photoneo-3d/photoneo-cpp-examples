cmake_minimum_required (VERSION 3.24)

project (CAPI LANGUAGES C)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

set(Files
    CAPIExample.c
    ReadMe.txt
)

add_executable (CAPIExample 
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(CAPIExample
    phoxi::phoxi
)

if(MSVC)
    if(NOT PHOXI_DLL_FOR_EXAMPLE)
        set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
    endif(NOT PHOXI_DLL_FOR_EXAMPLE)
    add_custom_command(TARGET CAPIExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PHOXI_DLL_FOR_EXAMPLE}
            $<TARGET_FILE_DIR:CAPIExample>
    )
endif(MSVC)

set_target_properties(CAPIExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)