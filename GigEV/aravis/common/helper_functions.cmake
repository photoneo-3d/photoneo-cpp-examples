cmake_minimum_required(VERSION 3.16)

macro(find_aravis_dependencies)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(aravis_deps REQUIRED
        IMPORTED_TARGET
            aravis-0.8
            glib-2.0
            gobject-2.0
    )
endmacro()

find_aravis_dependencies()

# Usage:
# generate_example_app(<target_name>
#     SOURCES
#         <source_file1>.cpp
#         <header_file1>.h
#         <source_file2>.cpp
#         <header_file2>.h
#     INCLUDE_DIRS
#         <some_include_directory>
#     LINK_LIBS
#         <some_library>
#         <lib::lib2>
# )
function(generate_example_app TARGET)
    set(OPTIONS)
    set(ONE_VALUE)
    set(MULTI_VALUE SOURCES INCLUDE_DIRS LINK_LIBS)
    cmake_parse_arguments(MY "${OPTIONS}" "${ONE_VALUE}" "${MULTI_VALUE}" ${ARGN})

    if(MY_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments ${MY_UNPARSED_ARGUMENTS}")
    endif()

    set(CMAKE_CXX_STANDARD 17)

    add_executable(${TARGET}
        ${MY_SOURCES}
    )

    target_include_directories(${TARGET}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${MY_INCLUDE_DIRS}
    )

    target_link_libraries(${TARGET}
        PRIVATE
            PkgConfig::aravis_deps
            ${MY_LINK_LIBS}
    )
endfunction()
