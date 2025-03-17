cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (ExternalCameraExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if(UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
else(UNIX)
    add_compile_options(/MP)
endif(UNIX)

set(Settings_DIR "${ExternalCameraExample_SOURCE_DIR}/Settings")
set(Data_DIR "${ExternalCameraExample_SOURCE_DIR}/Data")

if (PHO_DOWNLOAD_API_EXAMPLES_DATA)
    set(DATA_URL "https://photoneo.com/files/installer/PhoXi/api/external_camera_example_1.2.zip")
    set(DATA_DOWNLOAD_PATH "${ExternalCameraExample_BINARY_DIR}/external_camera_example_1.2.zip")
    
    if (NOT EXISTS "${DATA_DIR}" AND NOT EXISTS "${DATA_DOWNLOAD_PATH}")
        message("Downloading file")
        message("  src=${DATA_URL}")
        message("  dst=${DATA_DOWNLOAD_PATH}")
        
        file(DOWNLOAD "${DATA_URL}" "${DATA_DOWNLOAD_PATH}" STATUS DOWNLOAD_STATUS SHOW_PROGRESS)
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_STATUS_STRING)
        
        if (DOWNLOAD_STATUS_CODE)
            message("Download failed: ${DOWNLOAD_STATUS_STRING} (Error: ${DOWNLOAD_STATUS_CODE})")
            file(REMOVE ${DATA_DOWNLOAD_PATH})
        else()
            message("Downloading successful")
        endif()
    endif()
    
    if (EXISTS "${DATA_DOWNLOAD_PATH}" AND NOT EXISTS "${DATA_DIR}")
        message("Unpacking file ${DATA_DOWNLOAD_PATH}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${DATA_DOWNLOAD_PATH}
            WORKING_DIRECTORY "${ExternalCameraExample_BINARY_DIR}")
    endif()
endif()

file(COPY ${Settings_DIR}/
    DESTINATION ${ExternalCameraExample_BINARY_DIR}/Settings/
)

if (EXISTS ${Data_DIR} AND NOT EXISTS ${ExternalCameraExample_BINARY_DIR}/Data)
    file(COPY ${Data_DIR}/
    DESTINATION ${ExternalCameraExample_BINARY_DIR}/Data/)
endif()

if(NOT PhoXiAPI_ExampleUtils_DIR)
    set(PhoXiAPI_ExampleUtils_DIR "${ExternalCameraExample_SOURCE_DIR}/Utils")
endif()

set(SOURCE_LIST
    ${ExternalCameraExample_SOURCE_DIR}/Calibration.cpp
    ${ExternalCameraExample_SOURCE_DIR}/Calibration.h
    ${ExternalCameraExample_SOURCE_DIR}/ColorPointCloud.cpp
    ${ExternalCameraExample_SOURCE_DIR}/ColorPointCloud.h
    ${ExternalCameraExample_SOURCE_DIR}/DepthMap.cpp
    ${ExternalCameraExample_SOURCE_DIR}/DepthMap.h
    ${ExternalCameraExample_SOURCE_DIR}/ExternalCamera.cpp
    ${ExternalCameraExample_SOURCE_DIR}/ExternalCamera.h
    ${ExternalCameraExample_SOURCE_DIR}/Main.cpp
    ${ExternalCameraExample_SOURCE_DIR}/ReadMe.txt
)

set(PhoXiAPI_ExampleUtils_LIST
    ${PhoXiAPI_ExampleUtils_DIR}/Calibration.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Calibration.h
    ${PhoXiAPI_ExampleUtils_DIR}/Scanner.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Scanner.h
    ${PhoXiAPI_ExampleUtils_DIR}/FileCamera.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/FileCamera.h
    ${PhoXiAPI_ExampleUtils_DIR}/Util.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Util.h
    ${PhoXiAPI_ExampleUtils_DIR}/Checks.cpp
    ${PhoXiAPI_ExampleUtils_DIR}/Checks.h 
    ${PhoXiAPI_ExampleUtils_DIR}/Info.h   
)

add_executable (ExternalCameraExample 
    ${SOURCE_LIST} ${PhoXiAPI_ExampleUtils_LIST}
)

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${PhoXiAPI_ExampleUtils_DIR}/ PREFIX "Utils" FILES ${PhoXiAPI_ExampleUtils_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

if (UNIX)
    set(OPEN_CV_PATH "$ENV{HOME}/OpenCV" CACHE PATH "OpenCV library path")
else (UNIX)
    set(OPEN_CV_PATH "C:/opencv/build" CACHE PATH "OpenCV library path")
endif (UNIX)

set(PHO_OPENCV_LIBS "world")
find_package(OpenCV QUIET COMPONENTS ${PHO_OPENCV_LIBS} PATHS "${OPEN_CV_PATH}")

if (NOT ${OPENCV_WORLD_FOUND})
    set(PHO_OPENCV_LIBS "core;imgcodecs;imgproc")
    find_package(OpenCV REQUIRED COMPONENTS ${PHO_OPENCV_LIBS} PATHS "${OPEN_CV_PATH}")
endif()

foreach(PHO_OPENCV_LIB ${PHO_OPENCV_LIBS})
    list(APPEND PHO_TARGET_OPENCV_LIBS "opencv_${PHO_OPENCV_LIB}")
endforeach()

target_link_libraries(ExternalCameraExample ${PHO_TARGET_OPENCV_LIBS})

set(OPEN_CV_BIN_PATH "${OpenCV_LIB_DIR_OPT}/../bin")

target_link_libraries(ExternalCameraExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder ExternalCameraExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder ExternalCameraExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(MSVC_TOOLSET_VERSION GREATER_EQUAL 140)
        set(PHO_COMPILER_VERSION "vc14")
    elseif(MSVC_TOOLSET_VERSION EQUAL 120)
        set(PHO_COMPILER_VERSION "vc12")
    endif()
    
    foreach(PHO_OPENCV_LIB ${PHO_OPENCV_LIBS})
        set(OPEN_CV_RELEASE_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310.dll")
        set(OPEN_CV_DEBUG_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310d.dll")
        
        if (NOT EXISTS ${OPEN_CV_DEBUG_DLL_PATH})
            set(OPEN_CV_DEBUG_DLL_PATH "${OPEN_CV_BIN_PATH}/opencv_${PHO_OPENCV_LIB}310-gd.dll")
        endif()
        
        add_custom_command(TARGET ExternalCameraExample POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<$<CONFIG:Release>:${OPEN_CV_RELEASE_DLL_PATH}>
                $<$<CONFIG:Debug>:${OPEN_CV_DEBUG_DLL_PATH}>
                $<TARGET_FILE_DIR:ExternalCameraExample>
        )
        if(NOT PHOXI_DLL_FOR_EXAMPLE)
            set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
        endif(NOT PHOXI_DLL_FOR_EXAMPLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PHOXI_DLL_FOR_EXAMPLE}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endforeach()
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

target_include_directories(ExternalCameraExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(ExternalCameraExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)
