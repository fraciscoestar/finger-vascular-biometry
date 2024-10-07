# Header list
set(HEADER_LIST 
    ../include/finger_vascular_biometry/biometry.hpp 
    ../include/finger_vascular_biometry/DBHandler.hpp
)

# Source list
set(SOURCE_LIST 
    biometry.cpp
    DBHandler.cpp
)

# Add source code specific to Raspberry Pi
if (BCMHOST)
    list(APPEND SOURCE_LIST cvprog.cpp)
endif()

# Create the library
add_library(FINGER_VASCULAR_BIOMETRY_LIB
            ${SOURCE_LIST}
            ${HEADER_LIST})

# Include directories
target_include_directories(FINGER_VASCULAR_BIOMETRY_LIB PUBLIC
                            ../include
                            ${PROJECT_BINARY_DIR}/include
                            ${OpenCV_INCLUDE_DIRS}
                            ${SQLite3_INCLUDE_DIRS})

# Link libraries
target_link_libraries(FINGER_VASCULAR_BIOMETRY_LIB PUBLIC
                        ${OpenCV_LIBS}
                        ${SQLite3_LIBRARIES})

# Specify to compile target with C++17
target_compile_features(FINGER_VASCULAR_BIOMETRY_LIB PUBLIC cxx_std_17)
