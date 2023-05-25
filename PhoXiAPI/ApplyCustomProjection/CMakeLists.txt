# CMakeLists files in this project can
# refer to the root source directory of the project as ${ApplyCustomProjectionExample_SOURCE_DIR} and
# to the root binary directory of the project as ${ApplyCustomProjectionExample_BINARY_DIR}.
cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project(ApplyCustomProjectionExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if(UNIX AND NOT APPLE)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX AND NOT APPLE)

set(Files
    ${ApplyCustomProjectionExample_SOURCE_DIR}/ApplyCustomProjectionExample.cpp
    ${ApplyCustomProjectionExample_SOURCE_DIR}/ReadMe.txt
)

add_executable(ApplyCustomProjectionExample
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")

if(UNIX AND NOT APPLE)
    set(OPEN_CV_PATH "$ENV{HOME}/OpenCV" CACHE PATH "")
else()
    set(OPEN_CV_PATH "C:/opencv/build" CACHE PATH "")
endif()

set(PHO_OPENCV_LIBS "world")
find_package(OpenCV QUIET COMPONENTS ${PHO_OPENCV_LIBS} PATHS "${OPEN_CV_PATH}")

if (NOT ${OPENCV_WORLD_FOUND})
    set(PHO_OPENCV_LIBS "core;imgcodecs;imgproc")
    find_package(OpenCV REQUIRED COMPONENTS ${PHO_OPENCV_LIBS} PATHS "${OPEN_CV_PATH}")
endif()

foreach(PHO_OPENCV_LIB ${PHO_OPENCV_LIBS})
    list(APPEND PHO_TARGET_OPENCV_LIBS "opencv_${PHO_OPENCV_LIB}")
endforeach()

target_link_libraries(ApplyCustomProjectionExample ${PHO_TARGET_OPENCV_LIBS})

set(OPEN_CV_BIN_PATH "${OpenCV_LIB_DIR_OPT}/../bin")
 
target_link_libraries(ApplyCustomProjectionExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    foreach(PHO_OPENCV_LIB ${PHO_OPENCV_LIBS})
        set(OPEN_CV_RELEASE_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310.dll")
        set(OPEN_CV_DEBUG_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310d.dll")
        
        if (NOT EXISTS ${OPEN_CV_DEBUG_DLL_PATH})
            set(OPEN_CV_DEBUG_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310-gd.dll")
        endif()
    
        add_custom_command(TARGET ApplyCustomProjectionExample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<$<CONFIG:Release>:${OPEN_CV_RELEASE_DLL_PATH}>
                $<$<CONFIG:Debug>:${OPEN_CV_DEBUG_DLL_PATH}>
                $<TARGET_FILE_DIR:ApplyCustomProjectionExample>
        )
        add_custom_command(TARGET ApplyCustomProjectionExample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<$<CONFIG:Release>:${PHOXI_DLL_RELEASE}>
                $<$<CONFIG:Debug>:${PHOXI_DLL_DEBUG}>
                $<TARGET_FILE_DIR:ApplyCustomProjectionExample>
        )
    endforeach()
endif()

target_include_directories(ApplyCustomProjectionExample PUBLIC
    ${PHOXI_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)
