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

//TODO: �����¼ʹ��unique_ptr get() ��ָ�빹����� 
// ����ʹ��shared_ptr ���������Ϣ

class UniConv : public Singleton<UniConv> 
{
	friend class Singleton<UniConv>; // ���� Singleton ���� Convert ��˽�й��캯��
private:
	// �Զ���ɾ�����������ͷ� iconv_t ��Դ
	struct IconvDeleter {
		void operator()(iconv_t cd) const {
			std::cerr << "Closing iconv_t: " << cd << std::endl;
			// ֻ���� cd ������Ч���ʱ�ŵ��� iconv_close
			if (cd != reinterpret_cast<iconv_t>(-1)) {
				iconv_close(cd);
			}
		}
	};

	using IconvUniquePtr = std::unique_ptr < std::remove_pointer<iconv_t>::type, UniConv::IconvDeleter>;
	using IconvSharedPtr = std::shared_ptr <std::remove_pointer<iconv_t>::type>;
	
	/// <summary>
	/// ������Ϣ
	/// </summary>
	struct EncodingInfo
	{
		std::string dotNetName;//��������
		std::string extra_info;
	};

public:
	

	/// <summary>
	/// ת������ṹ��
	/// </summary>
	struct IConvResult {
		std::string        conv_result_str;// ת���ɹ��Ľ�� ʹ���±�����ַ���
		int                error_code = 0; // ������
		std::string_view   error_msg = {}; // ������Ϣ

		// �ж��Ƿ�ת���ɹ�
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
	/// ��ȡ��ǰϵͳ����
	/// </summary>
	/// <returns> ��ǰϵͳ�����.Net name </returns>
	static std::string GetCurrentSystemEncoding();

	/// <summary>
	/// ��ȡ��ǰϵͳ�������ҳ
	/// </summary>
	/// <returns> ��δ�ҵ�Ĭ��Ϊ65001 ��֧���򷵻�0 </returns>
	static std::uint16_t GetCurrentSystemEncodingCodePage();

	/// <summary>
	/// ��ȡָ������ҳ�ı�������
	/// </summary>
	/// <param name="codePage"></param>
	/// <returns></returns>
	static std::string   GetEncodingNameByCodePage(std::uint16_t codePage);

	/// <summary>
	/// ���ر���ת��ΪUTF-8
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	static std::string LocateConvertToUtf8(const std::string& sInput);

	/// <summary>
	/// ���ر���ת��ΪUTF-8
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	static std::string LocateConvertToUtf8(const char* sInput) {};

	/// <summary>
	/// ��װ��iconv ת������
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
	/// ��ȡiconv ת��������
	/// </summary>
	/// <param name="fromcode"> ԭ����c </param>
	/// <param name="tocode"> Ŀ����� </param>
	/// <returns>
	///  iconv_t ������ ��������ʱҲ�᷵��һ�������iconv_t ������
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

