#include "convert_tools.h"

// �������� UTF-8 ת��Ϊ std::wstring������Ϊ UCS-4��
std::wstring Utf8ConvertsToUcs4(const std::string& utf8str) {

	try {
		// ���� std::wstring_convert ����ʹ�� std::codecvt_utf8<wchar_t> ����ת��
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		// �� UTF-8 �ַ���ת��Ϊ std::wstring
		return converter.from_bytes(utf8str);
	}
	catch (const std::range_error& e) {
		// ���ת��ʧ�ܣ��������벻����Ч�� UTF-8�����׳��쳣
		throw std::runtime_error("Failed to convert UTF-8 to UCS-4: " + std::string(e.what()));
	}
}




std::string Ucs4ConvertToUtf8(const std::wstring& wstr) {
	try {
		// ���� std::wstring_convert ����ʹ�� std::codecvt_utf8<wchar_t> ����ת��
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		// �� std::wstring ת��Ϊ UTF-8 ����� std::string
		return converter.to_bytes(wstr);
	}
	catch (const std::range_error& e) {
		// ���ת��ʧ�ܣ��������������Ч�Ŀ��ַ������׳��쳣
		throw std::runtime_error("Failed to convert UCS-4 to UTF-8: " + std::string(e.what()));
	}
}



std::wstring U16StringToWString(const std::u16string& u16str)
{
	std::wstring wstr;

#ifdef _WIN32
	// Windows ƽ̨��wchar_t �� 2 �ֽڣ�UTF-16����ֱ�ӿ���
	wstr.assign(u16str.begin(), u16str.end());
#else
	// Linux ƽ̨��wchar_t �� 4 �ֽڣ�UTF-32������Ҫת��
	std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
	wstr = converter.from_bytes(
		reinterpret_cast<const char*>(u16str.data()),
		reinterpret_cast<const char*>(u16str.data() + u16str.size())
	);
#endif

	return wstr;
}