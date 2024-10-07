# Create configuration file to pass BCM_HOST
configure_file(${PROJECT_SOURCE_DIR}/include/security_system/config.h.in
               ${PROJECT_BINARY_DIR}/include/security_system/config.h)

# Create security system executable
add_executable(security_system 
    security_system.cpp
    ${PROJECT_SOURCE_DIR}/include/security_system/config.h.in
    ../include/security_system/security_system.hpp)

# Include directories
target_include_directories(security_system PUBLIC
                            ../include
                            ${PROJECT_BINARY_DIR}/include
                            ${CMAKE_CURRENT_BINARY_DIR}/security_system_autogen/include)

# Link libraries
target_link_libraries(security_system PUBLIC
                        FINGER_VASCULAR_BIOMETRY_LIB
                        Qt5::Core
                        Qt5::Gui
                        Qt5::Widgets)

# Specify to compile with C++17
target_compile_features(security_system PRIVATE cxx_std_17)

if(WIN32)
    set_target_properties(security_system PROPERTIES WIN32_EXECUTABLE ON)
endif()

# Copy resources
add_custom_command(TARGET security_system POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/apps/resources)

# Create camera test executable if RP system
if (BCMHOST)
    add_executable(camera_test camera_test.cpp)

    # Include directories
    target_include_directories(camera_test PUBLIC
                                ${OpenCV_INCLUDE_DIRS})

    # Link libraries
    target_link_libraries(camera_test PUBLIC
                        ${OpenCV_LIBS})

    # Specify to compile with C++17
    target_compile_features(camera_test PRIVATE cxx_std_17)

    # Add compile flag to include wiringpi (also adds to security_system)
    add_definitions(-lwiringPi)
endif()
