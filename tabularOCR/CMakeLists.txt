cmake_minimum_required(VERSION 3.8)

project(tabularOCR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

set(output_dir ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(ImageMagick REQUIRED COMPONENTS Magick++)
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
include_directories(${ImageMagick_INCLUDE_DIRS})

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp 
    ${PROJECT_SOURCE_DIR}/src/preparation/parser.cpp 
    ${PROJECT_SOURCE_DIR}/src/ocr_process/process.cpp)

add_subdirectory(.cppan)

add_executable(tabularOCR ${SOURCES})

target_link_libraries(tabularOCR
    pvt.cppan.demo.google.tesseract.libtesseract
    pvt.cppan.demo.danbloomberg.leptonica
    ${ImageMagick_LIBRARIES}
)