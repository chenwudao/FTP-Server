cmake_minimum_required(VERSION 3.10)
project(ftpd VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找 ACE 库
find_path(ACE_INCLUDE_DIR NAMES ace/ACE.h PATHS /usr/include /usr/local/include)
find_library(ACE_LIBRARY NAMES ACE ACE-6.4.5 PATHS /usr/lib /usr/local/lib)

if(NOT ACE_INCLUDE_DIR OR NOT ACE_LIBRARY)
    message(FATAL_ERROR "ACE library not found. Please install libace-dev.")
endif()

message(STATUS "Found ACE includes: ${ACE_INCLUDE_DIR}")
message(STATUS "Found ACE library: ${ACE_LIBRARY}")

# 添加子目录
add_subdirectory(src)
add_subdirectory(tests)

# Doxygen 支持
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
