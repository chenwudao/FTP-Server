/**
 * @file test_main.cpp
 * @brief Google Test 示例测试
 * @author Junxi Chen
 * @date 2026-03-03
 */

#include <gtest/gtest.h>
#include "ftpd/Types.h"

/**
 * @brief 测试 FTPResponse 结构
 */
TEST(FTPResponseTest, BasicConstruction) {
    ftpd::FTPResponse resp(220, "Welcome");
    EXPECT_EQ(resp.code, 220);
    EXPECT_EQ(resp.message, "Welcome");
}

/**
 * @brief 测试 AuthState 枚举
 */
TEST(AuthStateTest, EnumValues) {
    ftpd::AuthState state = ftpd::AuthState::WAITING_FOR_USER;
    EXPECT_EQ(state, ftpd::AuthState::WAITING_FOR_USER);
    
    state = ftpd::AuthState::AUTHENTICATED;
    EXPECT_EQ(state, ftpd::AuthState::AUTHENTICATED);
}

/**
 * @brief 测试 TransferMode 枚举
 */
TEST(TransferModeTest, EnumValues) {
    ftpd::TransferMode mode = ftpd::TransferMode::BINARY;
    EXPECT_EQ(mode, ftpd::TransferMode::BINARY);
    
    mode = ftpd::TransferMode::ASCII;
    EXPECT_EQ(mode, ftpd::TransferMode::ASCII);
}

// 后续添加更多测试：
// - test USER/PASS 命令逻辑
// - test PWD/CWD 路径操作
// - test PASV 端口分配
// - test LIST/RETR/STOR 数据传输
// 等等...

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
