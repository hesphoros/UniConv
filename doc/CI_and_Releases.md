# 跨平台构建与 GitHub Release

## 构建矩阵

工作流统一使用 CMake、Ninja、Release 配置和 C++17 构建库，现有测试另外覆盖 C++20、`char8_t` 开关及混合标准链接。

| 平台 | GitHub Runner | 架构 | 工具链 / 运行环境 |
| --- | --- | --- | --- |
| Linux | `ubuntu-22.04` | x64 | GCC，glibc 2.35 或更新的兼容环境 |
| Windows | `windows-2022` | x64 | VS 2022 MSVC，动态 CRT（`/MD`） |
| macOS | `macos-15-intel` | x64 | Apple Clang，最低 macOS 15.0 |
| macOS | `macos-15` | arm64 | Apple Clang，最低 macOS 15.0 |

每个平台构建 `static` / `shared` × `scalar` / `simd` 四个组合，共 16 个任务。SIMD 使用 simdutf 的运行时指令集选择；不使用 `-march=native`。macOS 分别提供两种架构的下载包，不是 Universal Binary。

macOS 构建时检测浮点 `std::from_chars` 的可用性；工具链缺失实现或目标系统版本不支持时，内部使用带独立 C locale 的 `strtof_l` / `strtod_l` 兼容路径，不修改进程 locale。保留严格输入语法、溢出/下溢检查及现有公开 API。

每个任务依次执行：

1. 从指定提交检出代码，获取固定版本依赖。
2. 编译库、示例和完整测试套件。
3. 运行 CTest；测试失败阻止打包。
4. 安装至临时目录，迁移安装前缀（含空格的路径），使用独立 CMake 项目执行 `find_package(UniConv CONFIG REQUIRED)`。
5. 分别以 C++17、C++20 编译、链接并运行消费端，验证 UTF 往返、GBK 转换和数值解析。
6. 打包库、公共依赖头文件、CMake 导出配置、许可证、构建元数据，计算 SHA-256。

静态包启用 `UNICONV_BUNDLE_DEPS=ON`，`UniConv` 静态库中合并 iconv、charset 及可选 simdutf。动态包将静态依赖链接至 UniConv；安装包仍携带公共头文件和 CMake 使用要求需要的依赖。Windows 动态包含 DLL 与导入库。

## 工作流与触发方式

- `.github/workflows/build.yml`：复用构建矩阵；CI 与 Release 使用相同的构建、测试和打包步骤。
- `.github/workflows/ci.yml`：推送 `main`、`codex/**` 分支，任意 PR，以及手动运行时触发；构建包在 Actions Artifacts 保留 14 天。
- `.github/workflows/release.yml`：推送 `v*` 标签触发，或手动指定**已存在**的版本标签。手动发布始终构建该标签解析出的提交，而不是所选工作流分支的最新源码。

工作流默认仅有 `contents: read`，只有最后发布任务获得 `contents: write`。无需配置个人访问令牌或额外仓库 Secret，发布使用 GitHub 自动提供的 `GITHUB_TOKEN`。Actions 必须在仓库中启用，组织策略需允许官方 Actions 与固定提交的 MSVC 环境初始化 Action。

## 发布新版本

先将改动合并到主分支并确认 CI 通过，再显式创建版本标签：

```sh
git switch main
git pull --ff-only
# 确认 CMakeLists.txt 中 project(UniConv VERSION ...) 与标签基础版本一致。
git tag -a v4.0.0 -m "UniConv 4.0.0"
git push origin v4.0.0
```

正式版标签格式为 `vMAJOR.MINOR.PATCH`；预发布如 `v4.0.0-rc.1`，会自动标记为 GitHub prerelease。标签基础版本必须与 CMake 项目版本一致；不接受浮动分支名，也不自动创建、移动标签。

所有构建任务成功后，发布任务检查 **16 个二进制包和 1 个完整源码包**及每个文件的 SHA-256，再创建 Release 草稿、上传附件并公开。任一构建或校验失败，都不会公开本次 Release。上传失败会保留草稿，修复后可重新运行；已经公开的 Release 不会被自动覆盖。

若重新执行发布任务，需要对应运行的 Actions Artifacts 尚未过期；否则重新运行整个 Release 工作流。工作流按版本串行执行发布。

## 附件命名和选择

```text
UniConv-4.0.0-linux-x64-static-scalar.tar.gz
UniConv-4.0.0-linux-x64-static-simd.tar.gz
UniConv-4.0.0-windows-x64-shared-simd.zip
UniConv-4.0.0-macos-x64-static-simd.tar.gz
UniConv-4.0.0-macos-arm64-shared-scalar.tar.gz
UniConv-4.0.0-sources.tar.gz
SHA256SUMS
```

选择与目标系统、CPU 架构及静态/动态链接方式匹配的包。Linux/macOS 使用 `.tar.gz`，Windows 使用 `.zip`。发布包均为 Release；Windows 消费方使用兼容 MSVC 工具链及 `/MD`，需要安装相应 Visual C++ Runtime。Debug、MinGW、Linux ARM64 和代码签名不在当前发布矩阵中。

`build-info.json` 记录 UniConv 提交、实际依赖提交、工具链名称和构建开关。完整源码包额外包含构建使用的 libiconv-native、parallel-hashmap、simdutf 和 GoogleTest 源码及其许可证，`BUILD_FROM_SOURCE.md` 提供离线重建方法。GitHub 自动生成的 Source code 下载仅含本仓库；离线重建请使用带 `-sources` 的附件。

校验附件，例如在 Linux：

```sh
sha256sum --check SHA256SUMS
```

在 macOS 使用 `shasum -a 256 --check SHA256SUMS`；只下载部分附件时，选择 `SHA256SUMS` 中对应行进行校验。在 Windows 可用 `Get-FileHash <文件> -Algorithm SHA256` 与清单比对。

## 使用预编译包

解压后，将顶层目录传入 `CMAKE_PREFIX_PATH`：

```cmake
cmake_minimum_required(VERSION 3.16)
project(example LANGUAGES CXX)
find_package(UniConv CONFIG REQUIRED)
add_executable(example main.cpp)
target_link_libraries(example PRIVATE UniConv::UniConv)
```

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/absolute/path/to/UniConv-4.0.0-linux-x64-static-simd
cmake --build build --config Release
```

头文件使用 `#include <UniConv/UniConv.h>`。无需单独安装 parallel-hashmap；编译选项和依赖由导出目标传播。若消费方源码包含中文窄字符串字面量，MSVC 项目自行设置 `/utf-8`。

动态包的应用部署需同时携带 UniConv 动态库。Windows 将 `bin/Release/UniConv.dll` 放到程序旁，或将该目录加入 `PATH`。Linux/macOS 将 `lib/Release` 纳入应用的运行时搜索路径；CMake 构建树消费端通常自动具有适当 RPATH，安装应用时仍应配置部署路径。

## 本地复现打包

在项目根目录执行，与 CI 使用同一脚本：

```sh
cmake -S . -B build-ci -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_INSTALL_LIBDIR=lib \
  -DUNICONV_BUILD_TESTS=ON -DUNICONV_BUNDLE_DEPS=ON -DUNICONV_USE_SIMDUTF=ON
cmake --build build-ci --parallel 3
ctest --test-dir build-ci --output-on-failure --no-tests=error
python3 scripts/ci/package.py package --build-dir build-ci --output-dir dist --platform macos-arm64
python3 scripts/ci/package.py sources --build-dir build-ci --output-dir dist
```

根据主机修改 `--platform`。Windows 先初始化 MSVC 开发环境，使用 `python`。打包脚本会核对平台、架构、版本和构建配置，不支持交叉编译打包。使用 FetchContent 依赖来源，以便收集精确依赖版本及许可证。
