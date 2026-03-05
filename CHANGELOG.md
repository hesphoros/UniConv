# 更新日志

## v3.1.0 (2026-01-07)

### 代码清理
- 移除 deprecated API：`IConvResult`、`StringResultToIConvResult()`、`IConvResultToStringResult()`
- 移除 deprecated 方法：返回 `IConvResult` 的 `ConvertEncoding()` 重载
- 移除 `ConvertEncodingFastWithHint()`：自动估算已足够高效
- 清理未使用的 include (`<iostream>`、`<fstream>`) 和生产代码中的调试输出

### 迁移指南

| 旧 API | 新 API |
|---------|--------|
| `IConvResult` | `StringResult` (`CompactResult<std::string>`) |
| `ConvertEncoding()` 返回 IConvResult | `ConvertEncodingFast()` 返回 StringResult |
| `ConvertEncodingFastWithHint()` | `ConvertEncodingFast()` |

## v3.0.0 (2026-01)

### 性能优化
- **快速路径**：同编码直接返回、ASCII 零开销传递
- **SIMD 加速**：可选 simdutf 集成，UTF-8/16 转换 4~12.5x 加速
- **O(1) LRU**：双向链表 + 哈希表实现常数时间缓存
- **FNV-1a 哈希键**：减少缓存键内存分配
- **分层缓冲池**：32×4KB + 8×64KB + 2×1MB 三级缓冲区
- **无锁线程池**：自适应并行策略
- **智能估算**：编码感知扩展因子

### 新特性
- 批量并行转换 API：`ConvertEncodingBatchParallel()`
- 零拷贝转换：`ConvertEncodingZeroCopy()`
- 实例化转换器：`UniConvInstance` 类
- CPU 优化信息：`CpuOptimization::GetInfo()`

## v2.0 (2025-12)

- 实现无锁并发缓存（parallel-hashmap）
- 添加零拷贝 BufferLease 机制
- 实现并行批处理 API
- 添加 string_view 重载
- 性能提升：单次转换 1.77x，批处理 1.94x

## v1.0

- 初始版本
- 基础编码转换功能
- 线程安全支持
