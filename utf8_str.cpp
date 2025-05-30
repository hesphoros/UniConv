/**
 * @file utf8_str.cpp
 * @brief UTF-8 encoded test strings for encoding validation
 * @details This file contains UTF-8 encoded test strings used for validating
 *          UTF-8 encoding detection and conversion functionality. The file
 *          must be saved with UTF-8 encoding to maintain proper character representation.
 * 
 * @warning This file must be saved with UTF-8 encoding (without BOM)
 * @since 1.0.0.1
 */

#include <string>

/// UTF-8 encoded C-style string for testing
const char* utf8_cstr = "我是一个Utf8字符串请确保我所在的文件以UTF-8编码保存";

/// UTF-8 encoded std::string for testing  
std::string utf8_str = "我是一个Utf8字符串请确保我所在的文件以UTf-8编码保存";