#if _MSC_VER >= 1600 
#pragma execution_character_set("utf-8")
#endif

#include "UniConv.h"
#include "LightLogWriteImpl.hpp"
#include <iostream>
#include <windows.h>



constexpr const char* LOG_INFO  = "[  INFO   ]";
constexpr const char* LOG_ERROR = "[  ERROR  ]";
constexpr const char* LOG_DEBUG = "[  DEBUG  ]";
constexpr const char* LOG_WARN  = "[  WARN   ]";
constexpr const char* LOG_FATAL = "[  FATAL  ]";
constexpr const char* LOG_TRACE = "[  TRACE  ]";
constexpr const char* LOG_OK    = "[   OK    ]";

auto g_conv = UniConv::GetInstance();

#define LOGERROR(msg) glogger.WriteLogContent(LOG_ERROR, msg)
#define LOGINFO(msg)  glogger.WriteLogContent(LOG_INFO, msg)
#define LOGDEBUG(msg) glogger.WriteLogContent(LOG_DEBUG, msg)
#define LOGWARN(msg)  glogger.WriteLogContent(LOG_WARN, msg)
#define LOGFATAL(msg) glogger.WriteLogContent(LOG_FATAL, msg)
#define LOGTRACE(msg) glogger.WriteLogContent(LOG_TRACE, msg)
#define LOGOK(msg)    glogger.WriteLogContent(LOG_OK, msg)



struct ConversionTask {
    std::string inputFile;
    std::string outputFile;
    std::string fromEncoding;
    std::string toEncoding;
    std::string description;
};



std::string UTF8    = UniConv::ToString(UniConv::Encoding::utf_8);
std::string GBK     = UniConv::ToString(UniConv::Encoding::gbk);
std::string UTF16LE = UniConv::ToString(UniConv::Encoding::utf_16le);
std::string UTF16BE = UniConv::ToString(UniConv::Encoding::utf_16be);


ConversionTask task_gb2312_to_utf8 = {
    "testdata/input_gb2312.txt",
    "testdata/output/output_gb2312_to_utf-8.txt",
	"GBK",
	"UTF-8",
	"GBK -> UTF-8"
};



std::vector<ConversionTask> tasks = {
    task_gb2312_to_utf8
};

// Convert the byte data to a hexadecimal string
std::string BytesToHex(const std::string& data) {
    std::ostringstream oss;
    for (unsigned char c : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c) << " ";
    }
    return oss.str();
}


std::string ReadFileBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool WriteFileBytes(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return false;
    }
    file.write(data.data(), data.size());
    return file.good();
}

// check file encoding and remove BOM if exists
std::pair<std::string, std::string> DetectEncodingAndRemoveBOM(const std::string& data) {
	if (data.empty()) {
		return std::make_pair("UTF-8", data);
	}

	// ���BOM
	if (data.size() >= 3 &&
		static_cast<unsigned char>(data[0]) == 0xEF &&
		static_cast<unsigned char>(data[1]) == 0xBB &&
		static_cast<unsigned char>(data[2]) == 0xBF) {
		return std::make_pair("UTF-8", data.substr(3));
	}

	if (data.size() >= 2 &&
		static_cast<unsigned char>(data[0]) == 0xFF &&
		static_cast<unsigned char>(data[1]) == 0xFE) {
		return std::make_pair("UTF-16LE", data.substr(2));
	}

	if (data.size() >= 2 &&
		static_cast<unsigned char>(data[0]) == 0xFE &&
		static_cast<unsigned char>(data[1]) == 0xFF) {
		return std::make_pair("UTF-16BE", data.substr(2));
	}

	
	return std::make_pair("", data);
}



void InitializeLogging() {
    
    glogger.SetLogsFileName("log/test_log.log");
}



/// <summary>
/// Test GetCurrentSystemEncoding API
/// </summary>
void TestGetCurrentSystemEncoding() {
	std::string sCurSysEnc =  g_conv->GetCurrentSystemEncoding();
    LOGINFO("Current system encoding:\t" + sCurSysEnc);
}
/// <summary>
/// Test GetCurrentSystemEncodingCodePage API
/// </summary>
void TestGetCurrentSystemEncodingCodePage() {
    int nCurSysEncCodePage = g_conv->GetCurrentSystemEncodingCodePage();
    LOGINFO("Current system codepage:\t" + std::to_string(nCurSysEncCodePage));
}

void TestGetEncodingNameByCodePage() {
    int codepage = g_conv->GetCurrentSystemEncodingCodePage();
    LOGINFO("Current system codepage:\t" + std::to_string(codepage));
    std::string encodingName = g_conv->GetEncodingNameByCodePage(codepage);
    LOGINFO("Encoding name for codepage " + std::to_string(codepage) + ":\t" + encodingName);
    std::string convResult = g_conv->GetEncodingNameByCodePage(codepage);
    if (convResult != encodingName) {
        LOGERROR("Encoding name mismatch for codepage " + std::to_string(codepage) + ": expected '" + encodingName + "', got '" + convResult + "'");
    } else {
        LOGOK("Encoding name for codepage " + std::to_string(codepage) + " is correct: " + convResult);
    }
}


void BatchConvertFiles() {
    std::cout << "=== Start the batch file conversion test === " << std::endl;
    LOGINFO("=== Start the batch file conversion test === ");

    for (const auto& task : tasks) {
        LOGINFO("--- " + task.description + " ---");
        std::cout << "--- " + task.description + " ---" << std::endl;
        std::string input_data = ReadFileBytes(task.inputFile);
        if( input_data.empty() ) {
            LOGERROR("Error: cannot read file\t" + task.inputFile);
			std::cout << "Error: cannot read file\t" + task.inputFile << std::endl;
        }

        // Check and remove BOM 
		auto result_pair = DetectEncodingAndRemoveBOM(input_data);
		std::string detectedEncoding = result_pair.first;
		std::string cleanedData = result_pair.second;
		std::string actualFromEncoding = detectedEncoding.empty() ? task.fromEncoding : detectedEncoding;

        // Log info
		LOGINFO("Input file: " + task.inputFile);
        LOGINFO("Input file size: " + std::to_string(input_data.size()) + " bytes");
		LOGINFO("Detected encoding: " + (detectedEncoding.empty()) ? "NO BOM" : detectedEncoding);
		LOGINFO("Clean data size: " + std::to_string(cleanedData.size()) + " bytes");
		LOGINFO("Input data hex: " + BytesToHex(cleanedData));

        std::cout << "Input file: " + task.inputFile << std::endl;
		std::cout << "Input file size: " + std::to_string(input_data.size()) + " bytes" << std::endl;
		std::cout << "Detected encoding: " + (detectedEncoding.empty() ? "NO BOM" : detectedEncoding) << std::endl;
		std::cout << "Clean data size: " + std::to_string(cleanedData.size()) + " bytes" << std::endl;
        std::cout << "Input data hex: " + BytesToHex(cleanedData) << std::endl;

		auto result = g_conv->ConvertEncoding(cleanedData, actualFromEncoding.c_str(), task.toEncoding.c_str());

        if (result.IsSuccess()) {
            LOGOK("Conversion successful: " + task.fromEncoding + " -> " + task.toEncoding);
			std::cout << "Conversion successful: " + task.fromEncoding + " -> " + task.toEncoding << std::endl;

            LOGINFO("Output file: " + task.outputFile);
            std::cout << "Output file: " + task.outputFile << std::endl;
            
            // Write the converted data to the output file
            if (WriteFileBytes(task.outputFile, result.conv_result_str)) {
                LOGOK("Output file written successfully: " + task.outputFile);
                std::cout << "Output file written successfully: " + task.outputFile << std::endl;
            } else {
                LOGERROR("Error writing output file: " + task.outputFile);
                std::cout << "Error writing output file: " + task.outputFile << std::endl;
            }
        } 
        else {
            
        }
    }
}


void TestTostring()
{
    std::string utf8 = "UTF-8";
	std::string gbk = "GBK";
    std::string uni_utf8 = UniConv::ToString(UniConv::Encoding::utf_8);
	std::string uni_gbk = UniConv::ToString(UniConv::Encoding::gbk);

    if(uni_utf8 == utf8 && uni_gbk == gbk) {
        LOGOK("UniConv::ToString() works correctly for UTF-8 and GBK.");
    } else {
        LOGERROR("UniConv::ToString() failed for UTF-8 or GBK.");
	}

}


void TestToLocalFromUtf8() {

	std::string inputFile = "testdata/input_utf8.txt";
	std::string outputFile = "testdata/output/output_utf-8_to_local.txt";
	std::string content = ReadFileBytes(inputFile);

	std::string convert = g_conv->ToLocalFromUtf8(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
	}

	WriteFileBytes(outputFile, convert);
	std::cout << "Converted content written to: " + outputFile << std::endl;
	std::cout << content << std::endl;
	std::cout << convert << std::endl;

}

void TestToUtf8FromLocal() {
	std::string inputFile = "testdata/input_gb2312.txt";
	std::string outputFile = "testdata/output/output_gb2312_to_utf-8_2.txt";
	std::string content = ReadFileBytes(inputFile);

	std::string convert = g_conv->ToUtf8FromLocal(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
	}
	WriteFileBytes(outputFile, convert);

	std::cout << content << std::endl;
	std::cout << convert << std::endl;
	LOGINFO("Converted content: " + convert);
}



void TestToUtf16LEFromLocal() {
	std::string inputFile = "testdata/input_gb2312.txt";
	std::string outputFile = "testdata/output/output_gb2312_to_utf-16le.txt";
	std::string content = ReadFileBytes(inputFile);

	std::u16string convert = g_conv->ToUtf16LEFromLocal(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
	}

	WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
	std::cout << "Converted content written to: " + outputFile << std::endl;
	std::cout << content << std::endl;
   
}


void TestToUtf16BEFromLocal() {
	std::string inputFile = "testdata/input_gb2312.txt";
    std::string outputFile = "testdata/output/output_gb2312_to_utf-16be.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string convert = g_conv->ToUtf16BEFromLocal(content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, std::string(reinterpret_cast<const char*>(convert.data()), convert.size() * sizeof(char16_t)));
    std::cout << "Converted content written to: " + outputFile << std::endl;
	std::cout << content << std::endl;

}

/// <summary>
/// Test API ToLocalFromUtf16BE
/// </summary>
void TestToLocalFromUtf16BE() {
	std::string inputFile = "testdata/input_utf16be_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16be_to_local.txt";
    std::string content = ReadFileBytes(inputFile);
	std::u16string utf16be_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    
    auto convert = g_conv->ToLocalFromUtf16BE(utf16be_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
	std::cout << "Converted content written to: " + outputFile << std::endl;
}

/// <summary>
/// Test API ToLocalFromUtf16LE
/// </summary>
void TestToLocalFromUtf16LE() {
    std::string inputFile = "testdata/input_utf16le_nobom.txt";
    std::string outputFile = "testdata/output/output_utf-16le_to_local.txt";
    std::string content = ReadFileBytes(inputFile);
    std::u16string utf16le_content(reinterpret_cast<const char16_t*>(content.data()), content.size() / sizeof(char16_t));
    
    auto convert = g_conv->ToLocalFromUtf16LE(utf16le_content);
    if (convert.empty()) {
        LOGERROR("Conversion failed for file: " + inputFile);
        std::cout << "Conversion failed for file: " + inputFile << std::endl;
        return;
    }
    WriteFileBytes(outputFile, convert);
    std::cout << "Converted content written to: " + outputFile << std::endl;
}

int  main() {

    SetConsoleOutputCP(CP_UTF8);
    InitializeLogging();

	//TestToUtf8FromLocal(); // OK
	//TestToLocalFromUtf8(); // OK
    //TestGetCurrentSystemEncoding();  // OK
	//TestGetEncodingNameByCodePage(); // OK
    //TestToUtf16LEFromLocal(); // OK
    //TestToUtf16BEFromLocal(); // OK
    //TestToLocalFromUtf16BE();   // OK
    //TestToLocalFromUtf16LE();   // OK
    //BatchConvertFiles();

    return 0;
}