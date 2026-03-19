/**
 * @file CommandTests.cpp
 * @brief FTP 各项命令的单元测试
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ftpd/FTPServer.h"
#include "ftpd/FTPConnection.h"
#include "ftpd/commands/AuthCommands.h"
#include "ftpd/commands/DirectoryCommands.h"
#include "ftpd/commands/FileCommands.h"
#include "ftpd/commands/TransferCommands.h"
#include "ftpd/commands/MiscCommands.h"
#include <ace/Reactor.h>
#include <ace/SOCK_Connector.h>
#include <ace/INET_Addr.h>
#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_unistd.h>
#include <fstream>
#include <cstdio>

using namespace ftpd;
using testing::_;
using testing::Return;

/**
 * @brief 模拟 FTPServer 以自定义认证行为
 */
class MockFTPServer : public FTPServer {
public:
    MOCK_CONST_METHOD2(authenticate, bool(const std::string& username, const std::string& password));
};

/**
 * @brief 模拟 FTPConnection 以捕获发送的响应
 */
class MockFTPConnection : public FTPConnection {
public:
    MOCK_METHOD2(send_response, int(int code, const std::string& message));
    MOCK_METHOD1(send_response, int(const FTPResponse& response));
    virtual int handle_close(ACE_HANDLE fd, ACE_Reactor_Mask mask) override { return 0; }
};

/**
 * @brief 命令测试基类，负责环境搭建
 */
class CommandTest : public ::testing::Test {
protected:
    void SetUp() override {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            test_root = std::string(cwd) + "/test_data_root_cmd";
        } else {
            test_root = "test_data_root_cmd";
        }
        ACE_OS::mkdir(test_root.c_str());
        
        mock_server = new MockFTPServer();
        mock_conn = new MockFTPConnection();
        mock_conn->set_server(mock_server);
        mock_conn->set_filesystem(new FileSystem(test_root));
    }

    void TearDown() override {
        delete mock_conn;
        delete mock_server;
        std::string cmd = "rm -rf " + test_root;
        system(cmd.c_str());
    }

    std::string test_root;
    MockFTPServer* mock_server;
    MockFTPConnection* mock_conn;
};

/**
 * @brief 用户认证相关命令测试 (USER, PASS)
 */
TEST_F(CommandTest, AuthCommandsTest) {
    UserCommand user_cmd;
    PassCommand pass_cmd;

    EXPECT_CALL(*mock_conn, send_response(331, _)).Times(1);
    user_cmd.execute(mock_conn, "testuser");
    EXPECT_EQ(mock_conn->get_username(), "testuser");
    EXPECT_EQ(mock_conn->get_auth_state(), AuthState::WAITING_FOR_PASS);

    EXPECT_CALL(*mock_server, authenticate("testuser", "wrongpass")).WillOnce(Return(false));
    EXPECT_CALL(*mock_conn, send_response(530, _)).Times(1);
    pass_cmd.execute(mock_conn, "wrongpass");
    EXPECT_EQ(mock_conn->get_auth_state(), AuthState::WAITING_FOR_USER);

    mock_conn->set_username("testuser");
    mock_conn->set_auth_state(AuthState::WAITING_FOR_PASS);
    EXPECT_CALL(*mock_server, authenticate("testuser", "testpass")).WillOnce(Return(true));
    EXPECT_CALL(*mock_conn, send_response(230, _)).Times(1);
    pass_cmd.execute(mock_conn, "testpass");
    EXPECT_EQ(mock_conn->get_auth_state(), AuthState::AUTHENTICATED);
}

/**
 * @brief 目录操作命令测试 (PWD, CWD, MKD, RMD)
 */
TEST_F(CommandTest, DirectoryCommandsTest) {
    PwdCommand pwd_cmd;
    CwdCommand cwd_cmd;
    MkdCommand mkd_cmd;
    RmdCommand rmd_cmd;

    mock_conn->set_auth_state(AuthState::AUTHENTICATED);

    // 1. PWD
    EXPECT_CALL(*mock_conn, send_response(257, _)).Times(1);
    pwd_cmd.execute(mock_conn, "");

    // 2. MKD
    EXPECT_CALL(*mock_conn, send_response(257, _)).Times(1);
    mkd_cmd.execute(mock_conn, "subdir");
    struct stat st;
    EXPECT_EQ(ACE_OS::stat((test_root + "/subdir").c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));

    // 3. CWD
    EXPECT_CALL(*mock_conn, send_response(250, _)).Times(1);
    cwd_cmd.execute(mock_conn, "subdir");
    EXPECT_EQ(mock_conn->get_work_dir(), "/subdir");

    // 4. RMD
    mock_conn->set_work_dir("/");
    EXPECT_CALL(*mock_conn, send_response(250, _)).Times(1);
    rmd_cmd.execute(mock_conn, "subdir");
    EXPECT_NE(ACE_OS::stat((test_root + "/subdir").c_str(), &st), 0);
}

/**
 * @brief 文件操作命令测试 (DELE, SIZE, TYPE)
 */
TEST_F(CommandTest, FileCommandsTest) {
    DeleCommand dele_cmd;
    SizeCommand size_cmd;
    TypeCommand type_cmd;

    mock_conn->set_auth_state(AuthState::AUTHENTICATED);
    std::string filename = "test.txt";
    std::ofstream ofs(test_root + "/" + filename);
    ofs << "Hello World";
    ofs.close();

    // 1. TYPE
    EXPECT_CALL(*mock_conn, send_response(200, _)).Times(2);
    type_cmd.execute(mock_conn, "I");
    EXPECT_EQ(mock_conn->get_transfer_mode(), TransferMode::BINARY);
    type_cmd.execute(mock_conn, "A");
    EXPECT_EQ(mock_conn->get_transfer_mode(), TransferMode::ASCII);

    // 2. SIZE
    EXPECT_CALL(*mock_conn, send_response(213, _)).Times(1);
    size_cmd.execute(mock_conn, filename);

    // 3. DELE
    EXPECT_CALL(*mock_conn, send_response(250, _)).Times(1);
    dele_cmd.execute(mock_conn, filename);
    struct stat st;
    EXPECT_NE(ACE_OS::stat((test_root + "/" + filename).c_str(), &st), 0);
}

/**
 * @brief 传输及杂项命令测试 (PASV, QUIT)
 */
TEST_F(CommandTest, MiscCommandsTest) {
    PasvCommand pasv_cmd;
    QuitCommand quit_cmd;

    mock_conn->set_auth_state(AuthState::AUTHENTICATED);

    EXPECT_CALL(*mock_conn, send_response(227, _)).Times(1);
    pasv_cmd.execute(mock_conn, "");
    EXPECT_NE(mock_conn->get_data_connection(), nullptr);

    EXPECT_CALL(*mock_conn, send_response(221, _)).Times(1);
    quit_cmd.execute(mock_conn, "");
}
