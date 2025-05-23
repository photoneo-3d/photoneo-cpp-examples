# CMakeLists files in this project can
# refer to the root source directory of the project as ${MinimalPclExample_SOURCE_DIR} and
# to the root binary directory of the project as ${MinimalPclExample_BINARY_DIR}.
cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (MinimalPclExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")


if (UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX)

set(Files
    ${MinimalPclExample_SOURCE_DIR}/MinimalPclExample.cpp
    ${MinimalPclExample_SOURCE_DIR}/ReadMe.txt
)

add_executable (MinimalPclExample
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(MinimalPclExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)
target_include_directories(MinimalPclExample PUBLIC
    ${PHOXI_INCLUDE_DIRS}
)

if(TARGET PCL::PCL)
    target_link_libraries(MinimalPclExample
        PCL::PCL
    )
else()
    if(UNIX)
        set(PCL_PATH "$ENV{HOME}/pcl-1.8.0/share/pcl-1.8" CACHE PATH "PCL library path")
    else()
        set(PCL_PATH "C:/pcl-1.8.0/cmake" CACHE PATH "PCL library path")
    endif()
    find_package(PCL 1.8.0 EXACT REQUIRED PATHS "${PCL_PATH}")
    target_link_libraries(MinimalPclExample
        ${PCL_COMMON_LIBRARIES}
    )
    target_include_directories(MinimalPclExample PUBLIC
        ${PCL_INCLUDE_DIRS}
    )
endif()

get_target_property(ReleaseBinFolder MinimalPclExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder MinimalPclExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if(MSVC)
    if(NOT PHOXI_DLL_FOR_EXAMPLE)
        set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PHOXI_DLL_FOR_EXAMPLE}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

set_target_properties(MinimalPclExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)
