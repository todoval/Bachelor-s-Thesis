# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name


project(hello)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LEPTONICA_PATH "/usr/include/leptonica")
set(TESSERACT_LOCAL_PATH "/usr/local/include/tesseract")
set(TESSERACT_PATH "/usr/include/tesseract")
#set(CMAKE_CXX_FLAGS "-I /usr/local/include/tesseract -I /usr/include/leptonica -I /usr/include/tesseract")
include_directories(${LEPTONICA_PATH})
include_directories(${TESSERACT_LOCAL_PATH})
include_directories(${TESSERACT_PATH})
add_executable(hello ${PROJECT_SOURCE_DIR}/main.cpp)
target_link_libraries(hello "/usr/lib/x86_64-linux-gnu/liblept.so")
target_link_libraries(hello "/usr/lib/x86_64-linux-gnu/libtesseract.so")
#target_link_libraries(hello "/usr/local/lib/libtesseract.a")
