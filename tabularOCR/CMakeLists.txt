cmake_minimum_required(VERSION 3.8)

project(tabularOCR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

set(output_dir ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir})

if (MSVC)
    #on windows use the cppan to create .cppan directory
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    find_package(CPPAN REQUIRED)
    cppan_add_package(
        pvt.cppan.demo.google.tesseract.libtesseract
        pvt.cppan.demo.danbloomberg.leptonica
    )
    cppan_execute()
else()
    #on unix, install the following libraries:
    #libleptonica-dev libtesseract-dev libopencv-dev
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    find_package(PkgConfig)
    pkg_check_modules(lept REQUIRED lept)
    pkg_check_modules(tesseract REQUIRED tesseract)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include( external_json )
include_directories( ${JSON_INCLUDE_DIRS} )

set(SOURCES
    ${PROJECT_SOURCE_DIR}/main.cpp 
    ${PROJECT_SOURCE_DIR}/src/preprocessing/parser.cpp 
	${PROJECT_SOURCE_DIR}/src/ocr_process/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/ocr_process/process.cpp
	${PROJECT_SOURCE_DIR}/src/preprocessing/preprocess.cpp)

add_executable(tabularOCR ${SOURCES})

if (MSVC)
    target_link_libraries(tabularOCR
        pvt.cppan.demo.google.tesseract.libtesseract
        pvt.cppan.demo.danbloomberg.leptonica
    )
else()
    target_link_libraries(tabularOCR lept tesseract stdc++fs)
endif()