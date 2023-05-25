# CMakeLists files in this project can
# refer to the root source directory of the project as ${ConnectAndGrabExample_SOURCE_DIR} and
# to the root binary directory of the project as ${ConnectAndGrabExample_BINARY_DIR}.
cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (ConnectAndGrabExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")


if (UNIX AND NOT APPLE)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX AND NOT APPLE)

set(Files
    ${ConnectAndGrabExample_SOURCE_DIR}/ConnectAndGrabExample.cpp
    ${ConnectAndGrabExample_SOURCE_DIR}/ReadMe.txt
)

add_executable (ConnectAndGrabExample 
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")

target_link_libraries(ConnectAndGrabExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder ConnectAndGrabExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder ConnectAndGrabExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if(MSVC)
    add_custom_command(TARGET ConnectAndGrabExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<CONFIG:Release>:${PHOXI_DLL_RELEASE}>
            $<$<CONFIG:Debug>:${PHOXI_DLL_DEBUG}>
            $<TARGET_FILE_DIR:ConnectAndGrabExample>
    )
endif(MSVC)

target_include_directories (ConnectAndGrabExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(ConnectAndGrabExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)