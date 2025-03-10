#include "UniConv.h"


const std::unordered_map<int, std::string_view> UniConv::m_iconvErrorMap = {
	{EILSEQ, "Invalid multibyte sequence"},
	{EINVAL, "Incomplete multibyte sequence"},
	{E2BIG,  "Output buffer too small"},
	{EBADF,  "Invalid conversion descriptor"},
	{EFAULT, "Invalid buffer address"},
	{EINTR,  "Conversion interrupted by signal"},
	{ENOMEM, "Out of memory"}
};

const std::unordered_map<std::uint16_t, UniConv::EncodingInfo> UniConv::m_encodingMap = {
	{37, {"IBM037", "IBM EBCDIC US-Canada"}},
	{437, {"IBM437", "OEM United States"}},
	{850, {"IBM850", "OEM Multilingual Latin 1; Western European (DOS)"}},
	{852, {"IBM852", "OEM Latin 2; Central European (DOS)"}},
	{855, {"IBM855", "OEM Cyrillic (primarily Russian)"}},
	{857, {"IBM857", "OEM Turkish; Turkish (DOS)"}},
	{860, {"IBM860", "OEM Portuguese; Portuguese (DOS)"}},
	{861, {"IBM861", "OEM Icelandic; Icelandic (DOS)"}},
	{862, {"DOS-862", "OEM Hebrew; Hebrew (DOS)"}},
	{863, {"IBM863", "OEM French Canadian; French Canadian (DOS)"}},
	{865, {"IBM865", "OEM Nordic; Nordic (DOS)"}},
	{866, {"CP866", "OEM Russian; Cyrillic (DOS)"}},
	{874, {"Windows-874", "Thai (Windows)"}},
	{932, {"Shift_JIS", "ANSI/OEM Japanese; Japanese (Shift-JIS)"}},
	{936, {"GB2312", "ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312)"}},
	{949, {"KS_C_5601-1987", "ANSI/OEM Korean (Unified Hangul Code)"}},
	{950, {"Big5", "ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5)"}},
	{1200, {"UTF-16", "Unicode UTF-16, little endian byte order (BMP of ISO 10646); available only to managed applications"}},
	{1201, {"UTF-16BE", "Unicode UTF-16, big endian byte order; available only to managed applications"}},
	{1250, {"Windows-1250", "ANSI Central European; Central European (Windows)"}},
	{1251, {"Windows-1251", "ANSI Cyrillic; Cyrillic (Windows)"}},
	{1252, {"Windows-1252", "ANSI Latin 1; Western European (Windows)"}},
	{1253, {"Windows-1253", "ANSI Greek; Greek (Windows)"}},
	{1254, {"Windows-1254", "ANSI Turkish; Turkish (Windows)"}},
	{1255, {"Windows-1255", "ANSI Hebrew; Hebrew (Windows)"}},
	{1256, {"Windows-1256", "ANSI Arabic; Arabic (Windows)"}},
	{1257, {"Windows-1257", "ANSI Baltic; Baltic (Windows)"}},
	{1258, {"Windows-1258", "ANSI/OEM Vietnamese; Vietnamese (Windows)"}},
	{20866, {"KOI8-R", "Russian (KOI8-R); Cyrillic (KOI8-R)"}},
	{21866, {"KOI8-U", "Ukrainian (KOI8-U); Cyrillic (KOI8-U)"}},
	{28591, {"ISO-8859-1", "ISO 8859-1 Latin 1; Western European (ISO)"}},
	{28592, {"ISO-8859-2", "ISO 8859-2 Central European; Central European (ISO)"}},
	{28595, {"ISO-8859-5", "ISO 8859-5 Cyrillic"}},
	{28597, {"ISO-8859-7", "ISO 8859-7 Greek"}},
	{28599, {"ISO-8859-9", "ISO 8859-9 Turkish"}},
	{28605, {"ISO-8859-15", "ISO 8859-15 Latin 9"}},
	{50220, {"ISO-2022-JP", "ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS)"}},
	{50225, {"ISO-2022-KR", "ISO 2022 Korean"}},
	{51932, {"EUC-JP", "EUC Japanese"}},
	{51936, {"EUC-CN", "EUC Simplified Chinese; Chinese Simplified (EUC)"}},
	{51949, {"EUC-KR", "EUC Korean"}},
	{52936, {"HZ-GB-2312", "HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ)"}},
	{54936, {"GB18030", "Windows XP and later: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030)"}},
	{65000, {"UTF-7", "Unicode (UTF-7)"}},
	{65001, {"UTF-8", "Unicode (UTF-8)"}}
};

// ����������Ƶ�����ҳ��ӳ���
const std::unordered_map<std::string, std::uint16_t> UniConv::m_encodingToCodePageMap = {
	{"UTF-8", 65001},          // Unicode (UTF-8)
	{"ANSI_X3.4-1968", 20127}, // US-ASCII
	{"ISO-8859-1", 28591},     // Latin-1
	{"ISO-8859-2", 28592},     // Latin-2
	{"ISO-8859-3", 28593},     // Latin-3
	{"ISO-8859-4", 28594},     // Latin-4 (Baltic)
	{"ISO-8859-5", 28595},     // Cyrillic
	{"ISO-8859-6", 28596},     // Arabic
	{"ISO-8859-7", 28597},     // Greek
	{"ISO-8859-8", 28598},     // Hebrew
	{"ISO-8859-9", 28599},     // Latin-5 (Turkish)
	{"ISO-8859-10", 28600},    // Latin-6 (Nordic)
	{"ISO-8859-11", 28601},    // Thai
	{"ISO-8859-13", 28603},    // Latin-7 (Baltic Rim)
	{"ISO-8859-14", 28604},    // Latin-8 (Celtic)
	{"ISO-8859-15", 28605},    // Latin-9 (Western European with Euro)
	{"ISO-8859-16", 28606},    // Latin-10 (South-Eastern European)
	{"GB2312", 936},           // Simplified Chinese
	{"GBK", 936},              // Simplified Chinese (GBK)
	{"GB18030", 54936},        // Simplified Chinese (GB18030)
	{"BIG5", 950},             // Traditional Chinese
	{"EUC-JP", 20932},         // Japanese
	{"EUC-KR", 51949},         // Korean
	{"KOI8-R", 20866},         // Russian
	{"KOI8-U", 21866},         // Ukrainian
	{"Windows-1250", 1250},    // Central European (Windows)
	{"Windows-1251", 1251},    // Cyrillic (Windows)
	{"Windows-1252", 1252},    // Western European (Windows)
	{"Windows-1253", 1253},    // Greek (Windows)
	{"Windows-1254", 1254},    // Turkish (Windows)
	{"Windows-1255", 1255},    // Hebrew (Windows)
	{"Windows-1256", 1256},    // Arabic (Windows)
	{"Windows-1257", 1257},    // Baltic (Windows)
	{"Windows-1258", 1258},    // Vietnamese (Windows)
	{"Shift_JIS", 932},        // Japanese (Shift-JIS)
	{"CP932", 932},            // Japanese (Shift-JIS, Windows)
	{"CP949", 949},            // Korean (Unified Hangul Code, Windows)
	{"CP950", 950},            // Traditional Chinese (Big5, Windows)
	{"CP866", 866},            // Cyrillic (DOS)
	{"CP850", 850},            // Western European (DOS)
	{"CP852", 852},            // Central European (DOS)
	{"CP855", 855},            // Cyrillic (DOS, primarily Russian)
	{"CP857", 857},            // Turkish (DOS)
	{"CP860", 860},            // Portuguese (DOS)
	{"CP861", 861},            // Icelandic (DOS)
	{"CP862", 862},            // Hebrew (DOS)
	{"CP863", 863},            // French Canadian (DOS)
	{"CP864", 864},            // Arabic (DOS)
	{"CP865", 865},            // Nordic (DOS)
	{"CP869", 869},            // Modern Greek (DOS)
	{"CP874", 874},            // Thai (Windows)
	{"CP1250", 1250},          // Central European (Windows)
	{"CP1251", 1251},          // Cyrillic (Windows)
	{"CP1252", 1252},          // Western European (Windows)
	{"CP1253", 1253},          // Greek (Windows)
	{"CP1254", 1254},          // Turkish (Windows)
	{"CP1255", 1255},          // Hebrew (Windows)
	{"CP1256", 1256},          // Arabic (Windows)
	{"CP1257", 1257},          // Baltic (Windows)
	{"CP1258", 1258},          // Vietnamese (Windows)
	{"MacRoman", 10000},       // Western European (Mac)
	{"MacCyrillic", 10007},    // Cyrillic (Mac)
	{"MacGreek", 10006},       // Greek (Mac)
	{"MacTurkish", 10081},     // Turkish (Mac)
	{"MacIcelandic", 10079},   // Icelandic (Mac)
	{"MacCentralEurope", 10029}, // Central European (Mac)
	{"MacThai", 10021},        // Thai (Mac)
	{"MacJapanese", 10001},    // Japanese (Mac)
	{"MacChineseTrad", 10002}, // Traditional Chinese (Mac)
	{"MacChineseSimp", 10008}, // Simplified Chinese (Mac)
	{"MacKorean", 10003},      // Korean (Mac)
	{"MacArabic", 10004},      // Arabic (Mac)
	{"MacHebrew", 10005},      // Hebrew (Mac)
	{"TIS-620", 874},          // Thai (TIS-620)
	{"ISCII-DEVANAGARI", 57002}, // ISCII Devanagari
	{"ISCII-BENGALI", 57003},  // ISCII Bangla
	{"ISCII-TAMIL", 57004},    // ISCII Tamil
	{"ISCII-TELUGU", 57005},   // ISCII Telugu
	{"ISCII-ASSAMESE", 57006}, // ISCII Assamese
	{"ISCII-ORIYA", 57007},    // ISCII Odia
	{"ISCII-KANNADA", 57008},  // ISCII Kannada
	{"ISCII-MALAYALAM", 57009}, // ISCII Malayalam
	{"ISCII-GUJARATI", 57010}, // ISCII Gujarati
	{"ISCII-PUNJABI", 57011},  // ISCII Punjabi
	{"VISCII", 1258},          // Vietnamese (VISCII)
	{"VPS", 1258},             // Vietnamese (VPS)
	{"UTF-16", 1200},          // Unicode UTF-16 (Little Endian)
	{"UTF-16BE", 1201},        // Unicode UTF-16 (Big Endian)
	{"UTF-32", 12000},         // Unicode UTF-32 (Little Endian)
	{ "UTF-32BE", 12001 },       // Unicode UTF-32 (Big Endian)
	{ "UTF-7", 65000 },          // Unicode UTF-7
	{ "HZ-GB-2312", 52936 },     // HZ-GB2312 Simplified Chinese
	{ "ISO-2022-JP", 50220 },    // Japanese (ISO-2022-JP)
	{ "ISO-2022-KR", 50225 },    // Korean (ISO-2022-KR)
	{ "ISO-2022-CN", 50227 },    // Simplified Chinese (ISO-2022-CN)
	{ "EUC-TW", 51950 },         // Traditional Chinese (EUC-TW)
	{ "ARMSCII-8", 0 },          // Armenian (ARMSCII-8, no Windows code page)
	{ "GEORGIAN-ACADEMY", 0 },   // Georgian (Academy, no Windows code page)
	{ "GEORGIAN-PS", 0 },        // Georgian (PS, no Windows code page)
	{ "TSCII", 0 },              // Tamil (TSCII, no Windows code page)
	{ "RK1048", 0 },             // Kazakh (RK1048, no Windows code page)
	{ "MULELAO-1", 0 },          // Lao (MULELAO-1, no Windows code page)
	{ "TCVN", 1258 },            // Vietnamese (TCVN)
	{ "VISCII1.1", 1258 },       // Vietnamese (VISCII 1.1)
	{ "VISCII1.1-HYBRID", 1258 }, // Vietnamese (VISCII 1.1 Hybrid)

};
std::unordered_map<std::string, UniConv::IconvSharedPtr> UniConv::m_iconvDesscriptorCacheMapS = {};


std::string UniConv::GetCurrentSystemEncoding()
{
	std::stringstream ss;	
#ifdef _WIN32
	UINT codePage = GetACP();
	auto it = m_encodingMap.find(codePage);
	if (it != m_encodingMap.end()) ss << it->second.dotNetName;

#endif // _WIN32


#ifdef __linux__
	setlocale(LC_ALL, "");
	char* locstr = setlocale(LC_CTYPE, NULL);
	char* encoding = nl_langinfo(CODESET);
	ss << encoding;
#endif // __linux__
	if (ss.str().empty()) ss << "Encoding not found.";
	

	return ss.str();
}

std::uint16_t UniConv::GetCurrentSystemEncodingCodePage()
{
#ifdef _WIN32
	UINT codePage = GetACP();
	return static_cast<std::uint16_t>(codePage);
#endif // _WIN32


#ifdef __linux__
	setlocale(LC_ALL, "");
	char* locstr = setlocale(LC_CTYPE, NULL);
	char* encoding = nl_langinfo(CODESET);
	auto it = m_encodingToCodePage.find(encoding);

	if (it != m_encodingToCodePage.end()) return it->second;
	else {
		// �����������δ��ӳ������ҵ�������Ĭ��ֵ��UTF-8��
		std::cerr << "Warning: Encoding '" << encoding << "' not found in mapping table. Defaulting to UTF-8 (65001)." << std::endl;
		return 65001;
	}

#endif // __linux__
	return 0;

}

std::string  UniConv::GetEncodingNameByCodePage(std::uint16_t codePage)
{
	auto it = m_encodingMap.find(codePage);
	if (it != m_encodingMap.end())
		return it->second.dotNetName;
	else
		return "Encoding not found.";
}

std::string UniConv::LocaleConvertToUtf8(const std::string& sInput)
{	
	return LocaleConvertToUtf8(sInput.c_str());
}

std::string UniConv::LocaleConvertToUtf8(const char* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(sInput, currentEncoding.c_str(), UniConv::utf_8_encoding);
	if (res) {
		return std::move(res.conv_result_str);
	}
	
	return std::string(res.error_msg);
}

std::string UniConv::Utf8ConvertToLocale(const char* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
    auto res = Convert(sInput, UniConv::utf_8_encoding, currentEncoding.c_str());
	if (res) {
        return std::move(res.conv_result_str);
	}
	
    return std::string(res.error_msg);
}

std::u16string UniConv::LocaleConvertToUtf16LE(const char* sInput)
{   
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(sInput, currentEncoding.c_str(), UniConv::utf_16le_encoding);

	if (res && res.conv_result_str.size() % sizeof(char16_t) == 0) {
		const char16_t* p = reinterpret_cast<const char16_t*>(res.conv_result_str.data());
		return std::u16string(p, res.conv_result_str.size() / sizeof(char16_t));
	}	
	return std::u16string(reinterpret_cast<const char16_t*>(sInput), strlen(sInput) / sizeof(char16_t));
}




std::u16string UniConv::LocaleConvertToUtf16LE(const std::string& sInput)
{
	return LocaleConvertToUtf16LE(sInput.c_str());
}

std::u16string UniConv::LocaleConvertToUtf16BE(const std::string& sInput)
{
	return LocaleConvertToUtf16BE(sInput.c_str());
}

std::u16string UniConv::LocaleConvertToUtf16BE(const char* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(sInput, currentEncoding.c_str(), UniConv::utf_16be_encoding);

	if (res && res.conv_result_str.size() % sizeof(char16_t) == 0) {		
		const char16_t* p = reinterpret_cast<const char16_t*>(res.conv_result_str.data());
		return std::u16string(p, res.conv_result_str.size() / sizeof(char16_t));
	}
	std::cout << __FUNCTION__ << "Convert failed Error:" << res.error_msg << std::endl;
	return std::u16string(reinterpret_cast<const char16_t*>(sInput), strlen(sInput) / sizeof(char16_t));
	
}

std::string UniConv::Utf16BEConvertToLocale(const std::u16string& sInput)
{
	return Utf16BEConvertToLocale(sInput.c_str());
}


std::string UniConv::Utf16BEConvertToLocale(const char16_t* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	
	auto res = Convert(reinterpret_cast<const char*>(sInput), UniConv::utf_16be_encoding, currentEncoding.c_str());
	if (res) {
		return std::move(res.conv_result_str);
	}
	return std::string(res.error_msg);
}

std::string UniConv::Utf16LEConvertToUtf8(const char16_t* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res    = Convert(reinterpret_cast<const char*>(sInput), UniConv::utf_16le_encoding, UniConv::utf_8_encoding);
	if (res.IsSuccess()) {
        return std::move(res.conv_result_str);
	}
	return std::string(res.error_msg);
}

std::string UniConv::Utf16LEConvertToUtf8(const std::u16string& sInput)
{
	return Utf16LEConvertToUtf8(sInput.c_str());
}

std::string UniConv::Utf16BEConvertToUtf8(const std::u16string& sInput)
{
	return Utf16BEConvertToUtf8(sInput.c_str());
}

std::string UniConv::Utf16BEConvertToUtf8(const char16_t* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
    auto res = Convert(reinterpret_cast<const char*>(sInput), UniConv::utf_16be_encoding, UniConv::utf_8_encoding);
	if (res) {
        return std::move(res.conv_result_str);
	}
	return std::string(res.error_msg);
}

std::u16string UniConv::Utf8ConvertToUtf16LE(const char* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(sInput, UniConv::utf_8_encoding, UniConv::utf_16le_encoding);
	if (res) {
		return std::u16string(reinterpret_cast<const char16_t*>(res.conv_result_str.data()), res.conv_result_str.size() / sizeof(char16_t));
	}
	return std::u16string(reinterpret_cast<const char16_t*>(res.error_msg,res.error_msg.size() / sizeof(char16_t)));
}

std::u16string UniConv::Utf8ConvertToUtf16LE(const std::string& sInput)
{
	return Utf8ConvertToUtf16LE(sInput.c_str());
}

std::u16string UniConv::Utf8ConvertToUtf16BE(const char* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(sInput, UniConv::utf_8_encoding, UniConv::utf_16be_encoding);
	if (res) {
#ifdef DEBUG
		std::cout << __FUNCTION__ << "Convert success" << std::endl;
#endif
		return std::u16string(reinterpret_cast<const char16_t*>(res.conv_result_str.data()), res.conv_result_str.size() / sizeof(char16_t));
	}
#ifdef DEBUG
	std::cout << __FUNCTION__ << " Convert failed " << sInput << std::endl;
#endif
	return std::u16string(reinterpret_cast<const char16_t*>(res.error_msg, res.error_msg.size() / sizeof(char16_t)));
}



std::u16string UniConv::Utf8ConvertToUtf16BE(const std::string& sInput)
{
	return Utf8ConvertToUtf16BE(sInput.c_str());
}

std::u16string UniConv::Utf16LEConvertToUtf16BE(const std::u16string& sInput)
{
	return Utf16LEConvertToUtf16BE(sInput.c_str());
}

std::u16string UniConv::Utf16LEConvertToUtf16BE(const char16_t* sInput)
{
	std::string currentEncoding = GetCurrentSystemEncoding();
	auto res = Convert(reinterpret_cast<const char*>(sInput), UniConv::utf_16le_encoding, UniConv::utf_16be_encoding);
	if (res) {
#ifdef DEBUG
		std::cout << __FUNCTION__ << "Convert success" << std::endl;
#endif
		return std::u16string(reinterpret_cast<const char16_t*>(res.conv_result_str.data()), res.conv_result_str.size() / sizeof(char16_t));
	}
#ifdef DEBUG
	std::cout << __FUNCTION__ << " Convert failed " << sInput << std::endl;
#endif
	return std::u16string(reinterpret_cast<const char16_t*>(res.error_msg, res.error_msg.size() / sizeof(char16_t)));
}

std::string UniConv::Utf8ConvertToLocale(const std::string& sInput)
{
	return Utf8ConvertToLocale(sInput.c_str());
}

UniConv::IConvResult UniConv::Convert(std::string_view in, const char* fromcode, const char* tocode) {
	// ת�����صĽ��
	IConvResult iconv_result;

	// ��ȡ iconv ��������ֱ��ʹ�� IconvUniquePtr��
	auto cd = GetIconvDescriptorS(fromcode, tocode);	
	if (!cd || (cd.get() == reinterpret_cast<iconv_t>(-1))) {

		iconv_result.error_code = errno;
		iconv_result.error_msg = GetIconvErrorString(iconv_result.error_code);
		return iconv_result;
	}

	// ���뻺����
	std::vector<char> in_buffer(in.begin(), in.end());
	const char* inbuf_ptr = in_buffer.data(); // ���뻺��
	std::size_t inbuf_letf = in.size(); // ���뻺��ʣ�೤��

	// ���������
	constexpr std::size_t initial_buffer_size = 4096;
	std::vector<char> out_buffer(initial_buffer_size);
	std::string converted_result;
	converted_result.reserve(in.size() * 2); // Ԥ����ռ�

	while (true) {
		char*       out_ptr  = out_buffer.data();
		std::size_t out_left = out_buffer.size();

		// ִ��ת��
		std::size_t ret = iconv(cd.get(), &inbuf_ptr, &inbuf_letf, &out_ptr, &out_left);
		// д����ת��������
		converted_result.append(out_buffer.data(), out_buffer.size() - out_left);
		if (static_cast<std::size_t>(-1) == ret) {
			iconv_result.error_code = errno;
			iconv_result.error_msg = GetIconvErrorString(iconv_result.error_code);
			break;
		}
		// ��̬��չ������
		if (out_left < 128 && out_buffer.size() < 1048576) { // ���1MB
			out_buffer.resize(out_buffer.size() * 2);
			continue;
		}


		// ��������Ƿ������
		if (inbuf_letf == 0) {
			// ˢ��ת�������ڲ�״̬
			out_ptr = out_buffer.data();
			out_left = out_buffer.size();
			ret = iconv(cd.get(), nullptr, &inbuf_letf, &out_ptr, &out_left);
			converted_result.append(out_buffer.data(), out_buffer.size() - out_left);

			if (static_cast<std::size_t>(-1) == ret) {
				iconv_result.error_code = errno;
				iconv_result.error_msg = GetIconvErrorString(iconv_result.error_code);
			}
			break;
		}
	}

	// ����ת�����
	if (iconv_result.error_code == 0) {
		converted_result.shrink_to_fit();
		iconv_result.conv_result_str = std::move(converted_result);
	}
	return iconv_result;
}


std::string_view UniConv::GetIconvErrorString(int err_code)
{
	auto it = m_iconvErrorMap.find(err_code);
	if (it != m_iconvErrorMap.end()) {
		return it->second;
	}
	return std::generic_category().message(err_code);

}

//UniConv::IconvUniquePtr  UniConv::GetIconvDescriptor(const char* fromcode, const char* tocode)
//{
//	//����key
//	std::string key = std::string(fromcode) + ":" + tocode;
//
//	std::cout << key << std::endl;
//
//	std::lock_guard<std::mutex> lock(m_iconvcCacheMutex);
//	// ���һ���
//	auto it = m_iconvDesscriptorCacheMap.find(key);
//
//	if (it != m_iconvDesscriptorCacheMap.end()) {
//		// ���ػ������������ֱ�ӷ��� IconvUniquePtr�� �˴�����ʹ��get()��ȡԭʼָ���������µ�IconvUniquePtr
//		return UniConv::IconvUniquePtr(it->second.release());
//	}
//
//	//���µ��µ�iconv ������
//	iconv_t cd = iconv_open(tocode, fromcode);
//	if (cd == reinterpret_cast<iconv_t>(-1)) {
//		std::cout <<"iconv_open error" << std::endl;
//		return IconvUniquePtr(nullptr); 
//	}
//	auto result = m_iconvDesscriptorCacheMap.emplace(key, IconvUniquePtr(cd));
//	if (!result.second) {
//		std::cerr << "Failed to insert into cache" << std::endl;
//		return IconvUniquePtr(nullptr);
//	}
//	//���ػ����
//	return IconvUniquePtr(result.first->second.release());
//}

UniConv::IconvSharedPtr UniConv::GetIconvDescriptorS(const char* fromcode, const char* tocode)
{
	std::string key = std::string(fromcode) + ":" + tocode;
	std::lock_guard<std::mutex> lock(m_iconvcCacheMutex);

	auto it = m_iconvDesscriptorCacheMapS.find(key);
	if (it != m_iconvDesscriptorCacheMapS.end()) {
		return it->second; // ���� shared_ptr �Ŀ���
	}

	iconv_t cd = iconv_open(tocode, fromcode);
	if (cd == reinterpret_cast<iconv_t>(-1)) {
		std::cout << "iconv_open error" << std::endl;
		return nullptr;
	}

	auto iconvPtr = std::shared_ptr<std::remove_pointer_t<iconv_t>>(cd, IconvDeleter());
	m_iconvDesscriptorCacheMapS.emplace(key, iconvPtr);
	return iconvPtr;
}



