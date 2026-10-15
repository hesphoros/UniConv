# UniConv

<div align="center">

**高性能 C++17 Unicode 与多编码转换库**

*180+ 种编码 · SIMD 加速 · 无锁并发 · 多核并行批处理*

[![Release](https://img.shields.io/github/v/release/hesphoros/UniConv?include_prereleases)](https://github.com/hesphoros/UniConv/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Tests](https://img.shields.io/badge/Tests-191%20passed-brightgreen.svg)]()

</div>

---

UniConv 将 GNU libiconv 的编码广度（180+ 种字符集）与现代 C++ 高性能技术栈相结合：可选 [simdutf](https://github.com/simdutf/simdutf) SIMD 加速、无锁并发描述符缓存、零拷贝 I/O 和自适应多核批处理。

## 核心特性

- **180+ 种编码** — UTF-8/16/32、GBK、Shift_JIS、ISO-8859、EBCDIC 等完整覆盖
- **SIMD 加速** — 可选 [simdutf](https://github.com/simdutf/simdutf) 集成，UTF-8/16 互转 **3.3~5.1 GB/s**（4~12.5x 加速）
- **多核并行批处理** — 4096 条数据并行转换达 **10.76 GB/s**（17.7x 加速比）
- **无锁并发缓存** — [parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap) 4-way 并行 + O(1) LRU 逐出
- **零拷贝 I/O** — `string_view` 直传 iconv + `BufferLease` 零拷贝输出 + iconv 直写 `std::string`
- **线程安全** — 所有公共 API 均可多线程直接调用
- **跨平台** — Windows / Linux / macOS，vcpkg / FetchContent / 源码内嵌

## 性能一览

> Apple Silicon (10核, ARM64)，Release 优化。完整数据见 [BENCHMARK.md](BENCHMARK.md)。

```
吞吐量梯队（UTF-8 → UTF-16, 1MB, 单线程）:

  simdutf (AVX-512)        ████████████████████████  12 GB/s
  UniConv+simdutf (NEON)   ███████████               3.6 GB/s  ← 开启 SIMD
  ─── SIMD ─────────────────────────────────────
  UniConv (iconv)          ███                        900 MB/s  ← 标量第一梯队
  glibc iconv              ██                         700 MB/s
  ICU ucnv                 ██                         500 MB/s
  ─── 标量 ─────────────────────────────────────
  UniConv (并行 batch)     ██████████████████████    10.76 GB/s ← 多核并行
```

| 场景 | 吞吐量 |
|------|--------|
| UTF-8 → UTF-16LE (iconv) | **895 MB/s** |
| UTF-8 → UTF-16LE (simdutf) | **3.62 GB/s** |
| ASCII 文本 (simdutf) | **5.62 GB/s** |
| 并行 batch 4096 条 | **10.76 GB/s** |
| `ThreadLocal()` 获取实例 | **0.75 ns** |

## 快速开始

**安装**

```bash
vcpkg install hesphoros-uniconv                            # vcpkg
# 或 CMake FetchContent:
# FetchContent_Declare(UniConv GIT_REPOSITORY https://github.com/hesphoros/UniConv.git GIT_TAG main)
# FetchContent_MakeAvailable(UniConv)
# target_link_libraries(your_target PRIVATE UniConv)
```

**使用**

```cpp
#include <UniConv/UniConv.h>

auto& conv = UniConv::ThreadLocal();  // 0.75ns 获取实例

// 单条转换
std::string output;
conv.ConvertEncoding("中文测试", "GBK", "UTF-8", output);

// 带错误处理
auto result = conv.ConvertEncodingFast("测试", "GBK", "UTF-8");
if (result.IsSuccess()) { /* result.GetValue() */ }

// 最快路径 (842 MB/s)
ErrorCode err = conv.ConvertEncodingFast("数据", "UTF-8", "UTF-16LE", output);

// 并行批处理 (4096条 → 10.76 GB/s)
std::vector<std::string> dataset = LoadData(), outputs;
conv.ConvertEncodingBatchParallel(dataset, "GBK", "UTF-8", outputs);

// string_view 零拷贝
conv.ConvertEncodingFast(std::string_view{large_buf, len}, "UTF-8", "UTF-16LE", output);
```

## 构建

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release                    # 标准构建
cmake .. -DUNICONV_USE_SIMDUTF=ON                      # 启用 SIMD
cmake .. -DUNICONV_BUILD_TESTS=ON                      # 单元测试 (Google Test)
cmake .. -DUNICONV_BUILD_BENCHMARKS=ON                 # 性能基准 (Google Benchmark)
```

测试套件包含运行时转换回归、头文件契约、`char8_t` 配置和混合 C++ 标准测试。

## 编译期宏与 UTF-8

UniConv 4.0 在 `<UniConv/UniConv.h>` 中提供两个只读检测宏：

| 宏 | 含义 |
|---|---|
| `UNICONV_CXX_STANDARD` | 当前翻译单元的 C++ 标准值，如 `201703L`、`202002L` |
| `UNICONV_HAS_CHAR8_T` | 当前翻译单元是否启用了 `char8_t`，值固定为 `0` 或 `1` |

```cpp
#include <UniConv/UniConv.h>

#if UNICONV_CXX_STANDARD >= 202002L && UNICONV_HAS_CHAR8_T
// 当前翻译单元启用了 C++20 char8_t
#endif
```

这两个宏由 UniConv 计算，调用方不能预定义或覆盖。UniConv 4.0 要求 C++17，并移除了旧的无前缀 `CPP_STANDARD`、头文件自动执行字符集 pragma，以及字面量/代码页包装宏。

UTF-8 字面量有两种明确路线：

- `u8"..."` 明确表示 UTF-8 code unit；C++17 中元素类型是 `char`，启用 `char8_t` 的 C++20 中是 `char8_t`，不能直接传给现有 `std::string_view` API。
- 普通窄字面量 `"..."` 保持 `char` 类型；消费方应在自己的目标上配置 UTF-8 编译选项，例如 MSVC 的 `/utf-8`。UniConv 自身使用的私有编译选项不会传播给调用方。

字符集编译选项属于构建系统职责。UniConv 不通过公共头文件修改调用方翻译单元的执行字符集，也不隐式执行 `char8_t` 到 `char` 的转换。

## 依赖

| 依赖 | 用途 | 许可证 |
|------|------|--------|
| [GNU libiconv](https://www.gnu.org/software/libiconv/) | 编码转换引擎 | LGPL-2.1+ |
| [parallel-hashmap](https://github.com/greg7mdp/parallel-hashmap) | 无锁并发哈希表 | Apache-2.0 |
| [simdutf](https://github.com/simdutf/simdutf) *(可选)* | SIMD UTF 加速 | Apache-2.0 / MIT |

**系统要求**：C++17 · CMake 3.16+ · Windows / Linux / macOS

## 许可证

本项目采用 **MIT 许可证** — 详见 [LICENSE](LICENSE)。
本仓库内嵌的 libiconv 源代码遵循 **LGPL-2.1+** — 详见 [COPYING.LIB](COPYING.LIB) 和 [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md)。
