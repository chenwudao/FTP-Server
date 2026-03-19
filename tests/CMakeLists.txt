# 使用 FetchContent 自动下载并构建 Google Test
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
)

# For Windows: 防止重载父项目的编译器/连接器设置
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# 定义测试源文件
set(TEST_SOURCES
    test_main.cpp
    CommandTests.cpp
    # 其他测试文件
)

# 仅在有测试源文件时才构建
if(TEST_SOURCES)
    add_executable(ftpd_test ${TEST_SOURCES})
    
    target_include_directories(ftpd_test PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${ACE_INCLUDE_DIR}
    )
    
    target_link_libraries(ftpd_test PRIVATE
        ftpd_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        ${ACE_LIBRARY}
        pthread
    )
    
    # 注册测试
    gtest_discover_tests(ftpd_test)
else()
    message(STATUS "No test sources found, skipping test build")
endif()