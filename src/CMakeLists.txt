# 收集所有源文件
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(GLOB COMMAND_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp"
)

set(ALL_SOURCES ${SOURCES} ${COMMAND_SOURCES})

if(NOT ALL_SOURCES)
    message(WARNING "No source files found in src/")
    return()
endif()

# 创建静态库供主程序和测试共用
add_library(ftpd_lib STATIC ${ALL_SOURCES})
target_include_directories(ftpd_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${ACE_INCLUDE_DIR}
)
target_link_libraries(ftpd_lib PUBLIC
    ${ACE_LIBRARY}
    pthread
)

# 添加可执行文件
add_executable(ftpd main.cpp)
target_link_libraries(ftpd ftpd_lib)

# 设置输出目录
set_target_properties(ftpd PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)