cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project(ColorCameraCalibrationToROS)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if (UNIX AND NOT APPLE)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX AND NOT APPLE)

set(Files
    ${ColorCameraCalibrationToROS_SOURCE_DIR}/ColorCameraCalibrationToROS.cpp
    ${ColorCameraCalibrationToROS_SOURCE_DIR}/ReadMe.txt
)

add_executable(ColorCameraCalibrationToROS
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")

target_link_libraries(ColorCameraCalibrationToROS
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

if(MSVC)
    add_custom_command(TARGET ColorCameraCalibrationToROS POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<CONFIG:Release>:${PHOXI_DLL_RELEASE}>
            $<$<CONFIG:Debug>:${PHOXI_DLL_DEBUG}>
            $<TARGET_FILE_DIR:ColorCameraCalibrationToROS>
    )
endif()

target_include_directories(ColorCameraCalibrationToROS PUBLIC 
    ${PHOXI_INCLUDE_DIRS}
)
