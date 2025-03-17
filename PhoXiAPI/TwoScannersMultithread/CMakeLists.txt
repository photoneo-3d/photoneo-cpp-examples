# CMakeLists files in this project can
# refer to the root source directory of the project as ${TwoScannersMultithreadExample_SOURCE_DIR} and
# to the root binary directory of the project as ${TwoScannersMultithreadExample_BINARY_DIR}.
cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (TwoScannersMultithreadExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

if (UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX)

set(Files
    ${TwoScannersMultithreadExample_SOURCE_DIR}/TwoScannersMultithreadExample.cpp
    ${TwoScannersMultithreadExample_SOURCE_DIR}/ReadMe.txt
)

add_executable (TwoScannersMultithreadExample
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(TwoScannersMultithreadExample
    ${PHOXI_LIBRARY}
    Threads::Threads
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder TwoScannersMultithreadExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder TwoScannersMultithreadExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if(MSVC)
    if(NOT PHOXI_DLL_FOR_EXAMPLE)
        set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
    endif(NOT PHOXI_DLL_FOR_EXAMPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PHOXI_DLL_FOR_EXAMPLE}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif(MSVC)

target_include_directories (TwoScannersMultithreadExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(TwoScannersMultithreadExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)
