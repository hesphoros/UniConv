#pragma once
#include <string.h>
#include <fstream>
#include <codecvt>
#include <locale>


std::wstring    Utf8ConvertsToUcs4(const  std::string& utf8str);
std::string     Ucs4ConvertToUtf8(const   std::wstring& wstr);
std::wstring    U16StringToWString(const  std::u16string& u16str);
