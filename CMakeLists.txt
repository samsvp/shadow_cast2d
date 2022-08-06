cmake_minimum_required(VERSION 3.15)
project(shadow_cast)

find_package(raylib 3.0 REQUIRED) # Requires at least version 3.0
find_package(glfw3 3.3 REQUIRED)


set(CMAKE_C_STANDARD 11) # Requires C11 standard

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} raylib glfw dl pthread)

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()