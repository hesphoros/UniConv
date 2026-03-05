# UniConv 性能基准

> 数据基于 [Google Benchmark](https://github.com/google/benchmark) 实测。
> 测试平台：Apple Silicon (10核, ARM64)，Release 优化，simdutf v7.7.1 NEON。

## 核心转换吞吐量（iconv 标量路径）

| 编码对 | 64B | 4KB | 64KB | 1MB |
|--------|-----|-----|------|-----|
| UTF-8 → UTF-16LE | 301 MB/s | 797 MB/s | 870 MB/s | **895 MB/s** |
| UTF-8 → UTF-16BE | 294 MB/s | 725 MB/s | 701 MB/s | 806 MB/s |
| UTF-8 → UTF-32LE | 317 MB/s | 756 MB/s | 691 MB/s | 814 MB/s |
| UTF-8 → GBK | 261 MB/s | 408 MB/s | 412 MB/s | 422 MB/s |
| UTF-16LE ↔ UTF-16BE | 199 MB/s | 757 MB/s | 709 MB/s | 763 MB/s |

## simdutf SIMD 加速（`-DUNICONV_USE_SIMDUTF=ON`）

| 编码对 | 1KB | 4KB | 16KB | 1MB | vs iconv |
|--------|-----|-----|------|-----|----------|
| UTF-8 → UTF-16LE | **2.43 GB/s** | **3.30 GB/s** | **3.62 GB/s** | **3.62 GB/s** | **4.0x** |
| UTF-16LE → UTF-8 | **2.83 GB/s** | **4.36 GB/s** | **5.01 GB/s** | **5.12 GB/s** | **9.4x** |
| UTF-8 → UTF-16BE | **2.45 GB/s** | **3.30 GB/s** | **3.60 GB/s** | **3.58 GB/s** | **4.4x** |

## 文本类型影响

| 文本类型 | iconv | simdutf | 加速比 |
|----------|-------|---------|--------|
| ASCII | 459 MB/s | **5.62 GB/s** | **12.5x** |
| CJK 中文 | 779 MB/s | **3.35 GB/s** | **4.3x** |
| Emoji | 898 MB/s | **2.57 GB/s** | **2.9x** |
| 混合文本 | 505 MB/s | **1.33 GB/s** | **2.6x** |

## API 风格对比

| API 风格 | 延迟 | 吞吐量 |
|----------|------|--------|
| 返回值 (`StringResult`) | 5038 ns | 775 MB/s |
| 输出参数 (`std::string&`) | 4952 ns | 789 MB/s |
| CompactResult (`Ex`) | 5019 ns | 778 MB/s |
| 输出参数 + ErrorCode | 4638 ns | **842 MB/s** |

## 批量处理性能

| 批量大小 | 逐条处理 | 串行 Batch | 并行 Batch | 并行加速比 |
|----------|---------|-----------|-----------|-----------|
| 16 条 | 647 MB/s | 658 MB/s | 679 MB/s | 1.05x |
| 64 条 | 644 MB/s | 698 MB/s | 948 MB/s | 1.47x |
| 256 条 | 652 MB/s | 703 MB/s | **2.93 GB/s** | 4.49x |
| 1024 条 | 633 MB/s | 681 MB/s | **6.29 GB/s** | 9.94x |
| 4096 条 | 609 MB/s | 680 MB/s | **10.76 GB/s** | 17.7x |

## 实例创建开销

| 方式 | 耗时 | 说明 |
|------|------|------|
| `UniConv::Create()` | 6261 ns | 堆分配 + 成员初始化 |
| `UniConv::ThreadLocal()` | **0.75 ns** | 线程局部单例，首次后零开销 |

## 业界对比

> 参考：[simdutf 官方基准](https://github.com/simdutf/simdutf)、[Daniel Lemire AVX-512 论文](https://arxiv.org/abs/2212.05098)、[ICU 性能测试](https://github.com/unicode-org/icu-perf)

| 库 | 技术路线 | 典型吞吐量 | 编码数 | 备注 |
|----|---------|-----------|--------|------|
| [simdutf](https://github.com/simdutf/simdutf) (独立) | SIMD (AVX-512/NEON) | 5~12 GB/s | 4 种 | Node.js/Chromium/WebKit 内核 |
| **UniConv + simdutf** | SIMD + iconv | **3.3~5.1 GB/s** | **180+** | UTF 走 SIMD，其余走优化 iconv |
| **UniConv** (纯 iconv) | 优化标量 | ~900 MB/s | 180+ |  |
| [ICU](https://github.com/unicode-org/icu) (ucnv) | 标量 | ~400-600 MB/s | 200+ | 功能最全 |
| glibc iconv | 标量 | ~500-800 MB/s | 100+ | POSIX 标准 |
| [libiconv](https://www.gnu.org/software/libiconv/) (GNU) | 标量 | ~500-700 MB/s | 150+ | 跨平台 |
| Python codecs | 解释器 | ~50-100 MB/s | 100+ | 解释器开销 |
| Java `String.getBytes()` | JVM | ~200-400 MB/s | 40+ | JIT 优化后 |

## 运行基准测试

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNICONV_BUILD_BENCHMARKS=ON
cmake --build . --target UniConvBench
./bin/UniConvBench

# 启用 SIMD
cmake .. -DCMAKE_BUILD_TYPE=Release -DUNICONV_BUILD_BENCHMARKS=ON -DUNICONV_USE_SIMDUTF=ON
```

测试维度：核心吞吐量（6 编码对 × 8 数据大小）、文本类型、API 风格、缓冲区复用、批量处理（串行/并行）、往返转换、实例创建。
