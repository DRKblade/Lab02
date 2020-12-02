cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Lab02 VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable(Lab02 main.cpp)
target_link_libraries( Lab02 ${OpenCV_LIBS} )

target_include_directories(Lab02 PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")

