#ifndef __UNICONV_H__
#define __UNICONV_H__

#include <iconv.h>
#include <iostream>
#include <string>
#include <malloc.h>
#include <unordered_map>
#include <cwchar>
#include <clocale>
#include <sstream>
#include <string>
#include <vector>
#include <errno.h>
#include <string_view>
#include <cerrno>
#include <cstring>
#include <system_error>
#include <mutex>
#include <memory>
#include <functional>

#include "Singleton.h"

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#ifdef __linux__
#include <unistd.h>
#include <langinfo.h>
#endif // __linux__


#ifdef UNICONV_DLL 
#define UNICONV_EXPORT __declspec(dllexport)
#else
#define UNICONV_EXPORT
#endif

#define UTF_8 (const char *)"UTF-8"
#define GBK   (const char *)"GBK"

//TODO: 错误记录使用unique_ptr get() 罗指针构造错误 
// 最终使用shared_ptr 保存错误信息

class UniConv : public Singleton<UniConv> 
{
	friend class Singleton<UniConv>; // 允许 Singleton 访问 Convert 的私有构造函数
private:
	// 自定义删除器，用于释放 iconv_t 资源
	struct IconvDeleter {
		void operator()(iconv_t cd) const {
			std::cerr << "Closing iconv_t: " << cd << std::endl;
			// 只有在 cd 不是无效句柄时才调用 iconv_close
			if (cd != reinterpret_cast<iconv_t>(-1)) {
				iconv_close(cd);
			}
		}
	};

	using IconvUniquePtr = std::unique_ptr < std::remove_pointer<iconv_t>::type, UniConv::IconvDeleter>;
	using IconvSharedPtr = std::shared_ptr <std::remove_pointer<iconv_t>::type>;
	
	/// <summary>
	/// 编码信息
	/// </summary>
	struct EncodingInfo
	{
		std::string dotNetName;//编码名称
		std::string extra_info;
	};

public:
	

	/// <summary>
	/// 转换结果结构体
	/// </summary>
	struct IConvResult {
		std::string        conv_result_str;// 转换成功的结果 使用新编码的字符串
		int                error_code = 0; // 错误码
		std::string_view   error_msg = {}; // 错误信息

		// 判断是否转换成功
		bool IsSuccess() const {
			return error_code == 0;
		}

		explicit operator bool() const {
			return IsSuccess();
		}
		bool operator!() const {
			return !IsSuccess();
		}
		bool operator==(int code) const {
			return error_code == code;
		}

		bool operator!=(int code) const {
			return error_code != code;
		}


		const char* c_str() const {
			return IsSuccess() ? conv_result_str.c_str() : error_msg.data();
		}
	};


	~UniConv()
	{
		CleanupIconvDescriptorCache();
	}
	

	/// <summary>
	/// 获取当前系统编码
	/// </summary>
	/// <returns> 当前系统编码的.Net name </returns>
	static std::string GetCurrentSystemEncoding();

	/// <summary>
	/// 获取当前系统编码代码页
	/// </summary>
	/// <returns> 若未找到默认为65001 不支持则返回0 </returns>
	static std::uint16_t GetCurrentSystemEncodingCodePage();

	/// <summary>
	/// 获取指定代码页的编码名称
	/// </summary>
	/// <param name="codePage"></param>
	/// <returns></returns>
	static std::string   GetEncodingNameByCodePage(std::uint16_t codePage);

	/// <summary>
	/// 本地编码转换为UTF-8
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	static std::string LocateConvertToUtf8(const std::string& sInput);

	/// <summary>
	/// 本地编码转换为UTF-8
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	static std::string LocateConvertToUtf8(const char* sInput) {};

	/// <summary>
	/// 包装的iconv 转换函数
	/// </summary>
	/// <param name="in"></param>
	/// <param name="fromcode"></param>
	/// <param name="tocode"></param>
	/// <returns></returns>
	static IConvResult            Convert(std::string_view in, const char* fromcode, const char* tocode);
private:
	static const std::unordered_map<std::uint16_t,EncodingInfo>             m_encodingMap;
	static const std::unordered_map<std::string,std::uint16_t>              m_encodingToCodePageMap;
	static std::unordered_map<std::string,UniConv::IconvUniquePtr>          m_iconvDesscriptorCacheMap;
	static std::mutex                                                       m_iconvcCacheMutex;
	static const std::unordered_map<int,std::string_view>                   m_iconvErrorMap;
	static std::unordered_map<std::string, IconvSharedPtr>                  m_iconvDesscriptorCacheMap_s;
private:

	static std::string_view        GetIconvErrorString(int err_code);

	/// <summary>
	/// 获取iconv 转换描述符
	/// </summary>
	/// <param name="fromcode"> 原编码c </param>
	/// <param name="tocode"> 目标编码 </param>
	/// <returns>
	///  iconv_t 描述符 当错误发生时也会返回一个错误的iconv_t 描述符
	/// </returns>
	static IconvUniquePtr          GetIconvDescriptor(const char* fromcode, const char* tocode);
	static IconvSharedPtr          GetIconvDescriptorS(const char* fromcode, const char* tocode);
	static void                    CleanupIconvDescriptorCache();

private:
	UniConv() = default;
	UniConv(const UniConv&) = delete;
	UniConv& operator=(const UniConv&) = delete;
};


#endif // __UNICONV_H__

