# UniConv FetchContent 集成指南

## 概述

UniConv 现已完全支持通过 CMake 的 FetchContent 机制作为子项目集成到其他项目中。这种集成方式具有以下优势：

- 🔧 **自动依赖管理**：无需手动下载或编译
- 🎯 **版本控制**：可指定具体的版本或分支
- 🚀 **构建优化**：作为子项目时自动禁用测试等非必要组件
- 📦 **完全隔离**：不会影响主项目的配置

## 使用方法

### 1. 基本用法

在您的 CMakeLists.txt 中添加以下代码：

```cmake
cmake_minimum_required(VERSION 3.14)
project(YourProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 FetchContent 包含 UniConv
include(FetchContent)

FetchContent_Declare(
    UniConv
    GIT_REPOSITORY https://github.com/your-username/UniConv.git
    GIT_TAG        main  # 或指定版本标签如 v2.0.0
)

# 设置 UniConv 选项（可选，在 FetchContent_MakeAvailable 之前）
set(UNICONV_BUILD_TESTS OFF CACHE BOOL "Disable UniConv tests in subproject")
set(UNICONV_BUILD_SHARED OFF CACHE BOOL "Build UniConv as static library")

# 让 FetchContent 下载并配置 UniConv
FetchContent_MakeAvailable(UniConv)

# 创建您的可执行文件
add_executable(your_app main.cpp)

# 链接 UniConv
target_link_libraries(your_app PRIVATE UniConv)
```

### 2. 从本地路径集成（开发时）

如果您有本地的 UniConv 副本，可以这样使用：

```cmake
FetchContent_Declare(
    UniConv
    SOURCE_DIR /path/to/local/UniConv  # 指向 UniConv 根目录
)
```

### 3. 代码示例

```cpp
#include <iostream>
#include <string>
#include "UniConv.h"

int main() {
    try {
        // 获取 UniConv 单例实例
        auto uniconv = UniConv::GetInstance();
        
        // 进行编码转换
        std::string input = "Hello, World!";
        auto result = uniconv->ConvertEncodingFast(input, "UTF-8", "GBK");
        
        if (result.IsSuccess()) {
            std::cout << "转换成功！" << std::endl;
        } else {
            std::cout << "转换失败：" << result.GetErrorMessage() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 配置选项

UniConv 提供以下配置选项，可在 `FetchContent_MakeAvailable` 之前设置：

| 选项 | 默认值（子项目） | 描述 |
|------|----------------|------|
| `UNICONV_BUILD_TESTS` | `OFF` | 是否构建测试 |
| `UNICONV_BUILD_SHARED` | `OFF` | 是否构建动态库 |

### 子项目自动优化

当 UniConv 作为子项目时，会自动应用以下优化：

- ✅ 测试默认关闭（`UNICONV_BUILD_TESTS=OFF`）
- ✅ 构建静态库而非动态库
- ✅ 不设置安装规则
- ✅ 配置文件生成到构建目录而不是源码目录

## 构建系统特性

### 清理操作

UniConv 提供了增强的清理目标：

```bash
# 标准清理（保留配置文件）
cmake --build build --target clean

# 清理生成的配置文件
cmake --build build --target clean-config

# 完全清理（包括所有生成文件）
cmake --build build --target distclean
```

### 生成文件管理

- `config.h` 现在正确生成到构建目录 (`build/include/iconv/config.h`)
- 源代码目录保持整洁，不包含生成的文件
- 符合 CMake 最佳实践

## 完整示例项目

参考 `examples/fetchcontent_example/` 目录中的完整示例：

- `CMakeLists.txt`: 完整的 FetchContent 配置
- `main.cpp`: 使用 UniConv API 的示例代码

## 系统要求

- CMake 3.14 或更高版本（FetchContent 支持）
- C++17 兼容的编译器
- 支持的平台：Windows、Linux、macOS

## 常见问题

### Q: 为什么配置时间较长？
A: 首次配置时需要下载依赖和检测系统特性，这是正常的。后续构建会更快。

### Q: 如何指定特定版本？
A: 在 `FetchContent_Declare` 中使用 `GIT_TAG` 指定版本标签或提交哈希。

### Q: 可以与其他构建系统一起使用吗？
A: UniConv 主要为 CMake 优化。对于其他构建系统，建议手动构建并链接静态库。

## 最佳实践

1. **版本固定**：在生产环境中使用具体的版本标签而不是 `main` 分支
2. **选项设置**：明确设置所需的构建选项
3. **依赖隔离**：使用 `PRIVATE` 链接避免传递依赖
4. **缓存配置**：在持续集成中缓存 `_deps` 目录以加速构建

---

通过 FetchContent 集成 UniConv，您可以轻松地在项目中获得强大的编码转换功能，而无需复杂的依赖管理！