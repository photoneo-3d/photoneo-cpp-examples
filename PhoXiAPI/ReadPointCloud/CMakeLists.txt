# CMakeLists files in this project can
# refer to the root source directory of the project as ${ReadPointCloudExample_SOURCE_DIR} and
# to the root binary directory of the project as ${ReadPointCloudExample_BINARY_DIR}.
cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (ReadPointCloudExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if (UNIX AND NOT APPLE)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX AND NOT APPLE)

set(Files
    ${ReadPointCloudExample_SOURCE_DIR}/ReadPointCloudExample.cpp
    ${ReadPointCloudExample_SOURCE_DIR}/ReadMe.txt
)

add_executable (ReadPointCloudExample
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")

target_link_libraries(ReadPointCloudExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder ReadPointCloudExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder ReadPointCloudExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_custom_command(TARGET ReadPointCloudExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<CONFIG:Release>:${PHOXI_DLL_RELEASE}>
            $<$<CONFIG:Debug>:${PHOXI_DLL_DEBUG}>
            $<TARGET_FILE_DIR:ReadPointCloudExample>
    )
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

target_include_directories (ReadPointCloudExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(ReadPointCloudExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)
