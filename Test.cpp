
#include "gtest/gtest.h"
#include "UniConv.h"
#include <fstream>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// ��������Ŀ¼
const std::string kTestDataDir = "testdata/";




// д���ļ���ָ�����룬ֱ��д�����ƣ�
void WriteFile(const std::string& path, const std::string& data) {
    ofstream ofs(path, ios::binary);
    ofs.write(data.data(), data.size());
}

// ��ȡ�ļ�Ϊstring
std::string ReadFile_(const std::string& path) {
    ifstream ifs(path, ios::binary);
    return std::string((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
}

// ͳһ���ļ���ȡ���������ַ�������Ϊ���в�������Դ
std::string GetSampleUtf8() {
	return ReadFile_("testdata/sample_utf8.txt");
}

// ���ɶ��ֱ���Ĳ����ļ�
void GenerateTestFiles() {
    fs::create_directories(kTestDataDir);
    std::string sample = GetSampleUtf8();
    // UTF-8
    WriteFile(kTestDataDir + "utf8.txt", sample);
    // ���ر��루����ΪGBK/CP936����ʵ�ʻ���֧�֣�
    std::string local = UniConv::GetInstance().get()->FromUtf8ToLocal(sample);
    WriteFile(kTestDataDir + "local.txt", local);
    // UTF-16LE
    std::u16string u16le = UniConv::GetInstance().get()->FromUtf8ToUtf16LE(sample);
    WriteFile(kTestDataDir + "utf16le.txt", std::string(reinterpret_cast<const char*>(u16le.data()), u16le.size()*2));
    // UTF-16BE
    std::u16string u16be = UniConv::GetInstance().get()->FromUtf8ToUtf16BE(sample);
    WriteFile(kTestDataDir + "utf16be.txt", std::string(reinterpret_cast<const char*>(u16be.data()), u16be.size()*2));
}

class UniConvTest : public ::testing::Test {
protected:
    void SetUp() override {
        GenerateTestFiles();
    }
};

TEST_F(UniConvTest, ToUtf8FromLocal) {
    std::string local = ReadFile_(kTestDataDir + "local.txt");
    std::string utf8 = UniConv::GetInstance().get()->ToUtf8FromLocal(local);
    EXPECT_EQ(utf8, GetSampleUtf8());
}

TEST_F(UniConvTest, FromUtf8ToLocal) {
    std::string utf8 = ReadFile_(kTestDataDir + "utf8.txt");
    std::string local = UniConv::GetInstance().get()->FromUtf8ToLocal(utf8);
    std::string reread = UniConv::GetInstance().get()->ToUtf8FromLocal(local);
    EXPECT_EQ(reread, GetSampleUtf8());
}

TEST_F(UniConvTest, Utf8_Utf16LE) {
    std::string utf8 = ReadFile_(kTestDataDir + "utf8.txt");
    std::u16string u16le = UniConv::GetInstance().get()->FromUtf8ToUtf16LE(utf8);
    std::string u16le_bin(reinterpret_cast<const char*>(u16le.data()), u16le.size()*2);
    std::string file_bin = ReadFile_(kTestDataDir + "utf16le.txt");
    EXPECT_EQ(u16le_bin, file_bin);
    // ����
    std::string utf8_back = UniConv::GetInstance().get()->FromUtf16LEToUtf8(u16le);
    EXPECT_EQ(utf8_back, GetSampleUtf8());
}

TEST_F(UniConvTest, Utf8_Utf16BE) {
    std::string utf8 = ReadFile_(kTestDataDir + "utf8.txt");
    std::u16string u16be = UniConv::GetInstance().get()->FromUtf8ToUtf16BE(utf8);
    std::string u16be_bin(reinterpret_cast<const char*>(u16be.data()), u16be.size()*2);
    std::string file_bin = ReadFile_(kTestDataDir + "utf16be.txt");
    EXPECT_EQ(u16be_bin, file_bin);
    // ����
    std::string utf8_back = UniConv::GetInstance().get()->FromUtf16BEToUtf8(u16be);
    EXPECT_EQ(utf8_back, GetSampleUtf8());
}

TEST_F(UniConvTest, Utf16LE_Utf16BE) {
    std::string u16le_bin = ReadFile_(kTestDataDir + "utf16le.txt");
    std::u16string u16le(reinterpret_cast<const char16_t*>(u16le_bin.data()), u16le_bin.size()/2);
    std::u16string u16be = UniConv::GetInstance().get()->FromUtf16LEToUtf16BE(u16le);
    std::string u16be_bin(reinterpret_cast<const char*>(u16be.data()), u16be.size()*2);
    std::string file_bin = ReadFile_(kTestDataDir + "utf16be.txt");
    EXPECT_EQ(u16be_bin, file_bin);
    // ����
    std::u16string u16le2 = UniConv::GetInstance().get()->FromUtf16BEToUtf16LE(u16be);
    std::string u16le_bin2(reinterpret_cast<const char*>(u16le2.data()), u16le2.size()*2);
    EXPECT_EQ(u16le_bin2, u16le_bin);
}

// �ɼ������������ӿڵĲ������� ...

int main(int argc, char **argv) {
	system("chcp 65001"); // ���ÿ���̨����ΪUTF-8
    std::cout << "Current System Encoding: " << UniConv::GetInstance().get()->GetCurrentSystemEncoding() << std::endl;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}