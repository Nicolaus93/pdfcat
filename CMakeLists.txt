cmake_minimum_required(VERSION 3.0)
project(ImageToAscii)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Attempt to find Poppler using pkg-config
find_package(PkgConfig)
pkg_search_module(POPPLER_CPP QUIET poppler-cpp)

if(POPPLER_CPP_FOUND)
    message(STATUS "Poppler found: ${POPPLER_CPP_LIBRARIES}")
else()
    message(FATAL_ERROR "Poppler not found: Please check your Poppler installation.")
endif()

# Add executable
add_executable(image_to_ascii main.cpp)

# Include directories for OpenCV and Poppler
include_directories(${OpenCV_INCLUDE_DIRS} ${POPPLER_CPP_INCLUDE_DIRS})

# Link libraries
target_link_libraries(image_to_ascii ${OpenCV_LIBS} ${POPPLER_CPP_LIBRARIES})
