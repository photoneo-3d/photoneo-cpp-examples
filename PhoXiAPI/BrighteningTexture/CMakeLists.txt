cmake_minimum_required (VERSION 3.10)

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

project(BrighteningTextureExample)

set(CMAKE_RELEASE_POSTFIX "_Release")
set(CMAKE_DEBUG_POSTFIX "_Debug")

if (UNIX)
    add_compile_options(-std=c++1y)
    add_compile_options(-pthread)
endif(UNIX)

set(Files
    ${BrighteningTextureExample_SOURCE_DIR}/BrighteningTextureExample.cpp
    ${BrighteningTextureExample_SOURCE_DIR}/ReadMe.txt
)

add_executable(BrighteningTextureExample
    ${Files}
)

if (NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(BrighteningTextureExample
    ${PHOXI_LIBRARY}
    $<$<PLATFORM_ID:Linux>:rt>
)

if(MSVC)
    if(NOT PHOXI_DLL_FOR_EXAMPLE)
        set(PHOXI_DLL_FOR_EXAMPLE ${PHOXI_DLL})
    endif(NOT PHOXI_DLL_FOR_EXAMPLE)
    add_custom_command(TARGET BrighteningTextureExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PHOXI_DLL_FOR_EXAMPLE}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

target_include_directories(BrighteningTextureExample PUBLIC
    ${PHOXI_INCLUDE_DIRS}
)
