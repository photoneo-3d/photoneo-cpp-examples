cmake_minimum_required (VERSION 3.10)
project(RotatedCalibrationExample)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")


if (UNIX AND NOT APPLE)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX AND NOT APPLE)


set (Files
    RotatedCalibrationExample.cpp
    ReadMe.txt
)

add_executable (RotatedCalibrationExample 
    ${Files}
)

set(CMAKE_AUTOMOC ON)


if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")

target_link_libraries(RotatedCalibrationExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder RotatedCalibrationExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder RotatedCalibrationExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)


if(MSVC)
    add_custom_command(TARGET RotatedCalibrationExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<CONFIG:Release>:${PHOXI_DLL_RELEASE}>
            $<$<CONFIG:Debug>:${PHOXI_DLL_DEBUG}>
            $<TARGET_FILE_DIR:RotatedCalibrationExample>
    )
endif(MSVC)

target_include_directories (RotatedCalibrationExample PUBLIC ${PHOXI_INCLUDE_DIRS})



set_target_properties(RotatedCalibrationExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)