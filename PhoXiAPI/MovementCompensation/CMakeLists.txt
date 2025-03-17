cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project (MovementCompensationExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if(UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
else(UNIX)
    add_compile_options(/MP)
endif(UNIX)

set(Data_DIR "${MovementCompensationExample_SOURCE_DIR}/Data")

if (PHO_DOWNLOAD_API_EXAMPLES_DATA)
    set(DATA_URL "https://photoneo.com/files/installer/PhoXi/api/movement_compensation_example_1.0.zip")
    set(DATA_DOWNLOAD_PATH "${MovementCompensationExample_BINARY_DIR}/movement_compensation_example_1.0.zip")
    
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
            WORKING_DIRECTORY "${MovementCompensationExample_BINARY_DIR}")
    endif()
endif()

if (EXISTS ${Data_DIR} AND NOT EXISTS ${MovementCompensationExample_BINARY_DIR}/Data)
    file(COPY ${Data_DIR}/
    DESTINATION ${MovementCompensationExample_BINARY_DIR}/Data/)
endif()

if(NOT PhoXiAPI_ExampleUtils_DIR)
    set(PhoXiAPI_ExampleUtils_DIR "${MovementCompensationExample_SOURCE_DIR}/Utils")
endif()

set(SOURCE_LIST
    ${MovementCompensationExample_SOURCE_DIR}/Main.cpp
    ${MovementCompensationExample_SOURCE_DIR}/ReadMe.txt
)

set(PhoXiAPI_ExampleUtils_LIST
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

add_executable (MovementCompensationExample ${SOURCE_LIST} ${PhoXiAPI_ExampleUtils_LIST})

# Create the source groups for source tree with root at CMAKE_CURRENT_SOURCE_DIR.
source_group(TREE ${PhoXiAPI_ExampleUtils_DIR}/ PREFIX "Utils" FILES ${PhoXiAPI_ExampleUtils_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(MovementCompensationExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

get_target_property(ReleaseBinFolder MovementCompensationExample RUNTIME_OUTPUT_DIRECTORY_RELEASE)
get_target_property(DebugBinFolder MovementCompensationExample RUNTIME_OUTPUT_DIRECTORY_DEBUG)

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

if (PHO_DOWNLOAD_API_EXAMPLES_DATA)
    add_custom_command(TARGET MovementCompensationExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            original.praw
            $<TARGET_FILE_DIR:MovementCompensationExample>/Data/original.praw
    )
    add_custom_command(TARGET MovementCompensationExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            velocity.txt
            $<TARGET_FILE_DIR:MovementCompensationExample>/Data/velocity.txt
    )
endif()

target_include_directories(MovementCompensationExample PUBLIC ${PHOXI_INCLUDE_DIRS})

set_target_properties(MovementCompensationExample
    PROPERTIES
    #for executables, inheritance of variables CMAKE_DEBUG_POSTFIX... does not work
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX}
)



