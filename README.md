# ftpd

基于 ACE Reactor 的 C++11 FTP 服务器，实现 [RFC 959](https://www.rfc-editor.org/rfc/rfc959) 的基本命令子集。数据连接使用被动模式（`PASV`）。文件大小命令 `SIZE` 遵循 [RFC 3659](https://www.rfc-editor.org/rfc/rfc3659)。

## 已实现的命令

| 命令 | 作用 |
| :--- | :--- |
| `USER` | 接收用户名 |
| `PASS` | 校验密码 |
| `PWD` | 返回当前工作目录 |
| `CWD` | 切换工作目录 |
| `MKD` | 创建目录 |
| `RMD` | 删除目录 |
| `DELE` | 删除文件 |
| `SIZE` | 返回文件大小 |
| `TYPE` | 设置传输类型：`A`（ASCII）或 `I`（二进制） |
| `PASV` | 打开被动模式数据端口 |
| `LIST` | 列出目录（需先执行 `PASV`） |
| `RETR` | 下载文件（需先执行 `PASV`） |
| `STOR` | 上传文件（需先执行 `PASV`） |
| `QUIT` | 结束会话 |

## 构建

编译需要 C++ 编译器、CMake 3.10 或更高版本，以及 ACE（`libace-dev`）。生成 API 文档需要 Doxygen。单元测试在 CMake 配置阶段通过 FetchContent 获取 Google Test 1.12.1，首次配置需要访问 GitHub。

```bash
sudo apt install build-essential cmake libace-dev doxygen
mkdir build && cd build
cmake ..
cmake --build . -- -j$(nproc)
```

生成 API 文档（输出为仓库内 `docs/doxygen/html/index.html`）：

```bash
cd build
cmake --build . --target doc
```

运行单元测试：

```bash
cd build
./bin/ftpd_test
```

## 运行

在 `build` 目录中启动，并使用仓库自带的示例根目录与密码文件：

```bash
cd build
./bin/ftpd -p 2121 -r ../data/admin -u ../ftpd.passwd
```

`Ctrl+C` 结束进程。

| 选项 | 说明 |
| :--- | :--- |
| `-p <port>` | 监听端口，默认 `21`。绑定 1024 以下端口需要相应权限，普通用户请使用更高端口 |
| `-r <root_dir>` | FTP 根目录，默认进程当前目录 |
| `-u <passwd_file>` | 用户密码文件。省略时拒绝所有登录；文件无法读取时进程启动失败 |
| `-h` | 打印上述选项 |

### 密码文件

每行一名用户，格式为明文 `username:password`。空行以及以 `#` 开头的行会被忽略。

仓库中的 `ftpd.passwd`：

```text
admin:admin123
```

### 使用客户端连接

标准 `ftp` 客户端：

```bash
ftp localhost 2121
```

使用 `ftpd.passwd` 中的账户登录：用户名 `admin`，密码 `admin123`。

| 操作 | 客户端输入 | 服务器命令 |
| :--- | :--- | :--- |
| 切换目录 | `cd <path>` | `CWD <path>` |
| 当前目录 | `pwd` | `PWD` |
| ASCII 模式 | `ascii` | `TYPE A` |
| 二进制模式 | `binary` | `TYPE I` |
| 列出目录 | `ls` 或 `dir` | `PASV`，然后 `LIST` |
| 下载 | `get <file>` | `PASV`，然后 `RETR <file>` |
| 上传 | `put <file>` | `PASV`，然后 `STOR <file>` |
| 删除文件 | `delete <file>` | `DELE <file>` |
| 创建目录 | `mkdir <path>` | `MKD <path>` |
| 删除目录 | `rmdir <path>` | `RMD <path>` |
| 文件大小 | `quote SIZE <file>` | `SIZE <file>` |
| 发送任意已实现命令 | `quote <CMD>` | `<CMD>` |

FileZilla 等图形客户端请使用被动模式：

- 主机：`127.0.0.1`
- 端口：`2121`
- 用户名：`admin`
- 密码：`admin123`

### 会话示例

下列应答与服务器实现一致。`PASV` 中的地址和端口由当次监听结果决定，端口等于 `p1 * 256 + p2`。目录列表采用 Unix `ls -l` 风格，属主与时间随系统变化。示例文件 `data/admin/hello.txt` 为 16 字节。

```text
$ ftp localhost 2121
Connected to localhost.
220 Welcome to ftpd FTP server
Name (localhost:user): admin
331 User name okay, need password
Password:
230 User logged in, proceed
ftp> pwd
257 "/"
ftp> binary
200 Type set to I
ftp> ls
227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
150 Opening data connection
-rw-r--r-- 1 owner group 16 Mar 03 11:00 hello.txt
226 Transfer complete
ftp> quit
221 Goodbye
```

## 目录结构

```text
.
├── CMakeLists.txt
├── README.md
├── ftpd.passwd                 # 示例密码文件
├── data/admin/hello.txt        # 示例文件
├── include/ftpd/
│   ├── Types.h                 # 公共类型
│   ├── ICommand.h              # 命令接口
│   ├── FTPServer.h             # 监听与认证
│   ├── FTPConnection.h         # 控制连接
│   ├── DataConnection.h        # PASV 数据连接
│   ├── FileSystem.h            # 根目录内的文件操作
│   └── commands/
│       ├── AuthCommands.h      # USER, PASS
│       ├── DirectoryCommands.h # PWD, CWD, MKD, RMD
│       ├── FileCommands.h      # DELE, SIZE, TYPE
│       ├── TransferCommands.h  # PASV, LIST, RETR, STOR
│       └── MiscCommands.h      # QUIT
├── src/                        # 对应实现与 main.cpp
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp
│   └── CommandTests.cpp
└── docs/
    ├── Doxyfile
    └── doxygen/                # Doxygen 输出
```

## 技术栈

- C++11
- ACE Reactor
- CMake 3.10 或更高版本
- Google Test 1.12.1（配置时自动获取）
- Doxygen

## 实现要点

1. 控制连接由 ACE Reactor 的事件循环接入，同一进程可同时服务多个客户端。
2. 每条 FTP 命令实现 `ICommand`，按命令名分发。
3. 虚拟路径会折叠 `.` 与 `..`，并限制在 `-r` 指定的根目录之内。
4. 数据连接只提供 `PASV`。`LIST`、`RETR` 与 `STOR` 在数据连接建立前采用短时等待。
5. 公共头文件带有 Doxygen 注释。

## 后续工作

- 为 `LIST`、`RETR`、`STOR` 补充单元测试（`USER`、`PASS`、`PWD`、`CWD`、`MKD`、`RMD`、`DELE`、`SIZE`、`TYPE`、`PASV`、`QUIT` 已有测试）
- 增加日志记录
- 将数据传输改为完全由 Reactor 驱动的异步处理
- 增加配额与限速
- 支持 TLS/SSL（FTPS）
- 增加持续集成配置

## 许可证

仓库尚未包含许可证。

## 作者

ftpd team，2026
