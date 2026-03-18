# UniConv 性能基准

> 数据基于 [Google Benchmark](https://github.com/google/benchmark) 实测。

### 测试平台

| 平台 | CPU | 核心/线程 | 缓存 | 架构 |
|------|-----|----------|------|------|
| **macOS** | Apple Silicon | 10 核 | — | ARM64 (NEON) |
| **Windows 11** | 16 × 3793 MHz | 8C/16T | L1 32KB×8, L2 1MB×8, L3 16MB | x86_64 |

## 核心转换吞吐量（iconv 标量路径）

| 编码对 | 64B | 4KB | 64KB | 1MB | 平台 |
|--------|-----|-----|------|-----|------|
| UTF-8 → UTF-16LE | 301 MB/s | 797 MB/s | 870 MB/s | **895 MB/s** | macOS |
| UTF-8 → UTF-16LE | 265 MB/s | 2.73 GiB/s | 1.75 GiB/s | **1.53 GiB/s** | Windows (2026-03-19) |
| UTF-16LE → UTF-8 | 287 MB/s | 3.27 GiB/s | 3.33 GiB/s | **1.70 GiB/s** | Windows (2026-03-19) |
| UTF-8 → UTF-16BE | 294 MB/s | 725 MB/s | 701 MB/s | 806 MB/s | macOS |
| UTF-8 → UTF-16BE | 246 MB/s | 2.43 GiB/s | 2.90 GiB/s | **1.40 GiB/s** | Windows (2026-03-19) |
| UTF-8 → UTF-32LE | 317 MB/s | 756 MB/s | 691 MB/s | 814 MB/s | macOS |
| UTF-8 → UTF-32LE | 246 MB/s | 560 MB/s | 669 MB/s | 411 MB/s | Windows (2026-03-19) |
| UTF-8 → GBK | 261 MB/s | 408 MB/s | 412 MB/s | 422 MB/s | macOS |
| UTF-8 → GBK | 191 MB/s | 372 MB/s | 403 MB/s | 320 MB/s | Windows (2026-03-19) |
| UTF-16LE ↔ UTF-16BE | 199 MB/s | 757 MB/s | 709 MB/s | 763 MB/s | macOS |
| UTF-16LE ↔ UTF-16BE | 164 MB/s | 553 MB/s | 669 MB/s | 427 MB/s | Windows (2026-03-19) |

## simdutf SIMD 加速（`-DUNICONV_USE_SIMDUTF=ON`）

> 以下数据来自 macOS ARM64 + simdutf v7.7.1 NEON

| 编码对 | 1KB | 4KB | 16KB | 1MB | vs iconv |
|--------|-----|-----|------|-----|----------|
| UTF-8 → UTF-16LE | **2.43 GB/s** | **3.30 GB/s** | **3.62 GB/s** | **3.62 GB/s** | **4.0x** |
| UTF-16LE → UTF-8 | **2.83 GB/s** | **4.36 GB/s** | **5.01 GB/s** | **5.12 GB/s** | **9.4x** |
| UTF-8 → UTF-16BE | **2.45 GB/s** | **3.30 GB/s** | **3.60 GB/s** | **3.58 GB/s** | **4.4x** |

### Windows + simdutf 多线程专项

> 构建：`build`（`UNICONV_USE_SIMDUTF=ON`）
> 命令：`UniConvBench --benchmark_filter="BM_MT_(SIMDUTF_RAW_Utf(8ToUtf16LE|16LEToUtf8)|Utf(8ToUtf16LE|16LEToUtf8))" --benchmark_min_time=0.2s`

| 用例 | 64KB T1 | 64KB T16 | 1MB T1 | 1MB T16 |
|------|---------|----------|--------|---------|
| UniConv API UTF-8 → UTF-16LE | 2.78 GiB/s | 978 MiB/s | 1.76 GiB/s | 342 MiB/s |
| UniConv API UTF-16LE → UTF-8 | 3.28 GiB/s | 1009 MiB/s | 1.66 GiB/s | 270 MiB/s |
| raw simdutf UTF-8 → UTF-16LE | 5.93 GiB/s | 2.85 GiB/s | 6.06 GiB/s | 2.39 GiB/s |
| raw simdutf UTF-16LE → UTF-8 | 4.69 GiB/s | 2.13 GiB/s | 4.68 GiB/s | 1.36 GiB/s |


## 文本类型影响

| 文本类型 | macOS iconv | macOS simdutf | 加速比 | Windows iconv |
|----------|-------------|---------------|--------|---------------|
| ASCII | 459 MB/s | **5.62 GB/s** | **12.5x** | **2.43 GiB/s** |
| CJK 中文 | 779 MB/s | **3.35 GB/s** | **4.3x** | **2.43 GiB/s** |
| Emoji | 898 MB/s | **2.57 GB/s** | **2.9x** | **2.19 GiB/s** |
| 混合文本 | 505 MB/s | **1.33 GB/s** | **2.6x** | **1.56 GiB/s** |

## API 风格对比

| API 风格 | macOS 延迟 | macOS 吞吐量 | Windows 延迟 | Windows 吞吐量 |
|----------|-----------|-------------|-------------|---------------|
| 返回值 (`StringResult`) | 5038 ns | 775 MB/s | 1395 ns | **2.73 GiB/s** |
| 输出参数 (`std::string&`) | 4952 ns | 789 MB/s | 1395 ns | **2.73 GiB/s** |
| CompactResult (`Ex`) | 5019 ns | 778 MB/s | 1569 ns | 2.43 GiB/s |
| 输出参数 + ErrorCode | 4638 ns | **842 MB/s** | 1395 ns | **2.73 GiB/s** |

## 缓冲区复用（Windows）

| 模式 | 延迟 | 吞吐量 |
|------|------|--------|
| 无复用 | 1395 ns | 2.73 GiB/s |
| 复用缓冲区 | 1395 ns | 2.73 GiB/s |

## 零拷贝 string_view vs string 拷贝（Windows）

| 数据大小 | string_view | string copy |
|----------|-------------|-------------|
| 64B | 345 MB/s | 313 MB/s |
| 256B | 581 MB/s | 996 MB/s |
| 4KB | 700 MB/s | 2.73 GiB/s |
| 64KB | 714 MB/s | 2.92 GiB/s |
| 1MB | 640 MB/s | **2.08 GiB/s** |

## 批量处理性能

| 批量大小 | macOS 逐条 | macOS 串行 | macOS 并行 | macOS 并行加速 | Windows 逐条 | Windows 串行 | Windows 并行 | Windows 并行加速 |
|----------|-----------|-----------|-----------|--------------|-------------|-------------|-------------|----------------|
| 16 条 | 647 MB/s | 658 MB/s | 679 MB/s | 1.05x | 930 MB/s | 500 MB/s | 496 MB/s | 0.53x |
| 64 条 | 644 MB/s | 698 MB/s | 948 MB/s | 1.47x | 992 MB/s | 558 MB/s | 992 MB/s | 1.00x |
| 256 条 | 652 MB/s | 703 MB/s | **2.93 GB/s** | 4.49x | 893 MB/s | 510 MB/s | **2.48 GiB/s** | 2.84x |
| 1024 条 | 633 MB/s | 681 MB/s | **6.29 GB/s** | 9.94x | 893 MB/s | 476 MB/s | **2.22 GiB/s** | 2.55x |
| 4096 条 | 609 MB/s | 680 MB/s | **10.76 GB/s** | 17.7x | 854 MB/s | 410 MB/s | **5.07 GiB/s** | 6.08x |

## 往返转换（Windows）

| 转换链 | 延迟 | 吞吐量 |
|--------|------|--------|
| UTF-8 ↔ UTF-16LE | 2267 ns | **3.36 GiB/s** |
| UTF-8 ↔ GBK | 19463 ns | 401 MB/s |
| UTF-8 → 16LE → 16BE → 32LE → UTF-8 | 20926 ns | 187 MB/s |

## 实例创建开销

| 方式 | macOS | Windows | 说明 |
|------|-------|---------|------|
| `UniConv::Create()` | 6261 ns | 15695 ns | 堆分配 + 成员初始化 |
| `UniConv::ThreadLocal()` | **0.75 ns** | **1.17 ns** | 线程局部单例，首次后零开销 |

## 业界对比

> 参考：[simdutf 官方基准](https://github.com/simdutf/simdutf)、[Daniel Lemire AVX-512 论文](https://arxiv.org/abs/2212.05098)、[ICU 性能测试](https://github.com/unicode-org/icu-perf)

| 库 | 技术路线 | 典型吞吐量 | 编码数 | 备注 |
|----|---------|-----------|--------|------|
| [simdutf](https://github.com/simdutf/simdutf) (独立) | SIMD (AVX-512/NEON) | 5~12 GB/s | 4 种 | Node.js/Chromium/WebKit 内核 |
| **UniConv + simdutf** | SIMD + iconv | **3.3~5.1 GB/s** | **180+** | UTF 走 SIMD，其余走优化 iconv |
| **UniConv** (纯 iconv) | 优化标量 | ~0.9~2.9 GB/s | 180+ | macOS ~900MB/s, Windows ~1.1~2.9GB/s |
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

测试维度：核心吞吐量（6 编码对 × 8 数据大小）、文本类型、API 风格、缓冲区复用、零拷贝对比、批量处理（串行/并行）、往返转换、实例创建。

## 新增测试矩阵

用于验证“并行策略生效”，建议至少跑以下四组：

1. 小数据多任务：`BM_Phase1_SmallMany_Parallel_Auto`
2. 大数据少任务：`BM_Phase1_LargeFew_Parallel_Auto`
3. ASCII 兼容编码对：`BM_Phase1_AsciiCompatiblePair`（`UTF-8 -> ISO-8859-1`）
4. 非 ASCII 兼容编码对：`BM_Phase1_NonAsciiCompatiblePair`（`UTF-8 -> UTF-16LE`）

2026-03-19 全量实测（Windows）关键结果：

| 用例 | 最佳或关键数据点 |
|------|------------------|
| BM_Phase1_SmallMany_Parallel_Auto | 4096 items: 1.14844 GiB/s |
| BM_Phase1_LargeFew_Parallel_Auto | 32 items: 12.4166 GiB/s |
| BM_Phase1_AsciiCompatiblePair | 64KB: 19.6001 GiB/s；1MB: 4 GiB/s |
| BM_Phase1_NonAsciiCompatiblePair | 64KB: 2.69227 GiB/s；1MB: 2.09896 GiB/s |

Google Benchmark ：

```bash
./bin/UniConvBench --benchmark_filter=BM_Phase1_.*
```

Windows 下建议做两种构建对比（验证 `thread_local` 路径差异）：

1. 非 DLL（默认）

```bash
cmake -S . -B build-static -DCMAKE_BUILD_TYPE=Release -DUNICONV_BUILD_BENCHMARKS=ON -DUNICONV_BUILD_SHARED=OFF
cmake --build build-static --config Release --target UniConvBench
build-static/bin/Release/UniConvBench.exe --benchmark_filter=BM_Phase1_.*
```

2. DLL 模式（会触发 `UNICONV_NO_THREAD_LOCAL`）

```bash
cmake -S . -B build-shared -DCMAKE_BUILD_TYPE=Release -DUNICONV_BUILD_BENCHMARKS=ON -DUNICONV_BUILD_SHARED=ON
cmake --build build-shared --config Release --target UniConvBench
build-shared/bin/Release/UniConvBench.exe --benchmark_filter=BM_Phase1_.*
```

## API 层模式对照（Convenience vs Stateless）

> 测试命令：`--benchmark_filter=BM_Mode_.* --benchmark_min_time=0.1s`

### Static 构建（2026-03-19）

| 维度 | Convenience | Stateless | 结论 |
|------|-------------|-----------|------|
| Single 4KB | 2.73 GiB/s | 2.43 GiB/s | Convenience 更快（缓存复用收益） |
| Single 1MB | 2.00 GiB/s | 1.40 GiB/s | Convenience 明显更快 |
| BatchSerial 1024 | 680.76 MiB/s | 802.85 MiB/s | Stateless 更快 |
| BatchSerial 4096 | 573.75 MiB/s | 737.68 MiB/s | Stateless 更快 |
| BatchParallel 1024 | 1.95 GiB/s | 2.32 GiB/s | Stateless 更快 |
| BatchParallel 4096 | 3.46 GiB/s | 5.58 GiB/s | Stateless 明显更快 |

### Shared 构建（2026-03-19）

| 维度 | Convenience | Stateless | 结论 |
|------|-------------|-----------|------|
| Single 4KB | 699.83 MiB/s | 668.86 MiB/s | Convenience 略快 |
| Single 1MB | 718.22 MiB/s | 640.00 MiB/s | Convenience 更快 |
| BatchSerial 1024 | 595.66 MiB/s | 357.71 MiB/s | Convenience 更快 |
| BatchSerial 4096 | 573.75 MiB/s | 318.75 MiB/s | Convenience 更快 |
| BatchParallel 1024 | 2.83 GiB/s | 2.27 GiB/s | Convenience 更快 |
| BatchParallel 4096 | 2.94 GiB/s | 3.49 GiB/s | Stateless 更快（大批量并行） |
