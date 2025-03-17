cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project(ReprojectionToExternalCameraExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if(UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
else(UNIX)
    add_compile_options(/MP)
endif(UNIX)

if(NOT PhoXiAPI_ExampleUtils_DIR)
    set(PhoXiAPI_ExampleUtils_DIR "${ReprojectionToExternalCameraExample_SOURCE_DIR}/Utils")
endif()

set(SOURCE_LIST
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/Calibration.cpp
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/Calibration.h
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/Reprojection.cpp
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/Reprojection.h
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/Main.cpp
    ${ReprojectionToExternalCameraExample_SOURCE_DIR}/ReadMe.txt
)

set(PhoXiAPI_ExampleUtils_LIST
    ${PhoXiAPI_ExampleUtils_DIR}/Calibration.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Calibration.h
    ${PhoXiAPI_ExampleUtils_DIR}/Scanner.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Scanner.h
    ${PhoXiAPI_ExampleUtils_DIR}/Util.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Util.h
    ${PhoXiAPI_ExampleUtils_DIR}/Info.h
)

add_executable(ReprojectionToExternalCameraExample 
    ${SOURCE_LIST} ${PhoXiAPI_ExampleUtils_LIST}
)

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${PhoXiAPI_ExampleUtils_DIR}/ PREFIX "Utils" FILES ${PhoXiAPI_ExampleUtils_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

if(NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(ReprojectionToExternalCameraExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder ReprojectionToExternalCameraExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder ReprojectionToExternalCameraExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(NOT PHOXI_DLL_FOR_EXAMPLE)
        set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
    endif(NOT PHOXI_DLL_FOR_EXAMPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PHOXI_DLL_FOR_EXAMPLE}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

target_include_directories(ReprojectionToExternalCameraExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(ReprojectionToExternalCameraExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)
