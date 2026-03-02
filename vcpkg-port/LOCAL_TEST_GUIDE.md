# UniConv vcpkg Port 本地测试指南

## 测试环境设置

### 1. 首先确保您的项目已经推送到GitHub
由于vcpkg需要从GitHub下载源代码，请确保：
- 代码已推送到 https://github.com/hesphoros/UniConv
- 创建了版本标签（如 v2.0.0）

### 2. 获取SHA512哈希值
运行以下PowerShell命令获取源代码的SHA512哈希：

```powershell
# 下载源代码压缩包
$url = "https://github.com/hesphoros/UniConv/archive/refs/tags/v2.0.0.tar.gz"
$outputPath = "$env:TEMP\uniconv-v2.0.0.tar.gz"
Invoke-WebRequest -Uri $url -OutFile $outputPath

# 计算SHA512哈希
Get-FileHash -Path $outputPath -Algorithm SHA512 | Select-Object Hash
```

### 3. 本地vcpkg测试

#### 3.1 复制port文件到vcpkg
假设您的vcpkg安装在 `C:\vcpkg`：

```powershell
# 复制port文件
Copy-Item -Path "d:\codespace\UniConv\vcpkg-port\ports\uniconv" -Destination "C:\vcpkg\ports\" -Recurse -Force
```

#### 3.2 更新portfile.cmake中的SHA512
将portfile.cmake中的SHA512值替换为上一步获取的真实值。

#### 3.3 测试安装
```powershell
# 进入vcpkg目录
cd C:\vcpkg

# 安装UniConv
.\vcpkg install uniconv

# 安装指定三元组（如需要）
.\vcpkg install uniconv:x64-windows
.\vcpkg install uniconv:x64-windows-static
```

### 4. 创建测试项目

创建一个简单的测试项目来验证安装：

#### 4.1 创建测试目录
```powershell
mkdir "d:\test-uniconv-vcpkg"
cd "d:\test-uniconv-vcpkg"
```

#### 4.2 创建CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)
project(TestUniConv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(UniConv CONFIG REQUIRED)

add_executable(test_uniconv main.cpp)
target_link_libraries(test_uniconv PRIVATE UniConv::UniConv)
```

#### 4.3 创建main.cpp测试代码
```cpp
#include <iostream>
#include <string>
#include <uniconv/UniConv.h>

int main() {
    try {
        auto uniconv = UniConv::GetInstance();
        
        std::string input = "Hello, 世界!";
        auto result = uniconv->ConvertEncodingFast(input, "UTF-8", "UTF-16LE");
        
        if (result.IsSuccess()) {
            std::cout << "UniConv is working correctly!" << std::endl;
            std::cout << "Converted " << input.size() << " bytes to " 
                      << result.GetValue().size() << " bytes" << std::endl;
        } else {
            std::cout << "Conversion failed: " << result.GetErrorMessage() << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

#### 4.4 构建测试项目
```powershell
# 配置项目（使用vcpkg工具链）
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# 构建项目
cmake --build build --config Release

# 运行测试
.\build\Release\test_uniconv.exe
```

## 5. 验证检查清单

- [ ] port文件语法正确
- [ ] SHA512哈希值正确
- [ ] 本地安装成功（无错误）
- [ ] 测试项目能找到UniConv包
- [ ] 测试项目编译成功
- [ ] 测试程序运行正常
- [ ] 头文件安装位置正确
- [ ] CMake配置文件生成正确

## 6. 常见问题排查

### 问题1：SHA512不匹配
**解决方案**：重新计算并更新portfile.cmake中的SHA512值

### 问题2：找不到包
**解决方案**：检查vcpkg integrate install是否已执行

### 问题3：头文件缺失
**解决方案**：检查portfile.cmake中的文件安装路径

### 问题4：链接错误
**解决方案**：确认CMake target导出正确

## 7. 成功标准

如果所有测试通过，您的port就可以提交到Microsoft/vcpkg仓库了！

## 下一步：提交到vcpkg官方仓库

测试成功后，我们将：
1. Fork Microsoft/vcpkg仓库
2. 创建PR分支
3. 提交port文件
4. 编写详细的PR描述