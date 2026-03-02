# UniConv

<div align="center">

**高性能 C++17 字符编码转换库**

[![Release](https://img.shields.io/github/v/release/hesphoros/UniConv?include_prereleases)](https://github.com/hesphoros/UniConv/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)

</div>

UniConv 是 GNU libiconv 的现代 C++17 封装，具有零拷贝优化、无锁并发缓存和高性能批处理能力。

## 核心特性

### v3.0 性能优化亮点
- **快速路径优化**: 同编码直接返回、ASCII 内容零开销传递
- **可选 SIMD 加速**: simdutf 集成，UTF 转换 5-15x 性能提升
- **O(1) LRU 缓存**: 双向链表 + 哈希表实现常数时间操作
- **分层缓冲池**: 32×4KB + 8×64KB + 2×1MB 三级缓冲区
- **无锁线程池**: 自适应并行策略，智能工作负载决策
- **智能缓冲估算**: 编码感知扩展因子，精确预估输出大小

### 性能与架构
- **无锁并发缓存**: 采用 parallel-hashmap 实现的无锁 iconv 描述符缓存
  - 4-way 并行 (16个子映射)，显著减少锁竞争
  - 支持多线程同时读写，无全局阻塞
  - 线程安全的 LRU 逐出策略
- **零拷贝转换**: BufferLease 机制实现零拷贝内存管理
- **高性能批处理**: 并行批量转换，最高 1.94x 性能提升
- **现代 C++17 API**: 基于 RAII 的资源管理，string_view 支持

### 功能特性
- **线程安全**: 原生多线程支持，无需外部同步
- **轻量依赖**: 仅需 libiconv 和 parallel-hashmap（支持 vcpkg/FetchContent 自动获取）
- **跨平台**: 支持 Windows、Linux 和 macOS
- **100+ 编码**: 支持 Unicode、亚洲和欧洲字符集
- **错误处理**: CompactResult 类型提供清晰的错误处理

## 性能基准

### v3.0 性能提升
```text
同编码快速路径:              ~0 ns (直接返回)
ASCII 快速路径:              ~50 ns/KB
simdutf UTF-8↔UTF-16:        5-15x 提升 (vs 传统 iconv)
O(1) LRU 缓存:               常数时间访问
分层缓冲池命中率:            >95%
```

### 单次转换性能
```text
ConvertEncodingFast:              434 ns/op  (230万 ops/s)
ConvertEncodingFast (zero-copy):  320 ns/op  (312万 ops/s)
性能提升:                          1.35x (零拷贝 vs 返回值)
```

### 批量转换性能
```text
并行批处理 (10个文本):       1.94x 性能提升
大文本批处理 (100KB+):       1.89x 性能提升
```

### 无锁缓存优势
- **减少锁竞争**: 16个子映射独立加锁
- **提升吞吐量**: 多线程同时访问不同子映射
- **降低延迟**: 无全局写锁阻塞读操作

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

#### 启用 SIMD 加速（可选）

UniConv 支持通过 [simdutf](https://github.com/simdutf/simdutf) 库实现 SIMD 加速的 UTF 转换。这是可选功能，默认关闭。

```bash
# 启用 simdutf SIMD 加速（通过 FetchContent 自动下载）
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNICONV_USE_SIMDUTF=ON
cmake --build . --config Release
```

**SIMD 加速支持的编码转换**：
- UTF-8 ↔ UTF-16LE
- UTF-8 ↔ UTF-16BE

**预期性能提升**：ASCII 主导文本 5-15x 性能提升

## 使用方法

### 基本转换

```cpp
#include <uniconv/UniConv.h>

int main() {
    auto conv = UniConv::GetInstance();
    
    // 基础转换 - GBK转UTF-8
    std::string output;
    bool success = conv->ConvertEncoding("中文测试", "GBK", "UTF-8", output);
    
    if (success) {
        std::cout << output << std::endl;
    }
    
    return 0;
}
```

### 使用 CompactResult 进行错误处理

```cpp
auto result = conv->ConvertEncodingFast("测试文本", "GBK", "UTF-8");

if (result.IsSuccess()) {
    std::cout << "转换成功: " << result.GetValue() << std::endl;
} else {
    std::cerr << "转换失败: " << static_cast<int>(result.GetError()) << std::endl;
}
```

### 零拷贝转换（使用 BufferLease）

```cpp
// 获取缓冲区
auto lease = conv->ConvertEncodingWithLease("大量文本数据", "GBK", "UTF-8");

if (lease.IsSuccess()) {
    // 零拷贝访问结果
    std::string_view result = lease.GetValue();
    ProcessData(result);  // 处理数据
    
    // lease 析构时自动归还缓冲区到池
}
```

### 批量转换

```cpp
// 批量转换
std::vector<std::string> texts = {"文本1", "文本2", "文本3"};  
auto results = conv->ConvertEncodingBatch(texts, "GBK", "UTF-8");

for (const auto& result : results) {
    if (result.IsSuccess()) {
        std::cout << result.GetValue() << std::endl;
    }
}
```

### 并行批处理（高性能）

```cpp
// 并行批量转换，自动利用多核CPU
std::vector<std::string> large_dataset = LoadLargeDataset();
auto results = conv->ConvertEncodingBatchParallel(
    large_dataset, 
    "GBK", 
    "UTF-8",
    std::thread::hardware_concurrency()  // 使用所有可用核心
);

// 1.94x 性能提升（相比顺序处理）
```

### string_view 支持（零拷贝输入）

```cpp
std::string_view input = GetLargeText();  // 无需拷贝
std::string output;

// 直接使用 string_view，避免输入拷贝
conv->ConvertEncoding(input, "GBK", "UTF-8", output);
```

## API 参考

### 核心转换 API

```cpp
// 基础转换（输出参数）
bool ConvertEncoding(const std::string& input, 
                     const char* fromEncoding, 
                     const char* toEncoding, 
                     std::string& output);

bool ConvertEncoding(std::string_view input,  // 零拷贝输入
                     const char* fromEncoding, 
                     const char* toEncoding, 
                     std::string& output);

// 返回 CompactResult
CompactResult<std::string> ConvertEncodingFast(
    const std::string& input,
    const char* fromEncoding,
    const char* toEncoding
);

// 零拷贝输出（BufferLease）
BufferLease ConvertEncodingWithLease(
    const std::string& input,
    const char* fromEncoding,
    const char* toEncoding
);
```

### 批量处理 API

```cpp
// 顺序批处理
std::vector<CompactResult<std::string>> ConvertEncodingBatch(
    const std::vector<std::string>& inputs,
    const char* fromEncoding,
    const char* toEncoding
);

// 并行批处理（推荐用于大批量）
std::vector<CompactResult<std::string>> ConvertEncodingBatchParallel(
    const std::vector<std::string>& inputs,
    const char* fromEncoding,
    const char* toEncoding,
    size_t num_threads = 0  // 0 = 自动检测
);
```

### 缓存统计 API

```cpp
struct PoolStats {
    size_t active_buffers;          // 活跃缓冲区数量
    size_t total_conversions;       // 总转换次数
    size_t cache_hits;              // 缓存命中次数
    double hit_rate;                // 命中率
    size_t iconv_cache_size;        // iconv缓存大小
    size_t iconv_cache_hits;        // iconv缓存命中
    size_t iconv_cache_misses;      // iconv缓存未命中
    size_t iconv_cache_evictions;   // iconv缓存逐出
};

PoolStats GetPoolStatistics() const;
```

## 技术架构

### 无锁并发缓存
- **parallel-hashmap**: Google Abseil 衍生的高性能并发哈希表
- **4-way 并行**: 16个子映射，减少 16x 锁竞争
- **细粒度锁定**: 每个子映射独立加锁
- **线程安全 LRU**: 无锁迭代 + 原子操作

### 零拷贝优化
- **BufferLease**: RAII 风格的缓冲区租约
- **对象池管理**: 预分配缓冲区，减少内存分配
- **string_view 输入**: 避免输入数据拷贝
- **移动语义**: 充分利用 C++17 移动优化

### 性能优化技术
- **线程局部缓存**: 每线程独立 iconv 描述符缓存
- **分支预测提示**: LIKELY/UNLIKELY 宏优化
- **缓存预取**: 提前加载数据到 CPU 缓存
- **内存对齐**: 优化缓存行访问

## 构建和测试

### 构建选项

```bash
# 标准构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 包含测试
cmake .. -DUNICONV_BUILD_TESTS=ON

# 包含示例
cmake .. -DBUILD_EXAMPLES=ON
```

### 运行测试

```bash
cd build
cmake --build . --config Release

# 运行所有测试
./bin/UniConvTests

# 运行性能基准测试
./bin/PerformanceBenchmark

# 运行缓存统计测试
./bin/CacheStatsTest
```

## 依赖库

### 外部依赖
- **libiconv**: 核心编码转换引擎（vcpkg 或 FetchContent 自动获取）
- **parallel-hashmap**: 无锁并发哈希表
  - 项目地址: [greg7mdp/parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap)
  - 基于 Google Abseil Swiss Tables
  - Header-only，vcpkg 或 FetchContent 自动获取

### 系统要求
- C++17 编译器
- CMake 3.16+
- Windows / Linux / macOS

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

- 基于 GNU libiconv 提供核心转换功能
- 采用 parallel-hashmap 实现无锁并发缓存
- 遵循现代 C++ 最佳实践和性能优化技术

## 更新日志

### v3.1.0 (2026-01-07)

#### 🗑️ 代码清理
- **移除 deprecated API**: 删除 `IConvResult`、`StringResultToIConvResult()`、`IConvResultToStringResult()`
- **移除 deprecated 方法**: 删除返回 `IConvResult` 的 `ConvertEncoding()` 重载
- **移除 `ConvertEncodingFastWithHint()`**: 自动估算已足够高效，使用 `ConvertEncodingFast()` 即可
- **清理未使用的 include**: 移除 `<iostream>`、`<fstream>`
- **移除生产代码中的调试输出**: 清理 `std::cout`/`std::cerr`

#### 🔄 迁移指南
| 旧 API | 新 API |
|---------|--------|
| `IConvResult` | `StringResult` (`CompactResult<std::string>`) |
| `ConvertEncoding()` 返回 IConvResult | `ConvertEncodingFast()` 返回 StringResult |
| `ConvertEncodingFastWithHint()` | `ConvertEncodingFast()` |

### v3.0.0 (2026-01)

#### ⚡ 性能优化
- **P0 快速路径**: 同编码快速路径、ASCII 内容快速路径
- **P1 SIMD 加速**: 可选的 simdutf 集成，UTF-8/UTF-16 转换 5-15x 性能提升
- **P1 O(1) LRU**: 使用双向链表 + 哈希表实现常数时间缓存操作
- **P2 哈希缓存键**: FNV-1a 64 位哈希替代字符串拼接，减少内存分配
- **P2 分层缓冲池**: 32×4KB + 8×64KB + 2×1MB 三级缓冲区
- **P2 无锁线程池**: 自适应并行策略，根据工作负载智能决策
- **P3 智能估算**: 编码感知扩展因子，精确预估输出缓冲区大小
- **P3 CPU 检测**: 运行时检测 SSE4.2/AVX2/AVX-512 等 SIMD 能力

#### ✨ 新特性
- 批量并行转换 API: `ConvertEncodingBatchParallel()`
- 零拷贝转换: `ConvertEncodingZeroCopy()`
- 实例化转换器: `UniConvInstance` 类
- CPU 优化信息: `CpuOptimization::GetInfo()`

### v2.0 (2025-12)
- ✨ 实现无锁并发缓存（parallel-hashmap）
- ✨ 添加零拷贝 BufferLease 机制
- ✨ 实现并行批处理 API
- ✨ 添加 string_view 重载
- 🚀 性能提升：单次转换 1.77x，批处理 1.94x
- 📝 完整的 API 文档和示例

### v1.0
- 初始版本
- 基础编码转换功能
- 线程安全支持

## 许可与第三方组件声明

- 本项目主体代码由 `hesphoros` 以 MIT 许可证发布（见 `LICENSE`）。
- 本仓库包含 GNU libiconv 的源代码（见 `src/` 目录中的 `iconv.c`、`localcharset.c` 等文件）。
    libiconv 源代码遵循 **GNU Lesser General Public License (LGPL) v2.1 或更高版本**。

为遵守开源许可：

- 仓库中包含 `COPYING.LIB`，说明 libiconv 的许可证及合规要点；
- 仓库中包含 `THIRD_PARTY_LICENSES.md`，列出已包含的第三方代码以及分发时需要注意的合规操作；
- 如果你要分发静态链接的二进制文件（即将 libiconv 源直接合并进可执行文件），你需要遵守 LGPL 要求（保留版权、提供修改后的 libiconv 源或提供可重链接的对象文件等）。

