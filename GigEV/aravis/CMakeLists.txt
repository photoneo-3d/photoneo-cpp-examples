cmake_minimum_required(VERSION 3.16)
project(aravisExamples)

include(common/helper_functions.cmake)

generate_example_app(ConnectAndGrabC
    SOURCES
        ConnectAndGrab-C/main.c
)

generate_example_app(ComponentSelector
    SOURCES
        ComponentSelector/main.cpp
)

generate_example_app(ConnectAndGrab
    SOURCES
        ConnectAndGrab/main.cpp
)

#YCoCg conversion uses OpenCV
find_package(OpenCV COMPONENTS core highgui imgproc)
message("OpenCV_LIBS = ${OpenCV_LIBS}")
if(OpenCV_FOUND)
    generate_example_app(ConnectAndGrab-ColorTexture
        SOURCES
            ConnectAndGrab-ColorTexture/main.cpp
        LINK_LIBS
            opencv_core
            opencv_highgui
            opencv_imgproc
    )
else()
    message(WARNING "OpenCV not found! It is a requirement for ConnectAndGrab-ColorTexture example.")
endif()

generate_example_app(ConnectAndGrab-SWTrigger
    SOURCES
        ConnectAndGrab-SwTrigger/main.cpp
)

generate_example_app(GenICamSettings
    SOURCES
        GenICamSettings/main.cpp
        GenICamSettings/ReadWriteHelpers.cpp
        GenICamSettings/ReadWriteHelpers.h
)

generate_example_app(IPSettings
    SOURCES
        IPSettings/main.cpp
)

generate_example_app(UserSets
    SOURCES
        UserSets/main.cpp
)
