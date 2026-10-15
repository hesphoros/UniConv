# UniConv 公共宏设计方案

状态：重构后设计稿（4.0 目标态）

本文是 UniConv 公共宏接口的重构方案，目标是为后续实现、测试和文档提供单一依据。本文只描述接口、边界和行为，不代表当前仓库已经实现这些宏。除非显式标注“当前仓库基线”，本文中的“应/必须”均指向 4.0 目标态。

## 1. 结论摘要

本方案的目标发布版本为 UniConv 4.0。第一版仍采用单头文件方案：公共宏直接放入 include/UniConv/UniConv.h，暂不新增 Macros.hpp 或 Macros/ 目录。

首版的核心原则：

1. 公开接口保持最小，只承诺确有用户价值且能长期维护的宏。
2. 不提供任何数值代码页常量宏；现有运行时编码映射和转换 API 保持不变。
3. 4.0 不因为包含 UniConv.h 自动改变调用方翻译单元的执行字符集；仅为明确标记的 3.x 兼容模式保留临时例外。
4. 不在宏中隐藏 char8_t 到 char 的转换。
5. 检测宏、配置宏、内部实现宏必须分层，避免把实现细节变成长期公共 API。
6. 语言能力宏不得改变导出类布局、导出函数签名或外部链接 inline 定义。

建议的首版公共接口：

| 类别 | 名称 | 说明 |
|---|---|---|
| 标准检测 | UNICONV_CXX_STANDARD | 使用 201703L、202002L 等标准值 |
| 能力检测 | UNICONV_HAS_CHAR8_T | 固定为 0/1，按 __cpp_char8_t 判断 |

首版不发布 UNICONV_U8_LITERAL。普通情况下，直接书写 u8"..." 或使用普通窄字面量并配置 UTF-8 编译选项更清晰；只有未来出现无法由标准语法覆盖的宏参数转发需求时，才另行提案。

## 2. 当前仓库基线

### 2.1 公开入口

当前 README、示例和测试均使用：

    #include <UniConv/UniConv.h>

CMake 安装规则当前以 UniConv.h 为主公共入口，同时还安装 encodings.inc、include/iconv 目录及构建生成的 include/iconv/config.h（启用 simdutf 时还会安装 simdutf.h）。因此第一版宏直接并入 UniConv.h 仍可避免新增宏专用安装入口、打包规则和用户迁移成本。

这也意味着首版明确接受一个取舍：即使用户只想使用检测宏，也会包含 UniConv 的完整主头文件。只有当宏的使用场景明显多于 UniConv 主 API 使用场景时，才拆出轻量宏头文件。

### 2.2 已存在的相关行为

当前 UniConv.h 已经包含以下相关内容：

- MSVC 的 execution_character_set pragma。
- UNICONV_* 性能和编译器属性宏。
- 没有前缀的 CPP_STANDARD C++ 版本宏。
- Windows、Linux、macOS 等条件编译。
- std::string、std::string_view、std::u16string、std::u32string 和 std::wstring API。

新方案应复用这些事实，避免新增同义宏或改变既有转换行为。编码名称、运行时编码映射和既有整数参数 API 属于现有功能，不在本宏方案中重新设计。

## 3. 范围与非目标

### 3.1 本方案解决的问题

- 提供可靠的 C++ 标准版本检测。
- 提供 char8_t 能力检测。
- 说明标准 UTF-8 字面量的跨标准类型语义。
- 明确执行字符集属于构建系统职责，并为现有 MSVC pragma 制定退出路径。
- 规定宏命名、值域、可覆盖性、兼容和测试策略。
- 约束混合 C++ 标准编译时的 ABI 和 ODR 风险。

### 3.2 本方案不解决的问题

这些宏不负责：

- 修改操作系统 locale 或用户区域设置。
- 修改 Windows、Linux、macOS 或其他系统的默认编码。
- 修改控制台、终端、文件系统或网络协议的编码。
- 保证 std::string 自动成为 UTF-8。
- 将 char8_t 自动转换成 char。
- 将任意字符串内容进行运行时转码。
- 替代 ConvertEncoding* 等运行时 API。
- 提供数值代码页常量或新的代码页选择语义。

### 3.3 术语约定

本文统一使用“字符集”描述源文件字符集、执行字符集及相关构建配置语义；“字符串”仅指 C++ 类型和值（如 std::string、u8 字面量）。本文不使用“字符串集”指代字符集，以避免语义混淆。

## 4. 宏分层与公共契约

宏应分为四类，且每类具有不同的兼容承诺。

### 4.1 用户配置宏

新增用户配置宏使用 UNICONV_CONFIG_ 前缀。它们必须在包含头文件前定义，并且文档明确默认值、作用范围和版本策略。

以下是现有的历史配置/构建宏，暂不在本方案中重命名：

- UNICONV_DLL
- UNICONV_DLL_IMPORT
- UNICONV_NO_THREAD_LOCAL

它们不满足新的 UNICONV_CONFIG_ 命名约定，但属于已有兼容面。新增配置项若确有必要，应使用 UNICONV_CONFIG_ 前缀，不能使用无前缀的通用名称。

### 4.2 公共只读检测宏

公共检测宏由头文件根据当前翻译单元计算，用户不得预定义或覆盖。布尔能力宏必须始终被定义为明确的 0 或 1，调用方使用 #if，不使用 #ifdef；UNICONV_CXX_STANDARD 则保留标准版本的长整数值。

所有由 UniConv 拥有并定义的新增公共宏名称都应在定义前检查是否被调用方预定义；若发现冲突，直接报错而不是静默覆盖。首版该规则适用于 UNICONV_CXX_STANDARD 和 UNICONV_HAS_CHAR8_T。

公共宏的值只描述当前消费翻译单元，不描述库文件是用什么标准或编译器构建的。

### 4.3 内部检测宏

编译器、平台和工具链事实默认使用 UNICONV_DETAIL_ 前缀，例如：

- UNICONV_DETAIL_COMPILER_CLANG
- UNICONV_DETAIL_COMPILER_MSVC_CL
- UNICONV_DETAIL_COMPILER_CLANG_CL
- UNICONV_DETAIL_TOOLCHAIN_MINGW
- UNICONV_DETAIL_PLATFORM_WINDOWS

这些宏只供 UniConv 内部实现使用，不承诺长期兼容。若确实出现稳定的外部使用场景，再单独评估是否提升为公共 API。

### 4.4 兼容宏

现有的 CPP_STANDARD 属于历史兼容宏，不再作为新实现的依据：

- 新代码统一使用 UNICONV_CXX_STANDARD。
- 3.x 兼容期间可保留别名，但必须进行冲突检测。
- 别名应记录弃用版本和移除版本。
- 4.0 删除该通用名称。

## 5. 头文件布局与卫生要求

### 5.1 推荐结构

include guard 必须位于所有 pragma 和声明之前：

    #ifndef UNICONV_UNICONV_H_INCLUDED
    #define UNICONV_UNICONV_H_INCLUDED

    // C++ standard and capability detection
    // Internal compiler/platform detection
    // Existing UniConv public and internal macros

    #if defined(_MSC_VER)
    // Scoped warning pragmas, if still required
    #endif

    // System and C++ standard library includes
    // Existing UniConv declarations and implementation details

    #if defined(_MSC_VER)
    // Matching warning pop
    #endif

    #endif

该结构只描述 4.0。若头文件卫生修复回移到 3.x，legacy execution-charset 兼容块也必须位于 include guard 内；该兼容块不进入 4.0。

### 5.2 必须修正的问题

1. __UNICONV_H__ 使用保留标识符，应改为 UNICONV_UNICONV_H_INCLUDED。
2. warning(push) 和 warning(pop) 必须位于 guard 内并严格配对。
3. 所有 _MSC_VER 判断使用 defined(_MSC_VER)，避免 -Wundef 警告。
4. 不应无条件禁用 MSVC C4819。启用 UTF-8 编译选项后，静默该警告会掩盖源文件编码问题。
5. UNICONV_NO_THREAD_LOCAL 应在保留现有 DLL 自动推导逻辑后补默认值：Windows DLL/DLL_IMPORT 场景仍优先得到 1，其他未配置场景再默认为 0；否则 #if UNICONV_NO_THREAD_LOCAL 在严格警告模式下可能报警。
6. 新宏的辅助名称使用 UNICONV_DETAIL_ 前缀，不复用已有的 UNICONV_STRINGIFY 等宏。
7. 能够在完成预处理任务后取消定义的内部辅助宏，应在适当位置取消定义；被公共宏后续展开依赖的辅助宏不能提前取消。

### 5.3 是否拆出独立宏头文件

第一版不拆分，原因是可以保持安装规则、FetchContent 和现有 include 路径不变。

只有在出现以下需求时再拆出 include/UniConv/Macros.h：

- 用户需要宏但不希望引入 iconv.h、parallel-hashmap 和线程相关头文件。
- 宏接口需要独立版本管理。
- 其他 UniConv 组件也需要复用这些宏。
- 安装、vcpkg 和 FetchContent 可以同时维护两个公开入口。

拆分时应让 UniConv.h 包含 Macros.h，保证现有用户代码不变。

## 6. C++ 标准和能力检测

### 6.1 UNICONV_CXX_STANDARD

UNICONV_CXX_STANDARD 保留编译器标准宏的数值语义，而不是转换为 17、20 这样的短整数：

    #if defined(_MSC_VER) && defined(_MSVC_LANG)
    #  define UNICONV_CXX_STANDARD _MSVC_LANG
    #elif defined(__cplusplus)
    #  define UNICONV_CXX_STANDARD __cplusplus
    #else
    #  define UNICONV_CXX_STANDARD 0L
    #endif

UniConv 的最低标准是 C++17，应在公共头文件尽早失败：

    #if UNICONV_CXX_STANDARD < 201703L
    #  error "UniConv requires C++17 or later"
    #endif

标准判断必须使用大于等于关系，以覆盖 C++23 及后续标准。首版不再公开 UNICONV_HAS_CPP17 或 UNICONV_HAS_CPP20；调用方直接比较 UNICONV_CXX_STANDARD。

如果 3.x 必须保留 CPP_STANDARD，应保证其值与 UNICONV_CXX_STANDARD 一致，并对调用方预定义的冲突值报错，而不是静默接受。新头文件还应在定义 UNICONV_CXX_STANDARD、UNICONV_HAS_CHAR8_T 等只读宏前执行预定义冲突检查。

### 6.2 UNICONV_HAS_CHAR8_T

char8_t 能力必须按特性宏判断：

    #if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    #  define UNICONV_HAS_CHAR8_T 1
    #else
    #  define UNICONV_HAS_CHAR8_T 0
    #endif

不能仅根据 C++20 判断，因为 MSVC、Clang 等编译器可以通过选项关闭 char8_t。

### 6.3 编译器和平台事实

编译器、前端、工具链和平台检测只作为内部实现事实：

- _MSC_VER 表示 MSVC 兼容 ABI，不等同于原生 Microsoft cl。
- __clang__ 和 _MSC_VER 可以同时成立，clang-cl 不应被误判为原生 cl。
- __GNUC__ 判断需要排除 Clang 前端。
- MinGW、Windows、Linux、Apple 等判断全部使用 defined(...)。
- __ANDROID__ 不应在正式支持和验证前作为稳定公共 API。
- UNICONV_DETAIL_PLATFORM_APPLE 表示 Apple 平台，不等同于 macOS。

如果这些事实未来提升为公共 API，必须单独定义互斥关系、版本承诺和测试矩阵。

## 7. 执行字符集策略

### 7.1 正式路径

不新增公共执行字符集动作宏或能力宏。Microsoft 已将 execution_character_set pragma 标记为 Visual Studio 2015 Update 2 之后的过时接口，正式路径应使用构建选项：

- MSVC 使用 /utf-8，或按需分别设置 /source-charset:utf-8 与 /execution-charset:utf-8。
- GCC/Clang 使用对应的源文件字符集和执行字符集选项。
- u8 字面量不依赖 execution_character_set pragma。

4.0 不应在包含 UniConv.h 时自动执行 pragma，也不提供 opt-in 开关。该指令会影响同一翻译单元中后续的普通窄字面量，属于顺序相关且不可局部撤销的副作用。

### 7.2 编译器和构建系统职责

| 环境 | 推荐构建配置 | 说明 |
|---|---|---|
| MSVC | /utf-8 | 同时指定源文件和执行字符集 |
| GCC | -finput-charset=UTF-8 -fexec-charset=UTF-8 | 由构建系统统一配置 |
| Clang/AppleClang | 按工具链文档和实测配置 UTF-8 相关选项 | 部分平台对 -fexec-charset 支持或行为有限，不依赖头文件 pragma |

对 GCC/Clang 相关字符集选项应以项目目标平台的编译器文档和 CI 实测结果为准，不把某一组 flag 在所有前端和平台上的行为视为固定事实。

pragma 只影响后续普通窄字面量的编译期表示，不保证：

- 源文件已经按 UTF-8 正确读取；
- std::string 自动带有编码类型；
- 系统 locale、终端或文件系统编码改变；
- iconv 自动识别输入编码。

当前 CMake 中的字符集选项应至少改为 target-scoped。库本身的编译选项和消费方源文件的编码选项是两个责任边界，不能因为库使用了 /utf-8 就假定上层项目也会继承。

### 7.3 3.x 兼容策略

4.0 直接移除自动 pragma，不提供 legacy 配置宏。若将本方案的部分头文件卫生修复回移到 3.x 维护分支，才使用以下临时兼容规则：

1. 3.x 兼容发布中，UNICONV_CONFIG_LEGACY_AUTO_EXECUTION_CHARSET 未定义时默认值为 1，以保持既有行为。
2. 用户在包含头文件前定义为 0 时，必须关闭自动应用；用户显式配置优先于默认值。
3. 该兼容宏只控制旧的内部自动行为，不形成新的公共动作接口。
4. 4.0 直接移除兼容宏和自动行为，不经历默认值改为 0 的过渡阶段。
5. 在文档中明确 3.x 默认值为临时例外；新代码和新示例只使用构建系统选项。

不能同时保留两份独立的 pragma 实现。

## 8. UTF-8 字面量策略

### 8.1 首版范围

4.0 首版不提供字面量包装宏，调用方直接使用：

    u8"UTF-8 text"

同样不提供 W_LITERAL、U16_LITERAL 和 U32_LITERAL。这些前缀已经是标准语法，而且 L 还具有平台相关宽度语义，包装宏不能消除这种差异。

未来只有出现标准语法无法覆盖的宏参数转发需求时，才单独评审 UNICONV_U8_LITERAL；它不属于本次实施范围。

本文不为未来候选宏预先规定名称、参数或展开方式；若出现真实需求，应另立提案并重新评审，不能把本节当作保留接口。

### 8.2 C++17/C++20 类型契约

    decltype(u8"x")

decltype 对该字符串字面量得到的是数组引用：C++17 为 const char (&)[N]；C++20 且 UNICONV_HAS_CHAR8_T 为 1 时为 const char8_t (&)[N]；C++20 但关闭 char8_t 时仍为 const char (&)[N]。若需要数组类型，应先使用 std::remove_reference_t<decltype(...)>。使用：

    auto text = u8"x";

时才会发生数组到指针的衰减，分别得到 const char* 或 const char8_t*。

因此不能把 u8 字面量描述成始终产生 const char*，也不能保证它直接适配当前的 std::string_view API。未来若增加包装宏，也必须保持完全相同的类型契约。

### 8.3 不提供隐式桥接

首版不提供 UNICONV_UTF8_VIEW，也不在宏中使用 reinterpret_cast 把 char8_t 隐式伪装成 char。

未来若确实需要将 std::u8string_view 传入现有字节 API，应优先设计：

- 具名的非拥有适配函数；
- 明确的 std::u8string_view 重载；
- 单独的字节视图类型。

若新增适配能力，按上述顺序推进：只有当前一层方案无法满足真实用例时，才进入下一层。

适配器必须明确“不转码、不拥有、不验证 UTF-8”，不能通过宏隐藏类型转换。

## 9. 两种 UTF-8 使用路线

方案必须明确区分以下两条路线：

| 路线 | 写法 | 优点 | 限制 | 当前建议 |
|---|---|---|---|---|
| 标准 UTF-8 字面量 | u8"..." | 明确表达 UTF-8 code unit | C++20 可能是 char8_t[]，不能直接传当前 char API | 需要类型语义时使用 |
| 普通窄字面量 | "..." 加 UTF-8 构建选项 | 保持 char[]，可直接传现有 API | 正确性依赖消费方构建配置 | 当前 std::string API 的推荐路线 |

两条路线不能通过宏悄悄混合。源文件读取编码、执行字符集和运行时字符串类型是不同层次的问题。

## 10. 与现有宏和 API 的关系

新宏不能覆盖或重命名现有的：

- UNICONV_EXPORT
- UNICONV_LIKELY / UNICONV_UNLIKELY
- UNICONV_PURE / UNICONV_CONST
- UNICONV_PRAGMA / UNICONV_STRINGIFY
- UNICONV_DEBUG_MODE
- UNICONV_NO_THREAD_LOCAL

宏重构不应批量替换现有所有 _WIN32、__linux__ 条件，也不应借此修改运行时转换、locale 处理、缓存或线程逻辑。

现有编码名称和转换函数继续按当前 API 使用。宏层不新增数值编码标识，不改变错误处理和编码识别规则。

## 11. ABI、ODR 和混合标准约束

能力宏按每个消费翻译单元计算，库文件和消费方可能使用不同的 C++ 标准。因此：

1. 检测宏不能改变导出类布局、成员顺序、虚函数表或导出函数签名。
2. 不能用 UNICONV_HAS_CHAR8_T 条件编译出不同的外部链接 inline 函数定义。
3. 不能让公共模板在 C++17 和 C++20 中产生不兼容的 ABI。
4. current_cpp_standard() 这类根据翻译单元标准改变函数体的 inline 函数，应删除、改为内部链接诊断函数，或移到统一标准编译的源文件。
5. UNICONV_CONSTEVAL 等会改变函数声明或语义的宏，只能用于不参与跨标准 ABI 的内部实现，并必须做混合标准测试。

必须增加“库以 C++17 构建、消费方以 C++20 构建”的编译和链接测试，不能只测试整个项目统一使用 C++17 或 C++20。

## 12. CMake 和构建配置

CMake 应使用 target-scoped 设置：

    target_compile_features(UniConv PUBLIC cxx_std_17)

    if(MSVC)
        target_compile_options(UniConv PRIVATE /utf-8)
    endif()

不建议使用目录级的全局 add_compile_options(/utf-8)。库的私有源文件配置不应无条件污染父项目。

如果 UniConv 的公共 API 要求调用方源文件中的窄字面量是 UTF-8，应在使用文档中要求调用方自行配置对应编译选项，而不是依赖头文件隐式修改状态。

## 13. 验证矩阵

### 13.1 编译器和标准

| 编译器 | 前端/工具链 | C++17 | C++20 |
|---|---|---:|---:|
| MSVC | 原生 cl | 必测 | 必测 |
| LLVM | clang++ | 必测 | 必测 |
| LLVM | clang-cl | 必测 | 必测 |
| GCC | GCC | 必测 | 必测 |
| GCC/LLVM | MinGW | 建议 | 建议 |
| Apple | AppleClang | 必测 | 必测 |

C++20 需要分别覆盖 char8_t 开启和关闭的配置，例如 MSVC /Zc:char8_t- 或对应 Clang 选项。

### 13.2 预处理和头文件卫生

至少验证：

1. 布尔检测宏全部已定义且值为 0/1；UNICONV_CXX_STANDARD 单独按 201703L、202002L 等标准值验证。
2. UNICONV_CXX_STANDARD 与实际编译模式一致。
3. UNICONV_HAS_CHAR8_T 与 __cpp_char8_t 一致，并覆盖 C++20 关闭 char8_t 的配置。
4. 使用 -Wundef -Werror 编译不产生未定义宏警告。
5. MSVC 使用 /W4 /WX 编译不产生新增警告。
6. 重复包含和不同 include 顺序不会造成 warning push/pop 失配。
7. 用户预定义任一新增公共宏名称时不会被静默接受。
8. UniConv 不新增任何 UNICONV_CP_* 数值代码页宏；操作系统 SDK 自带宏不在此限制内。

### 13.3 字面量和执行字符集

至少验证：

1. 对 decltype(u8"x") 先使用 std::remove_reference_t，再用 std::remove_extent_t 和 std::extent_v 区分 C++17 的 char[]、C++20 的 char8_t[]，以及 C++20 关闭 char8_t 时的 char[]。
2. 数组长度包含终止符，auto 衰减后的指针类型符合预期。
3. 普通字符串和 raw literal 均能通过编译；本版不测试未发布的字面量包装宏。
4. 4.0 的头文件不包含 execution_character_set pragma，也不定义 UNICONV_CONFIG_LEGACY_AUTO_EXECUTION_CHARSET。

仅当头文件卫生修复回移到 3.x 维护分支时，额外验证：

1. MSVC 的 legacy 自动 pragma 仅在兼容配置为 1 时执行，配置为 0 时不执行。
2. GCC、Clang 和 clang-cl 不进入仅供原生 MSVC 使用的 legacy pragma 分支。
3. legacy pragma 的编译测试不使用项目已有的全局 UTF-8 选项，避免测试结果被配置掩盖。

### 13.4 混合构建

至少包含：

- UniConv 库使用 C++17、消费方使用 C++20；
- UniConv 库使用 C++20、消费方使用 C++17（若工具链允许）；
- 多翻译单元分别使用不同标准；
- 公共 API 的类布局、函数签名和链接结果保持一致。

## 14. 实施顺序

### 阶段一：头文件卫生和最低标准

- 调整 include guard。
- 将 warning push/pop 放入 guard 并严格配对。
- 修正所有 _MSC_VER 未定义判断。
- 引入 UNICONV_CXX_STANDARD。
- 对低于 C++17 的编译直接失败。
- 为 UNICONV_NO_THREAD_LOCAL 提供明确默认值。

### 阶段二：内部检测和公共能力宏

- 加入内部编译器、工具链和平台事实宏。
- 加入 UNICONV_HAS_CHAR8_T。
- 所有布尔能力宏固定为 0/1；UNICONV_CXX_STANDARD 保留标准长整数值。
- 3.x 将历史 CPP_STANDARD 标记为弃用并保留兼容别名，4.0 删除该名称。

### 阶段三：执行字符集策略

- 不新增公共执行字符集动作宏。
- 4.0 不自动执行 pragma，也不提供 opt-in 开关。
- 4.0 移除自动 pragma；只有回移到 3.x 维护分支时才增加临时 legacy 自动模式。
- 在 CMake 中改用 target-scoped UTF-8 选项。
- 在文档中区分源文件字符集、执行字符集和运行时编码。

### 阶段四：字面量文档

- 4.0 不加入 UNICONV_U8_LITERAL；记录未来重新评审它所需的真实用例。
- 不加入 W/U16/U32 字面量包装。
- 不加入 UNICONV_UTF8_VIEW。
- 不执行隐式 char8_t 转换。

### 阶段五：测试和发布文档

- 完成编译器、标准、char8_t 和混合构建矩阵。
- 增加 README 中的公共宏清单和 UTF-8 路线说明。
- 记录 3.x legacy 兼容范围与 4.0 移除边界。
- 确认安装规则和 FetchContent 不需要额外头文件。

## 15. 验收标准

方案实现后必须满足：

1. 新增公共宏列表与本文一致，不增加未记录的检测宏；既有性能、导出和构建宏不在本清单内。
2. 不定义或新增 UNICONV_CP_* 数值代码页常量宏；操作系统 SDK 自带宏不在此限制内。
3. 4.0 包含 UniConv.h 不产生执行字符集的隐藏副作用；3.x 维护分支仅在内部 legacy 兼容默认值为 1 时例外。
4. C++17 以下编译在头文件阶段明确失败。
5. UNICONV_HAS_CHAR8_T 不把 C++20 强行等同于 char8_t 已启用。
6. 4.0 不发布 UNICONV_U8_LITERAL；文档准确说明标准 u8 字面量的跨标准类型差异。
7. 不提供隐式的 char8_t 到 char 的转换。
8. 混合 C++17/C++20 构建不改变公共 ABI 或产生 ODR 风险。
9. GCC/Clang 的 -Wundef -Werror 和 MSVC 的 /W4 /WX 测试通过。
10. 现有运行时转换、locale、缓存、线程和错误处理行为不因宏重构而改变。
