# ftpd - 简易 FTP 服务器

基于 ACE Reactor 实现的 FTP 服务器，支持 RFC 959 基本命令。

## 功能特性

### ✅ 已实现的 FTP 命令

#### 认证命令
- **USER** - 接收用户名
- **PASS** - 验证密码

#### 目录操作
- **PWD** - 返回当前工作目录
- **CWD** - 改变工作目录
- **MKD** - 创建目录
- **RMD** - 删除目录

#### 文件操作
- **DELE** - 删除文件
- **SIZE** - 返回文件大小
- **TYPE** - 设置传输类型（ASCII/Binary）

#### 数据传输（被动模式）
- **PASV** - 进入被动模式
- **LIST** - 列出目录内容
- **RETR** - 下载文件
- **STOR** - 上传文件

#### 其他
- **QUIT** - 退出连接

## 构建

### 依赖
```bash
sudo apt install build-essential cmake libace-dev doxygen
```

### 编译
```bash
mkdir build && cd build
cmake ..
cmake --build . -- -j$(nproc)
```

### 生成文档
```bash
cd build
cmake --build . --target doc
# 文档位于 ../docs/doxygen/html/index.html
```

### 运行测试
```bash
cd build
./bin/ftpd_test
```

## 使用

### 启动服务器
```bash
cd build
./bin/ftpd -p 2121 -r /tmp/ftp -u ../ftpd.passwd
```

参数说明：
- `-p <port>` : 监听端口（默认 21，非 root 用户建议使用 >1024 的端口）
- `-r <root_dir>` : FTP 根目录（默认当前目录）
- `-u <passwd_file>` : 用户密码文件路径
- `-h` : 显示帮助

### 密码文件格式
文件 `ftpd.passwd` 格式（每行一个用户）：
```
username:password
anonymous:anonymous
testuser:testpass
```

### 使用 FTP 客户端连接

**1. 使用标准 ftp 命令行工具：**
```bash
ftp localhost 2121
# 输入用户名和密码： admin / admin123
```

**常用交互指令映射：**
| 功能 | 客户端输入 | 对应服务器协议命令 |
| :--- | :--- | :--- |
| **切换目录** | `cd <path>` | `CWD <path>` |
| **当前路径** | `pwd` | `PWD` |
| **二进制模式** | `binary` | `TYPE I` |
| **列出文件** | `ls` 或 `dir` | `PASV` + `LIST` |
| **下载文件** | `get <file>` | `PASV` + `RETR <file>` |
| **上传文件** | `put <file>` | `PASV` + `STOR <file>` |
| **删除文件** | `delete <file>`| `DELE <file>` |
| **直接发命令** | `quote <CMD>` | `<CMD>` (如 `quote MKD folder`) |

**2. 使用 FileZilla 等 GUI 客户端：**
- 主机：127.0.0.1
- 端口：2121
- 用户名：admin
- 密码：admin123
- 传输设置：**被动模式 (Passive)**

### 示例会话
```text
$ ftp localhost 2121
Connected to localhost.
220 Welcome to ftpd FTP server
Name (localhost:scutech): admin
331 User name okay, need password
Password: 
230 User logged in, proceed
ftp> pwd
257 "/"
ftp> binary
200 Type set to I
ftp> ls
227 Entering Passive Mode (127,0,0,1,162,13)
150 Opening data connection
-rw-r--r-- 1 user group 16 Mar 03 11:00 hello.txt
226 Transfer complete
ftp> quit
221 Goodbye
```

## 项目结构

```
ftpd/
├── CMakeLists.txt          # 顶层 CMake 配置
├── ftpd.passwd             # 示例密码文件
├── include/ftpd/           # 头文件
│   ├── Types.h             # 通用类型定义
│   ├── ICommand.h          # 命令接口
│   ├── FTPServer.h         # 服务器主类
│   ├── FTPConnection.h     # 客户端连接类
│   ├── DataConnection.h    # 数据连接类（PASV）
│   ├── FileSystem.h        # 文件系统操作
│   └── commands/           # 所有命令类
│       ├── AuthCommands.h
│       ├── DirectoryCommands.h
│       ├── FileCommands.h
│       ├── TransferCommands.h
│       └── MiscCommands.h
├── src/                    # 实现文件
│   ├── main.cpp            # 程序入口
│   ├── FTPServer.cpp
│   ├── FTPConnection.cpp
│   ├── DataConnection.cpp
│   ├── FileSystem.cpp
│   └── commands/           # 命令实现
├── tests/                  # 单元测试（使用 Google Test）
│   ├── CMakeLists.txt
│   └── test_main.cpp
└── docs/                   # 文档
    ├── Doxyfile            # Doxygen 配置
    ├── doxygen/            # 生成的 API 文档
    ├── 需求分析.textile
    ├── 概要设计.textile
    └── 详细设计.textile
```

## 技术栈

- **C++ 标准**：C++11
- **网络框架**：ACE (Adaptive Communication Environment) Reactor 模式
- **构建系统**：CMake 3.10+
- **单元测试**：Google Test (自动下载)
- **文档生成**：Doxygen

## 设计特点

1. **事件驱动**：基于 ACE Reactor 单线程异步 I/O，支持多客户端并发
2. **模块化设计**：命令模式实现，易于扩展新命令
3. **安全性**：路径规范化防止目录遍历攻击
4. **仅被动模式**：简化实现，符合现代 FTP 使用习惯（防火墙友好）
5. **完整注释**：所有公共 API 使用 Doxygen 格式注释

## 下一步工作

- [ ] 为所有命令编写单元测试
- [ ] 添加日志系统
- [ ] 优化数据传输（当前为阻塞等待，应改为完全异步）
- [ ] 添加配额和限速功能
- [ ] 支持 TLS/SSL (FTPS)
- [ ] 添加 CI/CD 配置

## 许可证

（待添加）

## 作者

ftpd team - 2026
