# Seewo Offline QR Scanner

`注意: 本程序仅在V2版本进行过测试,V3是否适用未知.`

## 简介

`Seewo Offline QR Scanner` 是一个基于 C++ 的应用程序，能够从计算机屏幕上捕获Seewo的激活码解锁二维码并扫描获取激活码。该程序使用 `zbar` 库进行二维码识别，使用 `libcrypto`和 `libcryptagram` 库进行解密处理。通过分析激活码解锁二维码 URL 中的参数，程序可以解密并在屏幕左上角显示激活码。

## 依赖项

本项目使用的预构建的二进制库文件：

- `zbar`
- `OpenSSL libcrypto`

需要的工具:

- `i686-w64-mingw32` 用于交叉编译 (Windows需要MinGW工具链)
- `make` (用于操作Makefile)
- `upx` & `zip` (用于打包可执行文件)

## 文件结构

```text
(root)
├── Makefile               # Makefile 文件
├── qrScan.cpp             # 主文件
├── crypto.cc              # (CryptoJS兼容)AES解密相关功能实现
├── base64.cc              # Base64 编码/解码实现
├── run.bat                # 启动脚本 (wapper)
├── libcrypto-3.dll        # libcrypto 库 (预构建)
├── libzbar-0.dll          # libzbar 库 (预构建)
├── openssl/include/       # OpenSSL 头文件
│    └── ...
├── zbar/include/          # ZBar 头文件
│    └── ...
├── base/                  # CryptoJS兼容算法实现(头文件)
│    ├── ...
│    └── memory/
│        └── scoped_ptr.h   # scoped ptr实现
├── libiconv-2.dll         # libiconv 库 (预构建)
└── README.md              # 说明文件
```

## 构建项目

要构建项目，请确保您在项目根目录下，然后运行以下命令：

```shell
make
```

这将创建 `build/qrcodeScanner.exe` 可执行文件。

您可以通过运行以下命令来压缩可执行文件、复制依赖项并将项目打包成 zip 文件：

```shell
make pack
```

这将创建 `qrcodeScanner_release.zip`，其中包含压缩后的可执行文件和所有必要的库。

要清理项目并删除构建生成的文件，可以运行以下命令：

```shell
make clean
```

您可以使用 `wine` 来测试生成的可执行文件（在 Linux 上运行,需要先打包）：

```shell
make test
```

## 使用说明

1. 运行wapper或添加到开机启动项 `run.bat`。
2. 程序将每隔500ms捕获一次屏幕并尝试扫描激活码解锁的 QR 码。
3. 如果识别到 QR 码，将解密并在屏幕左上角显示激活码约10s。

## 贡献

欢迎任何贡献！如果您发现错误或有改进建议，请提出问题或提交请求。

## 许可证

该项目使用 MIT 许可证，详细信息请查看 LICENSE 文件。

## 开发者

开发者：[Steve3184](https://github.com/Steve3184)
