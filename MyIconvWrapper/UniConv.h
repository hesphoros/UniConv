#ifndef __UNICONV_H__
#define __UNICONV_H__

#include "iconv.h"
#include <iostream>
#include <string>
#include <malloc.h>
#include <unordered_map>
#include <map>
#include <cwchar>
#include <clocale>
#include <sstream>
#include <vector>
#include <errno.h>
#include <string_view>
#include <cerrno>
#include <cstring>
#include <system_error>
#include <mutex>
#include <memory>
#include <functional>
#include <io.h>
#include <fcntl.h>
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



//TODO: �����¼ʹ��unique_ptr get() ��ָ�빹����� 
// ����ʹ��shared_ptr ���������Ϣ


class __declspec(dllexport) UniConv : public Singleton<UniConv>
{

	friend class Singleton<UniConv>; // ���� Singleton ���� Convert ��˽�й��캯��
public:
	/// <summary>
	/// ���ﶨ��һЩ���õı���
	/// </summary>
	/*{@ */
		
	// European languages ŷ������
	static constexpr const char* ascii_encoding = "ASCII";
	static constexpr const char* iso_8859_1_encoding = "ISO-8859-1";
	static constexpr const char* iso_8859_2_encoding = "ISO-8859-2";
	static constexpr const char* iso_8859_3_encoding = "ISO-8859-3";
	static constexpr const char* iso_8859_4_encoding = "ISO-8859-4";
	static constexpr const char* iso_8859_5_encoding = "ISO-8859-5";
	static constexpr const char* iso_8859_7_encoding = "ISO-8859-7";
	static constexpr const char* iso_8859_9_encoding = "ISO-8859-9";
	static constexpr const char* iso_8859_10_encoding = "ISO-8859-10";
	static constexpr const char* iso_8859_13_encoding = "ISO-8859-13";
	static constexpr const char* iso_8859_14_encoding = "ISO-8859-14";
	static constexpr const char* iso_8859_15_encoding = "ISO-8859-15";
	static constexpr const char* iso_8859_16_encoding = "ISO-8859-16";
	static constexpr const char* koi8_r_encoding = "KOI8-R";
	static constexpr const char* koi8_u_encoding = "KOI8-U";
	static constexpr const char* koi8_ru_encoding = "KOI8-RU";
	static constexpr const char* cp1250_encoding = "CP1250";
	static constexpr const char* cp1251_encoding = "CP1251";
	static constexpr const char* cp1252_encoding = "CP1252";
	static constexpr const char* cp1253_encoding = "CP1253";
	static constexpr const char* cp1254_encoding = "CP1254";
	static constexpr const char* cp1257_encoding = "CP1257";
	static constexpr const char* cp850_encoding = "CP850";
	static constexpr const char* cp866_encoding = "CP866";
	static constexpr const char* cp1131_encoding = "CP1131";
	static constexpr const char* mac_roman_encoding = "MacRoman";
	static constexpr const char* mac_central_europe_encoding = "MacCentralEurope";
	static constexpr const char* mac_iceland_encoding = "MacIceland";
	static constexpr const char* mac_croatian_encoding = "MacCroatian";
	static constexpr const char* mac_romania_encoding = "MacRomania";
	static constexpr const char* mac_cyrillic_encoding = "MacCyrillic";
	static constexpr const char* mac_ukraine_encoding = "MacUkraine";
	static constexpr const char* mac_greek_encoding = "MacGreek";
	static constexpr const char* mac_turkish_encoding = "MacTurkish";
	static constexpr const char* macintosh_encoding = "Macintosh";

	// Semitic languages
	static constexpr const char* iso_8859_6_encoding = "ISO-8859-6";
	static constexpr const char* iso_8859_8_encoding = "ISO-8859-8";
	static constexpr const char* cp1255_encoding = "CP1255";
	static constexpr const char* cp1256_encoding = "CP1256";
	static constexpr const char* cp862_encoding = "CP862";
	static constexpr const char* mac_hebrew_encoding = "MacHebrew";
	static constexpr const char* mac_arabic_encoding = "MacArabic";

	// Japanese
	static constexpr const char* euc_jp_encoding = "EUC-JP";
	static constexpr const char* shift_jis_encoding = "SHIFT_JIS";
	static constexpr const char* cp932_encoding = "CP932";
	static constexpr const char* iso_2022_jp_encoding = "ISO-2022-JP";
	static constexpr const char* iso_2022_jp_2_encoding = "ISO-2022-JP-2";
	static constexpr const char* iso_2022_jp_1_encoding = "ISO-2022-JP-1";
	static constexpr const char* iso_2022_jp_ms_encoding = "ISO-2022-JP-MS";

	// Chinese
	static constexpr const char* euc_cn_encoding = "EUC-CN";
	static constexpr const char* hz_encoding = "HZ";
	static constexpr const char* gbk_encoding = "GBK";
	static constexpr const char* cp936_encoding = "CP936";
	static constexpr const char* gb18030_encoding = "GB18030";
	static constexpr const char* gb18030_2022_encoding = "GB18030:2022";
	static constexpr const char* euc_tw_encoding = "EUC-TW";
	static constexpr const char* big5_encoding = "BIG5";
	static constexpr const char* cp950_encoding = "CP950";
	static constexpr const char* big5_hkscs_encoding = "BIG5-HKSCS";
	static constexpr const char* big5_hkscs_2004_encoding = "BIG5-HKSCS:2004";
	static constexpr const char* big5_hkscs_2001_encoding = "BIG5-HKSCS:2001";
	static constexpr const char* big5_hkscs_1999_encoding = "BIG5-HKSCS:1999";
	static constexpr const char* iso_2022_cn_encoding = "ISO-2022-CN";
	static constexpr const char* iso_2022_cn_ext_encoding = "ISO-2022-CN-EXT";

	// Korean
	static constexpr const char* euc_kr_encoding = "EUC-KR";
	static constexpr const char* cp949_encoding = "CP949";
	static constexpr const char* iso_2022_kr_encoding = "ISO-2022-KR";
	static constexpr const char* johab_encoding = "JOHAB";

	// Armenian
	static constexpr const char* armscii_8_encoding = "ARMSCII-8";

	// Georgian
	static constexpr const char* georgian_academy_encoding = "Georgian-Academy";
	static constexpr const char* georgian_ps_encoding = "Georgian-PS";

	// Tajik
	static constexpr const char* koi8_t_encoding = "KOI8-T";

	// Kazakh
	static constexpr const char* pt154_encoding = "PT154";
	static constexpr const char* rk1048_encoding = "RK1048";

	// Thai
	static constexpr const char* tis_620_encoding = "TIS-620";
	static constexpr const char* cp874_encoding = "CP874";
	static constexpr const char* mac_thai_encoding = "MacThai";

	// Laotian
	static constexpr const char* mulelao_1_encoding = "MuleLao-1";
	static constexpr const char* cp1133_encoding = "CP1133";

	// Vietnamese
	static constexpr const char* viscii_encoding = "VISCII";
	static constexpr const char* tcvn_encoding = "TCVN";
	static constexpr const char* cp1258_encoding = "CP1258";

	// Platform specifics
	static constexpr const char* hp_roman8_encoding = "HP-ROMAN8";
	static constexpr const char* nextstep_encoding = "NEXTSTEP";

	// Full Unicode
	static constexpr const char* utf_8_encoding = "UTF-8";
	static constexpr const char* ucs_2_encoding = "UCS-2";
	static constexpr const char* ucs_2be_encoding = "UCS-2BE";
	static constexpr const char* ucs_2le_encoding = "UCS-2LE";
	static constexpr const char* ucs_4_encoding = "UCS-4";
	static constexpr const char* ucs_4be_encoding = "UCS-4BE";
	static constexpr const char* ucs_4le_encoding = "UCS-4LE";
	static constexpr const char* utf_16_encoding  = "UTF-16";
	static constexpr const char* utf_16be_encoding = "UTF-16BE";
	static constexpr const char* utf_16le_encoding = "UTF-16LE";
	static constexpr const char* utf_32_encoding = "UTF-32";
	static constexpr const char* utf_32be_encoding = "UTF-32BE";
	static constexpr const char* utf_32le_encoding = "UTF-32LE";
	static constexpr const char* utf_7_encoding = "UTF-7";

	// Locale dependent
	static constexpr const char* char_encoding = "char";
	static constexpr const char* wchar_t_encoding = "wchar_t";

	// EBCDIC compatible (not ASCII compatible, very rarely used)
	static constexpr const char* ibm_037_encoding = "IBM-037";
	static constexpr const char* ibm_273_encoding = "IBM-273";
	static constexpr const char* ibm_277_encoding = "IBM-277";
	static constexpr const char* ibm_278_encoding = "IBM-278";
	static constexpr const char* ibm_280_encoding = "IBM-280";
	static constexpr const char* ibm_282_encoding = "IBM-282";
	static constexpr const char* ibm_284_encoding = "IBM-284";
	static constexpr const char* ibm_285_encoding = "IBM-285";
	static constexpr const char* ibm_297_encoding = "IBM-297";
	static constexpr const char* ibm_423_encoding = "IBM-423";
	static constexpr const char* ibm_500_encoding = "IBM-500";
	static constexpr const char* ibm_870_encoding = "IBM-870";
	static constexpr const char* ibm_871_encoding = "IBM-871";
	static constexpr const char* ibm_875_encoding = "IBM-875";
	static constexpr const char* ibm_880_encoding = "IBM-880";
	static constexpr const char* ibm_905_encoding = "IBM-905";
	static constexpr const char* ibm_924_encoding = "IBM-924";
	static constexpr const char* ibm_1025_encoding = "IBM-1025";
	static constexpr const char* ibm_1026_encoding = "IBM-1026";
	static constexpr const char* ibm_1047_encoding = "IBM-1047";
	static constexpr const char* ibm_1112_encoding = "IBM-1112";
	static constexpr const char* ibm_1122_encoding = "IBM-1122";
	static constexpr const char* ibm_1123_encoding = "IBM-1123";
	static constexpr const char* ibm_1140_encoding = "IBM-1140";
	static constexpr const char* ibm_1141_encoding = "IBM-1141";
	static constexpr const char* ibm_1142_encoding = "IBM-1142";
	static constexpr const char* ibm_1143_encoding = "IBM-1143";
	static constexpr const char* ibm_1144_encoding = "IBM-1144";
	static constexpr const char* ibm_1145_encoding = "IBM-1145";
	static constexpr const char* ibm_1146_encoding = "IBM-1146";
	static constexpr const char* ibm_1147_encoding = "IBM-1147";
	static constexpr const char* ibm_1148_encoding = "IBM-1148";
	static constexpr const char* ibm_1149_encoding = "IBM-1149";
	static constexpr const char* ibm_1153_encoding = "IBM-1153";
	static constexpr const char* ibm_1154_encoding = "IBM-1154";
	static constexpr const char* ibm_1155_encoding = "IBM-1155";
	static constexpr const char* ibm_1156_encoding = "IBM-1156";
	static constexpr const char* ibm_1157_encoding = "IBM-1157";
	static constexpr const char* ibm_1158_encoding = "IBM-1158";
	static constexpr const char* ibm_1165_encoding = "IBM-1165";
	static constexpr const char* ibm_1166_encoding = "IBM-1166";
	static constexpr const char* ibm_4971_encoding = "IBM-4971";
	static constexpr const char* ibm_424_encoding = "IBM-424";
	static constexpr const char* ibm_425_encoding = "IBM-425";
	static constexpr const char* ibm_12712_encoding = "IBM-12712";
	static constexpr const char* ibm_16804_encoding = "IBM-16804";
	static constexpr const char* ibm_1097_encoding = "IBM-1097";
	static constexpr const char* ibm_838_encoding = "IBM-838";
	static constexpr const char* ibm_1160_encoding = "IBM-1160";
	static constexpr const char* ibm_1132_encoding = "IBM-1132";
	static constexpr const char* ibm_1130_encoding = "IBM-1130";
	static constexpr const char* ibm_1164_encoding = "IBM-1164";
	static constexpr const char* ibm_1137_encoding = "IBM-1137";
	/*@}*/
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

	/*using IconvUniquePtr = std::unique_ptr <std::remove_pointer<iconv_t>::type, UniConv::IconvDeleter>;*/
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
		std::string        error_msg = {}; // ������Ϣ

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
		
	}
	

	/// <summary>
	/// ��ȡ��ǰϵͳ����
	/// </summary>
	/// <returns> ��ǰϵͳ�����.Net name </returns>
	std::string          GetCurrentSystemEncoding();

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
	std::string          LocaleConvertToUtf8(const std::string& sInput);
	std::string          LocaleConvertToUtf8(const char* sInput);

	/// <summary>
	/// UTF-8 ת��Ϊ���ر���
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	std::string          Utf8ConvertToLocale(const std::string& sInput);
    std::string          Utf8ConvertToLocale(const char* sInput);

	/// <summary>
	/// ���ر���ת��ΪUTF-16LE С��
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	std::u16string       LocaleConvertToUtf16LE(const std::string& sInput);
    std::u16string       LocaleConvertToUtf16LE(const char* sInput);

	/// <summary>
	/// ���ر���ת��ΪUTF-16BE ���
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	std::u16string       LocaleConvertToUtf16BE(const std::string& sInput);
	std::u16string       LocaleConvertToUtf16BE(const char* sInput);

	/// <summary>
	/// UTF-16LE С�� ת��Ϊ���ر��� Ŀǰ��������
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	std::string          Utf16BEConvertToLocale(const std::u16string& sInput);
	std::string          Utf16BEConvertToLocale(const char16_t* sInput);

	/// <summary>
	/// UTF-16LE С�� ת��ΪUTF8����
	/// </summary>
	/// <param name="sInput"></param>
	/// <returns></returns>
	std::string          Utf16LEConvertToUtf8(const std::u16string& sInput);
	std::string          Utf16LEConvertToUtf8(const char16_t* sInput);


	std::string          Utf16BEConvertToUtf8(const std::u16string& sInput);
	std::string          Utf16BEConvertToUtf8(const char16_t* sInput);


	std::u16string       Utf8ConvertToUtf16LE(const std::string& sInput);
	std::u16string       Utf8ConvertToUtf16LE(const char* sInput);

	std::u16string       Utf8ConvertToUtf16BE(const std::string& sInput);
	std::u16string       Utf8ConvertToUtf16BE(const char* sInput);

	std::u16string       Utf16LEConvertToUtf16BE(const std::u16string& sInput);
	std::u16string       Utf16LEConvertToUtf16BE(const char16_t* sInput);

	std::u16string       Utf16BEConvertToUtf16LE(const std::u16string& sInput);
	std::u16string       Utf16BEConvertToUtf16LE(const char16_t* sInput);

	std::wstring         LocaleToWideString(const std::string& sInput);
	std::wstring         LocaleToWideString(const char* sInput);

	//// ���ַ�ת Locale
	std::string          WideConvertToLocale(const std::wstring& sInput);
	std::string          WideConvertToLocale(const wchar_t* sInput);

	std::wstring         Utf8ConvertToWide(const std::string& sInput);
	std::wstring         Utf8ConvertToWide(const char* sInput);

	std::string          WideConvertToUtf8(const std::wstring& sInput);
	std::string          WideConvertToUtf8(const wchar_t* sInput);

	std::string          Utf32ConvertToUtf8(const std::u32string& sInput);
	std::u16string       Utf32ConvertToUtf16LE(const std::u32string& sInput);
	std::u16string       Utf32ConvertToUtf16BE(const std::u32string& sInput);

	std::u32string       Utf8ConvertToUtf32(const std::string& sInput);
	std::u32string       Utf16LEConvertToUtf32(const std::u16string& sInput);
	std::u32string       Utf16BEConvertToUtf32(const std::u16string& sInput);
	
	
private:
	static const std::unordered_map<std::uint16_t,EncodingInfo>             m_encodingMap;
	static const std::unordered_map<std::string,std::uint16_t>              m_encodingToCodePageMap;
	//std::unordered_map<std::string,UniConv::IconvUniquePtr>               m_iconvDesscriptorCacheMap;
	std::mutex                                                              m_iconvcCacheMutex;
	static const std::unordered_map<int,std::string_view>                   m_iconvErrorMap;
	static std::unordered_map<std::string, IconvSharedPtr>                  m_iconvDesscriptorCacheMapS;
	
private:
	/// <summary>
	/// ��װ��iconv ת������
	/// </summary>
	/// <param name="in"></param>
	/// <param name="fromcode"></param>
	/// <param name="tocode"></param>
	/// <returns></returns>
	IConvResult                         Convert(std::string_view in, const char* fromcode, const char* tocode);

	static std::string_view             GetIconvErrorString(int err_code);

	/// <summary>
	/// ��ȡiconv ת��������
	/// </summary>
	/// <param name="fromcode"> ԭ����c </param>
	/// <param name="tocode"> Ŀ����� </param>
	/// <returns>
	///  iconv_t ������ ��������ʱҲ�᷵��һ�������iconv_t ������
	/// </returns>
	//IconvUniquePtr                    GetIconvDescriptor(const char* fromcode, const char* tocode);
	IconvSharedPtr                      GetIconvDescriptorS(const char* fromcode, const char* tocode);

	

private:
	UniConv() {					
	}
	UniConv(const UniConv&) = delete;
	UniConv& operator=(const UniConv&) = delete;
};

#endif // __UNICONV_H__

