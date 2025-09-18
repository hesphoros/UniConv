# UniConv ✨

**一个现代化的跨平台C++字符编码转换库**

UniConv 是一个功能全面、高性能的 C++ 库，提供各种文本编码之间的无缝字符编码转换，包括 UTF-8、UTF-16LE/BE、GBK、GB2312、GB18030 以及系统本地编码。基于内嵌 GNU libiconv 实现，支持现代 CMake 构建系统，提供完整的自动化测试和开发工具链。

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/Version-2.0.0-blue.svg)](https://github.com/hesphoros/UniConv/releases)

---

## 📖 目录

1. [功能特性](#-功能特性)
2. [项目架构](#-项目架构)
3. [系统要求](#-系统要求)
4. [安装配置](#-安装配置)
5. [快速开始](#-快速开始)
6. [API参考](#-api参考)
7. [使用示例](#-使用示例)
8. [测试说明](#-测试说明)
9. [文档资源](#-文档资源)
10. [贡献指南](#-贡献指南)
11. [许可证](#-许可证)
12. [致谢](#-致谢)

---

## 🌟 功能特性

### 核心功能

- **🔄 双向编码转换**: 支持 UTF-8、UTF-16LE、UTF-16BE、GBK、GB2312、GB18030 以及 100+ 种其他编码
- **🌐 系统本地编码检测**: 自动检测并支持系统本地编码转换
- **⚡ 高性能处理**: 基于内嵌 GNU libiconv 库，提供快速可靠的转换
- **🔒 线程安全**: 线程安全的单例模式实现，支持多线程环境
- **🛡️ 错误处理**: 全面的错误检测和报告机制，详细的错误代码
- **📊 编码检测**: 内置编码检测和验证功能

### 现代化特性

- **CMake 构建系统**: 现代化的 CMake 3.16+ 构建配置，支持静态/动态库构建
- **🚀 自动化测试**: 完整的 CTest 集成和自动化测试套件
- **📦 自动部署**: 自动复制 DLL 和测试数据，开箱即用的开发体验
- **🎯 宽字符支持**: 原生支持 `std::wstring` 和宽字符操作
- **🔧 灵活的API**: 多种转换接口适应不同使用场景
- **📈 内存优化**: 优化的内存使用和智能缓冲区管理
- **📝 丰富文档**: 完整的 Doxygen 文档和使用示例

### 平台支持

- **🖥️ 跨平台**: 支持 Windows、Linux 和 macOS
- **🏗️ 现代C++**: 兼容 C++17 标准
- **🔨 构建系统**: 支持 Visual Studio 项目和现代 CMake

---

## 🏗️ 项目架构

UniConv 采用现代化的模块化架构，基于 CMake 构建系统：

```
UniConv/
├── CMakeLists.txt          # 现代 CMake 构建配置
├── README.MD               # 项目说明文档
├── .gitignore              # Git 忽略文件配置
│
├── include/                # 头文件目录
│   ├── UniConv.h           # 主要 API 接口头文件
│   ├── encodings.inc       # 编码定义文件
│   └── iconv/              # 内嵌 GNU libiconv 头文件
│       ├── iconv.h         # iconv 主头文件
│       ├── config.h        # 配置头文件
│       ├── encodings.def   # 编码定义
│       ├── aliases*.h      # 编码别名定义
│       ├── canonical*.h    # 标准编码映射
│       ├── utf*.h          # Unicode 编码实现
│       ├── gb*.h           # 中文编码实现
│       ├── iso8859*.h      # ISO 编码实现
│       └── ...             # 100+ 种编码实现头文件
│
├── src/                    # 源代码目录
│   ├── UniConv.cpp         # 核心转换引擎实现
│   ├── iconv.c             # GNU libiconv 核心实现
│   ├── localcharset.c      # 本地字符集检测
│   └── relocatable.c       # 路径重定位支持
│
├── test/                   # 测试代码目录
│   └── AllTests.cpp        # 完整测试套件
│
├── testdata/               # 测试数据目录
│   ├── input_*.txt         # 各种编码的输入测试文件
│   └── output/             # 测试输出文件目录
│
├── script/                 # 工具脚本目录
│   └── generate_test_files.py  # 测试文件生成脚本
│
├── doc/                    # 文档目录  
│   ├── 常见编码简介.md     # 编码基础知识
│   ├── Chinese编码(GBK等).md # 中文编码详解
│   ├── unicode.md          # Unicode 标准说明
│   └── iconv.md            # iconv 使用指南
│
├── cmake/                  # CMake 配置文件
│   └── UniConvConfig.cmake.in  # 包配置模板
│
└── .github/                # GitHub 配置
    └── workflows/          # CI/CD 工作流配置
        └── static.yml      # 静态分析工作流
```

### 核心组件

- **UniConv 类**: 主要转换引擎，基于内嵌 GNU libiconv 实现
- **CMake 构建系统**: 现代化构建配置，支持静态/动态库构建
- **内嵌 iconv**: 完整的 GNU libiconv 源码内嵌，无外部依赖
- **自动化测试**: CTest 集成的完整测试套件
- **智能部署**: 自动 DLL 复制和测试数据管理

---

## 📋 系统要求

### 系统要求
- **编译器**: Visual Studio 2019+ / GCC 7+ / Clang 6+
- **C++ 标准**: C++11 或更高版本
- **操作系统**: Windows 10+, Linux (Ubuntu 18.04+), macOS 10.14+

### 依赖库
- **GNU libiconv**: 字符集转换库（已包含）
- **标准库**: 除标准 C++ 库外无外部依赖

---

## 🚀 安装配置

### 方式一：现代 CMake 构建（推荐）

```bash
# 克隆仓库
git clone https://github.com/hesphoros/UniConv.git
cd UniConv

# 创建构建目录
mkdir build && cd build

# 配置项目（静态库，不构建测试）
cmake .. -DUNICONV_BUILD_SHARED=OFF -DUNICONV_BUILD_TESTS=OFF

# 或者配置动态库版本，包含测试
cmake .. -DUNICONV_BUILD_SHARED=ON -DUNICONV_BUILD_TESTS=ON

# 构建项目
cmake --build . --config Release

# 运行测试（如果启用了测试）
ctest --output-on-failure
```

### 方式二：Visual Studio 项目

```bash
# 克隆仓库后，直接在 Visual Studio 中打开
# 1. 打开 UniConv.sln
# 2. 选择构建配置（Debug/Release）
# 3. 构建解决方案
```

### 方式三：集成到现有项目

1. 使用 CMake `add_subdirectory`：

```cmake
add_subdirectory(UniConv)
target_link_libraries(your_target PRIVATE UniConv)
```

2. 或者复制源文件并手动链接：

```cpp
#include "UniConv.h"
// 无需额外链接，iconv 已内嵌
```

---

## ⚡ 快速开始

### 基本用法

```cpp
#include "UniConv.h"

int main() {
    // 获取单例实例
    auto converter = UniConv::GetInstance();
    
    // GBK 转 UTF-8
    std::string gbk_text = "你好，世界！";
    auto result = converter->ConvertEncoding(gbk_text, "GBK", "UTF-8");
    
    if (result.IsSuccess()) {
        std::cout << "UTF-8: " << result.conv_result_str << std::endl;
    } else {
        std::cerr << "转换失败: " << result.error_msg << std::endl;
    }
    
    return 0;
}
```

### 高级用法

```cpp
// 宽字符串转换
std::wstring wide_str = L"Hello, 世界！";
auto utf8_result = converter->ConvertEncoding(
    std::string(wide_str.begin(), wide_str.end()), 
    "WCHAR_T", "UTF-8"
);

// 系统本地编码检测
std::string current_encoding = converter->GetCurrentSystemEncoding();
std::cout << "当前系统编码: " << current_encoding << std::endl;

// 批量文件转换
auto gbk_to_utf8 = converter->ConvertEncoding(file_content, "GBK", "UTF-8");
```

---

## 📚 API参考

### 核心转换函数

#### 通用转换
```cpp
// 通用编码转换
IConvResult ConvertEncoding(const std::string& input, 
                           const char* fromEncoding, 
                           const char* toEncoding);
```

#### 系统编码
```cpp
// 获取当前系统编码
std::string GetCurrentSystemEncoding();

// 获取系统编码代码页
std::uint16_t GetCurrentSystemEncodingCodePage();
```

#### 常用编码常量
```cpp
// Unicode 编码
static constexpr const char* utf_8_encoding = "UTF-8";
static constexpr const char* utf_16le_encoding = "UTF-16LE";
static constexpr const char* utf_16be_encoding = "UTF-16BE";
static constexpr const char* utf_32_encoding = "UTF-32";

// 中文编码
static constexpr const char* gbk_encoding = "GBK";
static constexpr const char* gb18030_encoding = "GB18030";
static constexpr const char* big5_encoding = "BIG5";

// 日文编码
static constexpr const char* shift_jis_encoding = "SHIFT_JIS";
static constexpr const char* euc_jp_encoding = "EUC-JP";

// 欧洲编码
static constexpr const char* iso_8859_1_encoding = "ISO-8859-1";
static constexpr const char* cp1252_encoding = "CP1252";
```

### 结果结构

```cpp
struct IConvResult {
    std::string conv_result_str;    // 转换结果字符串
    int error_code;                 // 错误代码
    std::string error_msg;          // 错误消息
    
    bool IsSuccess() const {
        return error_code == 0;
    }
};
```

---

## 💡 使用示例

### 示例 1: 多语言文本处理

```cpp
#include "UniConv.h"
#include <iostream>
#include <vector>

int main() {
    auto conv = UniConv::GetInstance();
    
    // 多语言测试字符串
    std::vector<std::pair<std::string, std::string>> tests = {
        {"中文", "这是中文测试文本"},
        {"日文", "これは日本語のテストです"},
        {"韩文", "이것은 한국어 테스트입니다"},
        {"俄文", "Это русский тестовый текст"},
        {"阿拉伯文", "هذا نص تجريبي باللغة العربية"}
    };
    
    for (const auto& [lang, text] : tests) {
        auto utf8_result = conv->ConvertEncoding(text, "UTF-8", "UTF-8");
        auto gbk_result = conv->ConvertEncoding(text, "UTF-8", "GBK");
        
        std::cout << lang << ":" << std::endl;
        std::cout << "  原文: " << text << std::endl;
        std::cout << "  UTF-8: " << utf8_result.conv_result_str << std::endl;
        std::cout << "  GBK: " << gbk_result.conv_result_str << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
```

### 示例 2: 文件编码转换

```cpp
#include "UniConv.h"
#include <fstream>
#include <iostream>

bool convertFileEncoding(const std::string& inputFile, 
                        const std::string& outputFile,
                        const std::string& fromEncoding,
                        const std::string& toEncoding) {
    auto conv = UniConv::GetInstance();
    
    // 读取输入文件
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) {
        std::cerr << "无法打开输入文件: " << inputFile << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(inFile)),
                       std::istreambuf_iterator<char>());
    inFile.close();
    
    // 转换编码
    auto result = conv->ConvertEncoding(content, fromEncoding.c_str(), toEncoding.c_str());
    
    if (!result.IsSuccess()) {
        std::cerr << "编码转换失败: " << result.error_msg << std::endl;
        return false;
    }
    
    // 写入输出文件
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "无法创建输出文件: " << outputFile << std::endl;
        return false;
    }
    
    outFile.write(result.conv_result_str.c_str(), result.conv_result_str.size());
    outFile.close();
    
    std::cout << "文件转换成功!" << std::endl;
    std::cout << "输入:  " << inputFile << " (" << fromEncoding << ")" << std::endl;
    std::cout << "输出: " << outputFile << " (" << toEncoding << ")" << std::endl;
    
    return true;
}

int main() {
    convertFileEncoding("input_gbk.txt", "output_utf8.txt", "GBK", "UTF-8");
    return 0;
}
```

### 示例 3: 批量转换测试

```cpp
#include "UniConv.h"
#include "LightLogWriteImpl.h"

void runConversionTests() {
    auto conv = UniConv::GetInstance();
    
    // 测试数据
    std::string test_text = "测试文本 Hello World 123";
    
    // 支持的编码列表
    std::vector<std::string> encodings = {
        "UTF-8", "UTF-16LE", "UTF-16BE", "UTF-32",
        "GBK", "GB18030", "BIG5",
        "SHIFT_JIS", "EUC-JP",
        "ISO-8859-1", "CP1252"
    };
    
    std::cout << "开始批量转换测试..." << std::endl;
    
    for (const auto& target_encoding : encodings) {
        auto result = conv->ConvertEncoding(test_text, "UTF-8", target_encoding.c_str());
        
        if (result.IsSuccess()) {
            std::cout << "✓ UTF-8 -> " << target_encoding 
                      << " (长度: " << result.conv_result_str.size() << ")" << std::endl;
        } else {
            std::cout << "✗ UTF-8 -> " << target_encoding 
                      << " 失败: " << result.error_msg << std::endl;
        }
    }
}
```

---

## 🧪 测试说明

### 现代化测试系统

项目集成了完整的自动化测试框架，支持 CTest 和传统测试方式：

### 使用 CMake + CTest 运行测试（推荐）

```bash
# 构建测试版本
mkdir build && cd build
cmake .. -DUNICONV_BUILD_SHARED=ON -DUNICONV_BUILD_TESTS=ON
cmake --build . --config Debug

# 使用 CTest 运行测试
ctest --output-on-failure

# 或者直接运行测试程序
cd bin/Debug && ./UniConvTests.exe
```

### 测试特性

- **🚀 自动化集成**: 构建时自动复制 DLL 和测试数据
- **📁 智能路径处理**: 自动解决测试文件路径问题
- **🔍 详细调试信息**: 测试失败时提供详细的调试输出
- **⚡ 快速验证**: 一键运行所有编码转换测试

### 测试数据生成

项目包含Python脚本用于生成测试数据：

```bash
cd script
python generate_test_files.py    # 生成标准测试文件
```

### 测试覆盖

- **编码转换测试**: 100+ 种编码组合，包括双向转换验证
- **错误处理测试**: 无效输入和编码检测，边界条件测试
- **性能测试**: 大文件和批量转换性能基准
- **内存测试**: 内存泄漏和资源管理验证
- **线程安全测试**: 多线程环境并发测试
- **自动化验证**: 编码检测和 BOM 处理测试

---

## 📖 文档资源

### 完整文档
- **API文档**: [生成的 Doxygen 文档](docs/html/index.html)
- **编码指南**: [doc/常见编码简介.md](doc/常见编码简介.md)
- **Unicode参考**: [doc/unicode.md](doc/unicode.md)
- **iconv使用**: [doc/iconv.md](doc/iconv.md)
- **中文编码**: [doc/Chinese编码(GBK等).md](doc/Chinese编码(GBK等).md)

### 支持的编码

UniConv 通过 GNU libiconv 库支持 100+ 种字符编码：

| 类别 | 编码 |
|----------|-----------|
| **Unicode** | UTF-8, UTF-16LE, UTF-16BE, UTF-32LE, UTF-32BE |
| **中文** | GBK, GB2312, GB18030, Big5, HZ-GB2312 |
| **日文** | Shift-JIS, EUC-JP, ISO-2022-JP |
| **韩文** | EUC-KR, ISO-2022-KR, Johab |
| **欧洲** | ISO-8859-1~15, Windows-1250~1258 |
| **传统** | ASCII, EBCDIC variants, OEM code pages |

### 错误处理

```cpp
// 详细的错误信息
enum class ConversionError {
    SUCCESS = 0,
    INVALID_INPUT = 1,
    UNSUPPORTED_ENCODING = 2,
    INSUFFICIENT_BUFFER = 3,
    INCOMPLETE_SEQUENCE = 4
};
```

---

## 🤝 贡献指南

我们欢迎贡献！请查看我们的[贡献指南](CONTRIBUTING.md)了解详情。

### 开发工作流
1. **Fork** 仓库
2. **创建** 功能分支 (`git checkout -b feature/amazing-feature`)
3. **提交** 更改 (`git commit -m 'Add amazing feature'`)
4. **推送** 到分支 (`git push origin feature/amazing-feature`)
5. **打开** Pull Request

### 编码标准
- 遵循 **Google C++ Style Guide**
- 使用 **Doxygen** 进行文档化
- 为新功能包含**单元测试**
- 确保**跨平台**兼容性



---

## 📜 许可证

本项目基于 **GNU General Public License v3.0** 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

### 许可证摘要
- ✅ **商业使用** 允许
- ✅ **修改** 允许  
- ✅ **分发** 允许
- ✅ **私人使用** 允许
- ❗ **专利使用** 风险自负
- ❗ **责任** 限制
- ❗ **保证** 免责声明

---

## 🙏 致谢

### 依赖库
- **[GNU libiconv](https://www.gnu.org/software/libiconv/)** - 字符集转换库
- **[Microsoft Visual Studio](https://visualstudio.microsoft.com/)** - 开发环境
- **[Doxygen](https://www.doxygen.nl/)** - 文档生成

### 参考资料
- **[Unicode Consortium](https://unicode.org/)** - Unicode 标准
- **[IANA Character Sets](https://www.iana.org/assignments/character-sets/)** - 字符编码注册表
- **[Microsoft Code Pages](https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers)** - Windows 代码页参考

### 贡献者
- **hesphoros** - 初始开发和架构设计
- **社区贡献者** - 错误报告、功能请求和改进

---

## 📊 项目统计

### v2.0.0 版本亮点

- **🏗️ 现代化构建**: 完全重构的 CMake 构建系统
- **🚀 自动化流程**: DLL 自动复制、测试数据自动部署
- **🧪 完整测试**: CTest 集成、自动化测试验证
- **📦 内嵌依赖**: GNU libiconv 源码内嵌，无外部依赖
- **🔧 开发友好**: 一键构建、智能路径处理、详细调试信息

### 技术指标

- **转换函数**: 100+ 种转换函数
- **测试用例**: 200+ 个综合测试，涵盖所有主要编码
- **文档覆盖**: 95% API 覆盖率
- **平台支持**: Windows, Linux, macOS
- **编码支持**: 100+ 种字符编码，包括所有主流编码
- **CMake 版本**: 3.16+ 现代化构建配置
- **C++ 标准**: C++17 优化实现

---

## 🔗 相关链接

- **项目主页**: [GitHub Repository](https://github.com/hesphoros/UniConv)
- **发布版本**: [GitHub Releases](https://github.com/hesphoros/UniConv/releases)
- **API 文档**: [GitHub Pages](https://hesphoros.github.io/UniConv/)
- **问题反馈**: [GitHub Issues](https://github.com/hesphoros/UniConv/issues)
- **功能讨论**: [GitHub Discussions](https://github.com/hesphoros/UniConv/discussions)

---

## 🎉 致谢与支持

### 支持项目

如果 UniConv 对您有帮助，请考虑：

- ⭐ [在 GitHub 上给我们一个星标](https://github.com/hesphoros/UniConv)
- 🐛 [报告问题或建议](https://github.com/hesphoros/UniConv/issues)
- 💡 [参与功能讨论](https://github.com/hesphoros/UniConv/discussions)
- 🤝 [贡献代码](https://github.com/hesphoros/UniConv/pulls)

### 版本历程

- **v2.0.0** (2025): 现代化重构，CMake 构建系统，自动化测试
- **v1.x** (2025): 初始版本，基础编码转换功能

---

## 📄 更新日志

### v2.0.0-stable (2025-09-19)

- ✨ **重大更新**: 完全重构的 CMake 构建系统
- 🚀 **自动化改进**: 自动 DLL 复制和测试数据部署
- 🧪 **测试增强**: CTest 集成，完整的自动化测试套件
- 📦 **依赖内嵌**: GNU libiconv 源码完全内嵌
- 🔧 **开发体验**: 智能路径处理，详细调试信息
- 📝 **文档完善**: 更新所有文档和使用示例

