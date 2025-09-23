# UniConv

A high-performance C++ character encoding conversion library with modern C++17 interface.

[中文文档](README_CN.md) | [English](README.MD)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-green.svg)](https://cmake.org/)

UniConv provides a modern C++17 wrapper around GNU libiconv with performance optimizations and thread-safe operations.

## Features

- **Modern C++17 API**: RAII-based resource management
- **High Performance**: Up to 2.35x faster than traditional iconv
- **Zero Dependencies**: Self-contained with embedded GNU libiconv  
- **Thread Safe**: Native multi-threading support
- **Cross Platform**: Windows, Linux, and macOS support
- **100+ Encodings**: Unicode, Asian, and European character sets

## Performance

```text
ConvertEncodingFast:         434 ns/op  (2.3M ops/s)
ConvertEncodingFastWithHint: 358 ns/op  (2.8M ops/s)
Traditional method:          810 μs/1000 ops  (1.23M ops/s)
Maximum improvement:         2.35x
```

## Supported Encodings

- **Unicode**: UTF-8, UTF-16LE/BE, UTF-32LE/BE
- **Chinese**: GBK, GB2312, GB18030, Big5  
- **Japanese**: Shift-JIS, EUC-JP, ISO-2022-JP
- **European**: ISO-8859-1~15, Windows-1252
- **100+ more**: See documentation for complete list

## Quick Start

### Requirements

- C++17 compatible compiler
- CMake 3.16 or higher
- Windows, Linux, or macOS

### Installation

#### Using vcpkg (Recommended)

```bash
vcpkg install uniconv
```

#### Using CMake FetchContent

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

#### Manual Build

```bash
git clone https://github.com/hesphoros/UniConv.git
cd UniConv
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Usage

### Basic Usage

```cpp
#include "UniConv.h"

int main() {
    auto conv = UniConv::GetInstance();
    
    // Convert GBK to UTF-8
    auto result = conv->ConvertEncodingFast("中文测试", "GBK", "UTF-8");
    
    if (result.IsSuccess()) {
        std::cout << result.GetValue() << std::endl;
    } else {
        std::cerr << "Conversion failed: " << result.GetError() << std::endl;
    }
    
    return 0;
}
```

### Batch Processing

```cpp
std::vector<std::string> texts = {"文本1", "文本2", "文本3"};  
auto results = conv->ConvertEncodingBatch(texts, "GBK", "UTF-8");
```

## API Reference

```cpp
// Core functions
ConvertEncodingFast(input, from_encoding, to_encoding)
ConvertEncodingFastWithHint(input, from_encoding, to_encoding, buffer)  
ConvertEncodingBatch(inputs, from_encoding, to_encoding)

// Error handling
auto result = conv->ConvertEncodingFast(input, from, to);
if (result.IsSuccess()) {
    auto converted = result.GetValue();
}
```

## Building and Testing

### Build Options

```bash
# Standard build
cmake .. -DCMAKE_BUILD_TYPE=Release

# With tests
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# With examples
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON
```

### Running Tests

```bash
cd build
ctest --output-on-failure
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Based on GNU libiconv for core conversion functionality
- Inspired by modern C++ best practices and performance optimization techniques
