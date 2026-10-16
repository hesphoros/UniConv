# UniConv API Reference

本文档是 UniConv 4.x 公共转换 API 的参考，覆盖编码转换、数值解析与格式化、native UTF 快速路径、批量转换和错误处理。接口以当前 include/UniConv/UniConv.h 为准。

~~~cpp
#include <UniConv/UniConv.h>
~~~

最低语言标准为 C++17。UNICONV_USE_SIMDUTF 只影响实现路径，不改变 UniConv 的公共 API；关闭 SIMD 时，native UTF 快速 API 仍可用并使用标量实现。

## 1. 接口选择

| 使用场景 | 推荐入口 | 返回形式 |
| --- | --- | --- |
| 单次任意编码转换，要求详细错误 | ConvertEncodingFast | StringResult |
| 单次任意编码转换，复用输出缓冲区 | ConvertEncodingFast(input, from, to, output) | ErrorCode |
| 单次转换，只需要成功/失败 | ConvertEncoding | bool |
| 不使用实例缓存/池 | ConvertEncodingStatelessFast | ErrorCode |
| 多条数据并行转换 | ConvertEncodingBatchParallel | vector<StringResult> 或 bool |
| 系统 locale 与 UTF-8 | ToUtf8FromLocale / ToLocaleFromUtf8 | 值返回或输出参数 |
| UTF-8 与本机 char16_t code unit | ToUtf16FromUtf8Fast / ToUtf8FromUtf16Fast | U16StringResult / StringResult |
| 数值文本解析 | ParseInt32 / ParseInt64 / ParseFloat / ParseDouble | CompactResult<T> |
| 数值格式化 | ToStringFromInt32 等 | CompactResult 或 ErrorCode |

## 2. 基本约定

### 2.1 实例和线程

~~~cpp
UniConv local;
auto owned = UniConv::Create();
UniConv& thread_local_converter = UniConv::ThreadLocal();
~~~

- UniConv() 创建一个独立实例，每个实例有自己的缓存和 API 层模式。
- UniConv::Create() 返回 std::unique_ptr<UniConv>。
- UniConv::ThreadLocal() 返回当前线程独占的实例；不要把该引用传到其他线程。
- UniConv 不可复制。

### 2.2 字符串类型和字节序

- 通用编码 API 中，std::string 是任意字节序列，编码由 fromEncoding/toEncoding 指定。
- UTF-8 专用 API 中，std::string 约定保存 UTF-8 字节。
- std::u16string 和 std::u32string 保存本机 char16_t/char32_t code unit，自身没有字节序标签。
- 名称带 LE/BE 的旧接口是 iconv 字节序接口。需要本机 UTF-16 code unit 语义时，优先使用第 9 节的 Fast API。
- std::wstring 的宽度由平台决定：Windows 通常为 UTF-16 code unit，Linux 和 macOS 通常为 32 位宽字符。UCS4 API 的调用方必须确认平台语义。

### 2.3 输入形态

| 形态 | 语义 | 是否支持嵌入 NUL |
| --- | --- | --- |
| const std::basic_string<CharT>& | 拥有长度的字符串 | 支持 |
| std::basic_string_view<CharT> | 非拥有视图 | 支持 |
| const CharT* | NUL 终止字符串 | 不支持完整的嵌入 NUL 数据 |
| const CharT*, size_t | 显式 code unit/byte 数量 | 支持 |

显式长度的单位取决于 API：native UTF-8 输入使用字节数，native UTF-16 输入使用 char16_t code unit 数量。不要把字节数和 code unit 数量混用。

### 2.4 结果类型

~~~cpp
using StringResult = CompactResult<std::string>;
using StringViewResult = CompactResult<std::string_view>;
using IntResult = CompactResult<int>;
using BoolResult = CompactResult<bool>;
using Int32Result = CompactResult<std::int32_t>;
using Int64Result = CompactResult<std::int64_t>;
using FloatResult = CompactResult<float>;
using DoubleResult = CompactResult<double>;
using U16StringResult = CompactResult<std::u16string>;
using U32StringResult = CompactResult<std::u32string>;
using WideStringResult = CompactResult<std::wstring>;
~~~

使用 CompactResult 时先检查状态：

~~~cpp
auto result = conv.ConvertEncodingFast(input, "GBK", "UTF-8");
if (!result.IsSuccess()) {
    ErrorCode error = result.GetErrorCode();
    return;
}
std::string utf8 = std::move(result).GetValue();
~~~

GetValue() 只应在成功状态下调用；ValueOr(default_value) 可用于安全地取得默认值。CompactResult 是 nodiscard 类型，不能忽略。

## 3. 错误码和失败语义

| 错误码 | 含义 |
| --- | --- |
| Success | 操作成功 |
| InvalidParameter | 空指针、非法参数或参数组合 |
| InvalidSourceEncoding | 源编码名称无效 |
| InvalidTargetEncoding | 目标编码名称无效 |
| ConversionFailed | 转换引擎报告未分类失败 |
| IncompleteSequence | 输入在多字节序列中途结束 |
| InvalidSequence | 输入包含非法字节、非法 UTF-8 或非法 UTF-16 序列 |
| OutOfMemory | 分配输出缓冲区失败 |
| BufferTooSmall | 内部输出上限不足 |
| FileNotFound / FileReadError / FileWriteError | 文件相关保留错误码 |
| InternalError | 内部迭代或状态错误 |
| EncodingNotFound | 代码页或编码映射不存在 |
| SystemError | 平台系统错误 |
| InvalidNumber | 数值语法错误 |
| NumericOverflow | 数值超出目标类型范围 |
| UnsupportedNumericBase | 整数进制不是支持的 10 或 16 |

旧式值返回接口没有错误码，失败和空输入通常都表现为空目标字符串。需要区分空结果与失败时，使用 Ex、ErrorCode + output 或 CompactResult 接口。输出参数调用失败时，不应继续消费 output 的内容；以返回码为准。

## 4. 编码和环境元数据

### 4.1 编码枚举和名称

UniConv::Encoding 由 include/UniConv/encodings.inc 生成，包含 ASCII、ISO-8859、Windows code page、GBK/GB18030、BIG5、Shift_JIS、EUC、UTF-7/8/16/32、UCS-2/UCS-4、平台宽字符和 EBCDIC 等编码，末项为 Encoding::count。

头文件还公开了 `BomEncoding`（None、UTF8、UTF16_LE、UTF16_BE、UTF32_LE、
UTF32_BE）枚举。当前 BOM 检测和移除函数是 `UniConv` 的私有辅助函数，没有独立
的公共 BOM 转换入口；调用方应先自行处理 BOM，或通过通用编码转换 API 处理完整
输入。

~~~cpp
std::string name = UniConv::ToString(UniConv::Encoding::utf_8);
~~~

常用名称常量为 ENC_UTF8、ENC_UTF16LE、ENC_UTF16BE、ENC_UTF32LE 和平台相关的 ENC_WCHAR。不要在调用方定义新的代码页宏。

### 4.2 系统编码和代码页

~~~cpp
static std::string GetCurrentSystemEncoding() noexcept;
static std::uint16_t GetCurrentSystemEncodingCodePage() noexcept;
static std::string GetEncodingNameByCodePage(std::uint16_t codePage) noexcept;
static const char* GetEncodingNamePtr(int codepage) noexcept;
static StringViewResult GetEncodingNameFast(int codepage) noexcept;
static IntResult GetSystemCodePageFast() noexcept;
~~~

- GetCurrentSystemEncoding() 返回平台当前 locale 编码名。
- GetCurrentSystemEncodingCodePage() 在 Windows 返回活动 ACP；Linux/macOS 能映射时返回代码页，无法映射时返回 0；其他平台可能返回 uint16_t(-1)。
- GetEncodingNameByCodePage() 找不到时返回文本 Encoding not found.。
- GetEncodingNamePtr() 找不到时返回 nullptr；返回指针由库持有，调用方不得释放或修改。
- GetEncodingNameFast() 找不到时返回 EncodingNotFound。
- GetSystemCodePageFast() 以 IntResult 返回失败原因。

SetDefaultEncoding(const std::string&) noexcept 可设置实例的默认编码状态，但现有 locale 便捷包装仍通过平台的 GetCurrentSystemEncoding() 获取编码；不要把它当作所有 locale API 的全局覆盖开关。

## 5. 通用编码转换

### 5.1 CompactResult 版本

~~~cpp
StringResult ConvertEncodingFast(const std::string& input,
                                 const char* fromEncoding,
                                 const char* toEncoding) noexcept;
~~~

这是通用转换的 CompactResult 版本。支持 encodings.inc 中的编码名称，内部按输入和目标选择 ASCII 快路径、simdutf 路径或 iconv 路径。空输入是成功并返回空字符串；空编码名返回 InvalidParameter，源/目标名称无效分别返回 InvalidSourceEncoding 或 InvalidTargetEncoding。

### 5.2 输出参数版本

~~~cpp
bool ConvertEncoding(const std::string& input,
                     const char* fromEncoding, const char* toEncoding,
                     std::string& output) noexcept;

ErrorCode ConvertEncodingFast(const std::string& input,
                              const char* fromEncoding, const char* toEncoding,
                              std::string& output) noexcept;

bool ConvertEncoding(std::string_view input,
                     const char* fromEncoding, const char* toEncoding,
                     std::string& output) noexcept;

ErrorCode ConvertEncodingFast(std::string_view input,
                              const char* fromEncoding, const char* toEncoding,
                              std::string& output) noexcept;
~~~

output 由调用方持有，成功时可复用其 capacity。bool 版本只保留成功/失败，需要诊断时使用 ErrorCode 版本。

### 5.3 无状态版本

~~~cpp
bool ConvertEncodingStateless(const std::string& input,
                              const char* fromEncoding, const char* toEncoding,
                              std::string& output) noexcept;
ErrorCode ConvertEncodingStatelessFast(const std::string& input,
                                       const char* fromEncoding, const char* toEncoding,
                                       std::string& output) noexcept;
bool ConvertEncodingStateless(std::string_view input,
                              const char* fromEncoding, const char* toEncoding,
                              std::string& output) noexcept;
ErrorCode ConvertEncodingStatelessFast(std::string_view input,
                                       const char* fromEncoding, const char* toEncoding,
                                       std::string& output) noexcept;
~~~

无状态层不使用实例的共享 iconv 描述符缓存和共享字符串池，适合需要明确隔离状态的路径。ApiLayerMode::Stateless 会让普通输出参数和 batch API 路由到该层。

## 6. 旧式专用转换 API

这些接口保持早期命名和返回形式。除特别注明的长度重载外，const CharT* 都是 NUL 终止版本；失败通常返回目标字符串的空值。

### 6.1 Locale 与 UTF-8

~~~cpp
std::string ToUtf8FromLocale(const std::string& input);
std::string ToUtf8FromLocale(const char* input);
std::string ToLocaleFromUtf8(const std::string& input);
std::string ToLocaleFromUtf8(const char* input);
~~~

### 6.2 Locale 与 UTF-16

~~~cpp
std::u16string ToUtf16LEFromLocale(const std::string& input);
std::u16string ToUtf16LEFromLocale(const char* input);
std::u16string ToUtf16BEFromLocale(const std::string& input);
std::u16string ToUtf16BEFromLocale(const char* input);
std::string ToLocaleFromUtf16LE(const std::u16string& input);
std::string ToLocaleFromUtf16LE(const char16_t* input);
std::string ToLocaleFromUtf16BE(const std::u16string& input);
std::string ToLocaleFromUtf16BE(const char16_t* input);
~~~

### 6.3 UTF-8 与 UTF-16

~~~cpp
std::string ToUtf8FromUtf16LE(const std::u16string& input);
std::string ToUtf8FromUtf16LE(const char16_t* input, size_t len);
std::string ToUtf8FromUtf16LE(const char16_t* input);
std::string ToUtf8FromUtf16BE(const std::u16string& input);
std::string ToUtf8FromUtf16BE(const char16_t* input, size_t len);
std::string ToUtf8FromUtf16BE(const char16_t* input);
std::u16string ToUtf16LEFromUtf8(const std::string& input);
std::u16string ToUtf16LEFromUtf8(const char* input);
std::u16string ToUtf16BEFromUtf8(const std::string& input);
std::u16string ToUtf16BEFromUtf8(const char* input);
~~~

只有 UTF-16 -> UTF-8 的旧式指针接口提供 pointer + length 形式；其余旧式指针接口是 NUL 终止形式。含 NUL 的数据应使用 string_view、显式长度或第 9 节的 native fast API。

### 6.4 UTF-16 字节序互转

~~~cpp
std::u16string ToUtf16BEFromUtf16LE(const std::u16string& input);
std::u16string ToUtf16BEFromUtf16LE(const char16_t* input);
std::u16string ToUtf16LEFromUtf16BE(const std::u16string& input);
std::u16string ToUtf16LEFromUtf16BE(const char16_t* input);
~~~

### 6.5 UTF-32 家族

~~~cpp
std::string ToUtf8FromUtf32LE(const std::u32string& input);
std::string ToUtf8FromUtf32LE(const char32_t* input);
std::u16string ToUtf16LEFromUtf32LE(const std::u32string& input);
std::u16string ToUtf16LEFromUtf32LE(const char32_t* input);
std::u16string ToUtf16BEFromUtf32LE(const std::u32string& input);
std::u16string ToUtf16BEFromUtf32LE(const char32_t* input);
std::u32string ToUtf32LEFromUtf8(const std::string& input);
std::u32string ToUtf32LEFromUtf8(const char* input);
std::u32string ToUtf32LEFromUtf16LE(const std::u16string& input);
std::u32string ToUtf32LEFromUtf16LE(const char16_t* input);
std::u32string ToUtf32LEFromUtf16BE(const std::u16string& input);
std::u32string ToUtf32LEFromUtf16BE(const char16_t* input);
~~~

### 6.6 宽字符串和 UCS-4

~~~cpp
std::wstring ToWideStringFromLocale(const std::string& input);
std::wstring ToWideStringFromLocale(const char* input);
std::string ToLocaleFromWideString(const std::wstring& input);
std::string ToLocaleFromWideString(const wchar_t* input);
std::string ToUtf8FromUcs4(const std::wstring& input);
std::wstring ToUcs4FromUtf8(const std::string& input);
std::wstring ToWStringFromU16String(const std::u16string& input);
std::wstring ToWStringFromU16String(const char16_t* input);
~~~

ToUtf8FromUcs4 和 ToUcs4FromUtf8 依赖平台 wchar_t 宽度，不能把 Windows 的 16 位 wstring 自动当作 Linux 风格 UCS-4 使用。

## 7. Ex 详细错误 API

Ex 接口只接受拥有长度的标准字符串，不提供 string_view 或 C 指针重载。所有接口返回 CompactResult<T>，空输入是成功的空目标字符串。

~~~cpp
CompactResult<std::string> ToUtf8FromLocaleEx(const std::string& input);
CompactResult<std::string> ToLocaleFromUtf8Ex(const std::string& input);
CompactResult<std::u16string> ToUtf16LEFromLocaleEx(const std::string& input);
CompactResult<std::u16string> ToUtf16BEFromLocaleEx(const std::string& input);
CompactResult<std::string> ToUtf8FromUtf16LEEx(const std::u16string& input);
CompactResult<std::string> ToUtf8FromUtf16BEEx(const std::u16string& input);
CompactResult<std::string> ToUtf8FromUtf32LEEx(const std::u32string& input);
CompactResult<std::u32string> ToUtf32LEFromUtf8Ex(const std::string& input);
CompactResult<std::u32string> ToUtf32LEFromUtf16LEEx(const std::u16string& input);
CompactResult<std::u32string> ToUtf32LEFromUtf16BEEx(const std::u16string& input);
CompactResult<std::u16string> ToUtf16LEFromUtf16BEEx(const std::u16string& input);
CompactResult<std::u16string> ToUtf16BEFromUtf16LEEx(const std::u16string& input);
CompactResult<std::wstring> ToWideStringFromLocaleEx(const std::string& input);
CompactResult<std::string> ToLocaleFromWideStringEx(const std::wstring& input);
CompactResult<std::string> ToUtf8FromUcs4Ex(const std::wstring& input);
CompactResult<std::wstring> ToUcs4FromUtf8Ex(const std::string& input);
CompactResult<std::wstring> ToWStringFromU16StringEx(const std::u16string& input);
~~~

示例：

~~~cpp
auto result = conv.ToUtf8FromUtf16LEEx(u16"你好");
if (result) {
    UseUtf8(result.GetValue());
} else if (result.GetErrorCode() == ErrorCode::InvalidSequence) {
    RejectInput();
}
~~~

## 8. 输出参数转换 API

输出参数版本适合循环转换和复用 capacity。以下专用方法均为 bool method(const Source&, Target& output) noexcept。

| 方向 | 方法 |
| --- | --- |
| locale -> UTF-8 | ToUtf8FromLocale |
| UTF-8 -> locale | ToLocaleFromUtf8 |
| UTF-16LE/BE -> UTF-8 | ToUtf8FromUtf16LE, ToUtf8FromUtf16BE |
| UTF-32LE -> UTF-8 | ToUtf8FromUtf32LE |
| UTF-8 -> UTF-16LE/BE | ToUtf16LEFromUtf8, ToUtf16BEFromUtf8 |
| locale -> UTF-16LE/BE | ToUtf16LEFromLocale, ToUtf16BEFromLocale |
| UTF-16LE <-> UTF-16BE | ToUtf16BEFromUtf16LE, ToUtf16LEFromUtf16BE |
| UTF-8 -> UTF-32LE | ToUtf32LEFromUtf8 |
| UTF-16LE/BE -> UTF-32LE | ToUtf32LEFromUtf16LE, ToUtf32LEFromUtf16BE |
| UTF-32LE -> UTF-16LE/BE | ToUtf16LEFromUtf32LE, ToUtf16BEFromUtf32LE |
| UTF-16LE/BE -> locale | ToLocaleFromUtf16LE, ToLocaleFromUtf16BE |
| wide -> locale | ToLocaleFromWideString |
| UCS-4/wide <-> UTF-8 | ToUtf8FromUcs4, ToUcs4FromUtf8 |
| locale -> wide | ToWideStringFromLocale |
| UTF-16LE -> wide | ToWStringFromU16String |

示例：

~~~cpp
std::string output;
bool ok = conv.ToUtf8FromUtf16LE(input16, output);
if (!ok) {
    output.clear();
}
~~~

### 8.1 string_view 输入

以下专用方法提供 string_view 输入和输出参数：

~~~cpp
bool ToUtf8FromLocale(std::string_view, std::string&) noexcept;
bool ToLocaleFromUtf8(std::string_view, std::string&) noexcept;
bool ToUtf16LEFromUtf8(std::string_view, std::u16string&) noexcept;
bool ToUtf16BEFromUtf8(std::string_view, std::u16string&) noexcept;
bool ToUtf32LEFromUtf8(std::string_view, std::u32string&) noexcept;
bool ToUtf16LEFromLocale(std::string_view, std::u16string&) noexcept;
bool ToUtf16BEFromLocale(std::string_view, std::u16string&) noexcept;
~~~

通用 ConvertEncoding/ConvertEncodingFast 也提供 string_view 输入的输出参数版本；没有 u16string_view、u32string_view 或 wstring_view 的专用输出重载。

## 9. Native UTF-8/UTF-16 快速 API

这组 API 不使用 LE/BE 名称，因为它们转换的是本机 char16_t code unit，不是一个外部字节序的 UTF-16 byte buffer。

### 9.1 视图和输出参数

~~~cpp
U16StringResult ToUtf16FromUtf8Fast(std::string_view input) noexcept;
StringResult ToUtf8FromUtf16Fast(std::u16string_view input) noexcept;
ErrorCode ToUtf16FromUtf8Fast(std::string_view input,
                              std::u16string& output) noexcept;
ErrorCode ToUtf8FromUtf16Fast(std::u16string_view input,
                              std::string& output) noexcept;
~~~

启用 simdutf 且输入达到实现阈值时会走 SIMD；否则走经过验证的标量路径。两条路径都验证 UTF-8/UTF-16，非法序列返回 InvalidSequence，不会返回看似成功的空结果。

### 9.2 显式长度指针

~~~cpp
U16StringResult ToUtf16FromUtf8Fast(const char* input,
                                    size_t byte_count) noexcept;
StringResult ToUtf8FromUtf16Fast(const char16_t* input,
                                 size_t code_unit_count) noexcept;
ErrorCode ToUtf16FromUtf8Fast(const char* input, size_t byte_count,
                              std::u16string& output) noexcept;
ErrorCode ToUtf8FromUtf16Fast(const char16_t* input, size_t code_unit_count,
                              std::string& output) noexcept;
~~~

### 9.3 NUL 终止指针

~~~cpp
U16StringResult ToUtf16FromUtf8Fast(const char* input) noexcept;
StringResult ToUtf8FromUtf16Fast(const char16_t* input) noexcept;
ErrorCode ToUtf16FromUtf8Fast(const char* input, std::u16string& output) noexcept;
ErrorCode ToUtf8FromUtf16Fast(const char16_t* input, std::string& output) noexcept;
~~~

空指针返回 InvalidParameter；显式长度为零的非空指针是成功的空转换。

## 10. 批量和并行转换

### 10.1 串行 batch

~~~cpp
std::vector<StringResult> ConvertEncodingBatch(
    const std::vector<std::string>& inputs,
    const char* fromEncoding, const char* toEncoding) noexcept;

bool ConvertEncodingBatch(
    const std::vector<std::string>& inputs,
    const char* fromEncoding, const char* toEncoding,
    std::vector<std::string>& outputs) noexcept;
~~~

两种版本都让输出数量与输入数量一致。结果版本为每一项保留独立的 StringResult 和错误码；输出参数版本返回 false 表示至少一项失败。

### 10.2 并行 batch

~~~cpp
std::vector<StringResult> ConvertEncodingBatchParallel(
    const std::vector<std::string>& inputs,
    const char* fromEncoding, const char* toEncoding,
    size_t numThreads = 0) noexcept;

bool ConvertEncodingBatchParallel(
    const std::vector<std::string>& inputs,
    const char* fromEncoding, const char* toEncoding,
    std::vector<std::string>& outputs,
    size_t numThreads = 0) noexcept;
~~~

numThreads == 0 使用线程池默认线程数；库会根据条数和总字节数决定串行、轻量并行或完整并行。小批量强制并行通常会增加调度开销。

## 11. Native 数值转换 API

数值 API 是静态函数，不读取 locale，也不依赖 UniConv 实例缓存。输入语法只接受 ASCII 数字文本。

### 11.1 整数选项

~~~cpp
enum class IntegerBase : std::uint8_t {
    Auto = 0,
    Decimal = 10,
    Hexadecimal = 16
};

struct IntegerParseOptions {
    IntegerBase base = IntegerBase::Auto;
};
~~~

解析规则：

- 允许一个前导 + 或 -。
- Auto 识别 0x/0X 为十六进制，否则按十进制。
- Decimal 拒绝 0x 前缀；Hexadecimal 可有或没有该前缀。
- 不支持八进制、二进制、空白、下划线和尾随字符。
- 必须完整消费输入；溢出返回 NumericOverflow。

### 11.2 整数解析

ParseInt32 和 ParseInt64 各有四种 view 重载：std::string_view、std::u16string_view、std::u32string_view 和 std::wstring_view。

~~~cpp
Int32Result ParseInt32(std::string_view, IntegerParseOptions = {}) noexcept;
Int64Result ParseInt64(std::string_view, IntegerParseOptions = {}) noexcept;
~~~

四种字符类型各有对应的显式长度和 NUL 指针版本：

~~~cpp
Int32Result ParseInt32(const CharT*, size_t code_units,
                       IntegerParseOptions = {}) noexcept;
Int64Result ParseInt64(const CharT*, size_t code_units,
                       IntegerParseOptions = {}) noexcept;
Int32Result ParseInt32(const CharT*, IntegerParseOptions = {}) noexcept;
Int64Result ParseInt64(const CharT*, IntegerParseOptions = {}) noexcept;
~~~

CharT 为 char、char16_t、char32_t 或 wchar_t。带长度版本支持嵌入 NUL。

### 11.3 浮点选项和解析

~~~cpp
struct FloatingParseOptions {
    bool allow_non_finite = false;
};
~~~

ParseFloat 和 ParseDouble 与整数解析具有完全相同的四种 view、四种显式长度指针和四种 NUL 指针重载：

~~~cpp
FloatResult ParseFloat(std::string_view,
                       FloatingParseOptions = {}) noexcept;
DoubleResult ParseDouble(std::string_view,
                         FloatingParseOptions = {}) noexcept;
~~~

支持十进制小数和科学计数法，例如 -1.25、6.02e23。默认拒绝 nan、inf 和 infinity；设置 allow_non_finite = true 后允许这些值。空白、十六进制浮点文本和尾随字符返回 InvalidNumber，超出目标类型范围返回 NumericOverflow。

### 11.4 数值格式化

~~~cpp
struct FloatingFormatOptions {
    std::chars_format format = std::chars_format::general;
    int precision = -1;
    bool allow_non_finite = true;
};
~~~

支持 general、fixed、scientific 和 hex。precision == -1 请求最短 round-trip 表示；非负值传给 std::to_chars。allow_non_finite == false 时，格式化 NaN/无穷返回 InvalidParameter。

每个 int32_t/int64_t、float/double 组合都提供四个目标类型，整数接口例如：

~~~cpp
StringResult ToStringFromInt32(std::int32_t value) noexcept;
StringResult ToStringFromInt64(std::int64_t value) noexcept;
U16StringResult ToUtf16StringFromInt32(std::int32_t value) noexcept;
U16StringResult ToUtf16StringFromInt64(std::int64_t value) noexcept;
U32StringResult ToUtf32StringFromInt32(std::int32_t value) noexcept;
U32StringResult ToUtf32StringFromInt64(std::int64_t value) noexcept;
WideStringResult ToWideStringFromInt32(std::int32_t value) noexcept;
WideStringResult ToWideStringFromInt64(std::int64_t value) noexcept;
~~~

浮点版本在每个方法上增加 FloatingFormatOptions options = {}：

~~~cpp
StringResult ToStringFromFloat(float, FloatingFormatOptions = {}) noexcept;
StringResult ToStringFromDouble(double, FloatingFormatOptions = {}) noexcept;
U16StringResult ToUtf16StringFromFloat(float, FloatingFormatOptions = {}) noexcept;
U16StringResult ToUtf16StringFromDouble(double, FloatingFormatOptions = {}) noexcept;
U32StringResult ToUtf32StringFromFloat(float, FloatingFormatOptions = {}) noexcept;
U32StringResult ToUtf32StringFromDouble(double, FloatingFormatOptions = {}) noexcept;
WideStringResult ToWideStringFromFloat(float, FloatingFormatOptions = {}) noexcept;
WideStringResult ToWideStringFromDouble(double, FloatingFormatOptions = {}) noexcept;
~~~

每个返回值版本都有一个对应的 ErrorCode + output 重载：整数为 ToStringFromInt32/64(value, output)，浮点为同名方法加上 FloatingFormatOptions 参数；目标输出类型分别是 std::string、std::u16string、std::u32string 和 std::wstring。

~~~cpp
auto parsed = UniConv::ParseInt64("0x7fff", {IntegerBase::Auto});
if (parsed) {
    auto text = UniConv::ToStringFromInt64(parsed.GetValue());
}

FloatingFormatOptions format;
format.format = std::chars_format::fixed;
format.precision = 3;
std::string text;
ErrorCode ec = UniConv::ToStringFromDouble(3.1415926, text, format);
~~~

## 12. API 层模式和性能接口

~~~cpp
enum class ApiLayerMode : std::uint8_t {
    Convenience = 0,
    Stateless = 1
};

void SetApiLayerMode(ApiLayerMode mode) noexcept;
ApiLayerMode GetApiLayerMode() const noexcept;
UniConv::PoolStats GetPoolStatistics() const;
~~~

Convenience 是默认模式，允许使用实例缓存和字符串池；Stateless 让普通输出参数和 batch 转换走无共享缓存的路径。模式只影响实例调用，不影响静态数值 API 和 native UTF fast API。

PoolStats 提供 active buffer、转换次数、缓存命中率、iconv cache 大小、命中、未命中和逐出计数，适合诊断，不应作为业务正确性的依据。

## 13. 完整用法

### 13.1 任意编码转换

~~~cpp
UniConv& conv = UniConv::ThreadLocal();
std::string utf8;
ErrorCode ec = conv.ConvertEncodingFast(gbk_bytes, "GBK", "UTF-8", utf8);
if (ec != ErrorCode::Success) {
    return;
}
~~~

### 13.2 二进制安全的 UTF 快速转换

~~~cpp
std::string_view bytes(buffer, byte_count);
auto utf16 = UniConv::ToUtf16FromUtf8Fast(bytes);
if (utf16) {
    ConsumeUtf16(utf16.GetValue());
}
~~~

### 13.3 循环复用输出

~~~cpp
std::string output;
output.reserve(4096);
for (const std::string& item : items) {
    if (conv.ToUtf8FromLocale(item, output)) {
        Consume(output);
    }
}
~~~

### 13.4 批量并行

~~~cpp
std::vector<StringResult> results =
    conv.ConvertEncodingBatchParallel(items, "CP936", "UTF-8", 0);
for (const auto& result : results) {
    if (result) {
        Consume(result.GetValue());
    }
}
~~~

## 14. 迁移和注意事项

1. 新代码优先使用 ConvertEncodingFast 的 StringResult 或 ErrorCode + output 版本，不要用空字符串判断旧式接口是否失败。
2. 需要处理嵌入 NUL 的数据时使用 string_view 或显式长度指针；NUL 终止重载只适用于 C 字符串。
3. 不要把 ToUtf16LEFromUtf8 的 u16string 与 ToUtf16FromUtf8Fast 的本机 UTF-16 code unit 语义混为一谈。
4. 数值解析不会跳过空白，也不会读取 locale；调用方应在调用前明确清理输入。
5. std::u8string 不是 C++17 公共必需类型。C++20 char8_t 数据需要调用方显式转换为 char 字节序列。
6. simdutf 开关只改变性能路径；正确性、错误码和 API 名称不应依赖是否启用 SIMD。
7. 输出参数失败时以返回值为准，不要继续消费失败调用留下的 output 内容。

