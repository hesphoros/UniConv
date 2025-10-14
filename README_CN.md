# UniConv

高性能C++字符编码转换库，提供现代化C++17接口。

[中文文档](README_CN.md) | [English](README.MD)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)

UniConv是GNU libiconv的现代C++17封装，具有性能优化和线程安全特性。

## 特性

- **现代C++17 API**: 基于RAII的资源管理
- **高性能**: 比传统iconv快2.35倍
- **零外部依赖**: 内嵌GNU libiconv，开箱即用
- **线程安全**: 原生多线程支持
- **跨平台**: 支持Windows、Linux和macOS
- **100+编码**: 支持Unicode、亚洲和欧洲字符集

## 性能

```text
ConvertEncodingFast:         434 ns/op  (230万 ops/s)
ConvertEncodingFastWithHint: 358 ns/op  (280万 ops/s)
传统方法:                    810 μs/1000次  (123万 ops/s)
最大提升:                    2.35倍
```

## 支持的编码

- **Unicode**: UTF-8, UTF-16LE/BE, UTF-32LE/BE
- **中文**: GBK, GB2312, GB18030, Big5
- **日文**: Shift-JIS, EUC-JP, ISO-2022-JP
- **欧洲**: ISO-8859-1~15, Windows-1252
- **100+种**: 详见文档完整列表

## 快速开始 

### 系统要求

- 支持C++17的编译器
- CMake 3.16或更高版本
- Windows、Linux或macOS

### 安装

#### 使用vcpkg（推荐）

```bash
vcpkg install uniconv
```

#### 使用CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    UniConv
    GIT_REPOSITORY https://github.com/hesphoros/UniConv.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(UniConv)

target_link_libraries(your_target PRIVATE UniConv)
```

#### 手动构建

```bash
git clone https://github.com/hesphoros/UniConv.git
cd UniConv
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 使用方法

### 基本用法

```cpp
#include "UniConv.h"

int main() {
    auto conv = UniConv::GetInstance();
    
    // GBK转UTF-8
    auto result = conv->ConvertEncodingFast("中文测试", "GBK", "UTF-8");
    
    if (result.IsSuccess()) {
        std::cout << result.GetValue() << std::endl;
    } else {
        std::cerr << "转换失败: " << result.GetError() << std::endl;
    }
    
    return 0;
}
```

### 批量处理

```cpp
std::vector<std::string> texts = {"文本1", "文本2", "文本3"};  
auto results = conv->ConvertEncodingBatch(texts, "GBK", "UTF-8");
```

## API参考

```cpp
// 核心函数
ConvertEncodingFast(input, from_encoding, to_encoding)
ConvertEncodingFastWithHint(input, from_encoding, to_encoding, buffer)  
ConvertEncodingBatch(inputs, from_encoding, to_encoding)

// 错误处理
auto result = conv->ConvertEncodingFast(input, from, to);
if (result.IsSuccess()) {
    auto converted = result.GetValue();
}
```

## 构建和测试

### 构建选项

```bash
# 标准构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 包含测试
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# 包含示例
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
```

### 运行测试

```bash
cd build
ctest --output-on-failure
```

## 贡献

欢迎贡献！请随时提交Pull Request。对于重大变更，请先开issue讨论。

1. Fork仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 许可证

本项目采用MIT许可证 - 详见 [LICENSE](LICENSE) 文件。

## 致谢

- 基于GNU libiconv提供核心转换功能
- 采用现代C++最佳实践和性能优化技术
