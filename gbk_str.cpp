/**
 * @file gbk_str.cpp
 * @brief GBK encoded test strings for encoding validation
 * @details This file contains GBK encoded test strings used for validating
 *          GBK encoding detection and conversion functionality. The file
 *          must be saved with GBK encoding to maintain proper character representation.
 * 
 * @warning This file must be saved with GBK encoding
 * @note The content may appear garbled in UTF-8 editors
 * @since 1.0.0.1
 */

#include <string>

/// GBK encoded C-style string for testing (content: "This is a GBK string, please ensure the cpp file is saved as GBK!")
char const* gbk_cstr = "����һ��GBK�ַ�������ַ���,��ȷ�������ڵ�cpp�ļ�����ΪGBK!";

/// GBK encoded std::string for testing (content: "This is a GBK string, please ensure the cpp file is saved as GBK!")
std::string gbk_str = "����һ��GBK�ַ�������ַ���,��ȷ�������ڵ�cpp�ļ�����ΪGBK!";
