cmake_minimum_required(VERSION 3.15)

# Set the project name
project(FINGER_VASCULAR_BIOMETRY VERSION 1.0.0
                                 DESCRIPTION "An open-source vascular biometry implementation"
                                 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Check if system is a RPi
    find_file(BCMHOST bcm_host.h PATHS /opt/vc/include/)
    if (BCMHOST)
        message(STATUS "Detected system as a Raspberry Pi")
    else()
        message(STATUS "Detect system as not a Raspberry Pi")
    endif()

    # Build type options - force 'Release' if none is selected
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build configurations" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()

    # Specify the C++17 standard and ensure using -std=c++17 instead of -std=g++17
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Enable MOC, RCC & UIC
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

    # Set -fPIC flag
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Find Qt5 library
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

# Find OpenCV Libraries
find_package(OpenCV REQUIRED)

# Find SQLite3 library
find_package(SQLite3 REQUIRED)

add_subdirectory(src)
add_subdirectory(apps)
