# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)

# Project's name


project(hello)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LEPTONICA_PATH "/usr/include/leptonica")
set(TESSERACT_LOCAL_PATH "/usr/local/include/tesseract")
set(TESSERACT_PATH "/usr/include/tesseract")
set(PREPROCESSING_PATH ${PROJECT_SOURCE_DIR}/src/preprocessing)
set(PROCESS_PATH ${PROJECT_SOURCE_DIR}/src/ocr_process)
set(PREPARATION_PATH ${PROJECT_SOURCE_DIR}/src/ocr_process)

include_directories(${LEPTONICA_PATH})
include_directories(${TESSERACT_LOCAL_PATH})
include_directories(${TESSERACT_PATH})
include_directories(${PREPROCESSING_PATH})
include_directories(${PROCESS_PATH})
include_directories(${PREPARATION_PATH})

add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
find_package(ImageMagick COMPONENTS Magick++)

include_directories(${ImageMagick_INCLUDE_DIRS})
message(${ImageMagick_INCLUDE_DIRS})

add_executable(hello ${PROJECT_SOURCE_DIR}/main.cpp ${PROJECT_SOURCE_DIR}/src/preparation/parser.cpp ${PROJECT_SOURCE_DIR}/src/ocr_process/process.cpp)

target_link_libraries(hello "/usr/lib/x86_64-linux-gnu/liblept.so")
target_link_libraries(hello "/usr/lib/x86_64-linux-gnu/libtesseract.so")
target_link_libraries(hello ${ImageMagick_LIBRARIES})

