/**
 * @file test_basic.cpp
 * @brief UniConv 基础功能测试 - 验证 Google Test 框架集成与核心 API
 */

#include <gtest/gtest.h>
#include <uniconv/UniConv.h>
#include <memory>
#include <string>

// ============================================================================
// 测试：提供共享的 UniConv 实例
// ============================================================================
class UniConvTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = UniConv::Create();
        ASSERT_NE(converter, nullptr);
    }

    std::unique_ptr<UniConv> converter;
};

// ============================================================================
// 实例创建测试
// ============================================================================
TEST(UniConvInstanceTest, CreateReturnsValidInstance) {
    auto inst = UniConv::Create();
    EXPECT_NE(inst, nullptr);
}

TEST(UniConvInstanceTest, StackAllocationWorks) {
    UniConv converter;
    auto result = converter.ConvertEncodingFast("hello", "UTF-8", "UTF-16LE");
    EXPECT_TRUE(result.IsSuccess());
}

TEST(UniConvInstanceTest, MultipleInstancesAreIndependent) {
    auto inst1 = UniConv::Create();
    auto inst2 = UniConv::Create();
    ASSERT_NE(inst1, nullptr);
    ASSERT_NE(inst2, nullptr);

    auto r1 = inst1->ConvertEncodingFast("test", "UTF-8", "UTF-16LE");
    auto r2 = inst2->ConvertEncodingFast("test", "UTF-8", "UTF-16LE");
    EXPECT_TRUE(r1.IsSuccess());
    EXPECT_TRUE(r2.IsSuccess());
    EXPECT_EQ(r1.GetValue(), r2.GetValue());
}

// ============================================================================
// 基础编码转换测试 (使用测试)
// ============================================================================
TEST_F(UniConvTest, Utf8ToUtf16LE) {
    auto result = converter->ConvertEncodingFast("Hello", "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(UniConvTest, Utf8ToUtf16LEWithChinese) {
    auto result = converter->ConvertEncodingFast("你好世界", "UTF-8", "UTF-16LE");
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
}

TEST_F(UniConvTest, Utf8RoundTrip) {
    std::string original = "Hello, 世界! 🌍";
    auto to16 = converter->ConvertEncodingFast(original, "UTF-8", "UTF-16LE");
    ASSERT_TRUE(to16.IsSuccess());

    auto back = converter->ConvertEncodingFast(to16.GetValue(), "UTF-16LE", "UTF-8");
    ASSERT_TRUE(back.IsSuccess());
    EXPECT_EQ(back.GetValue(), original);
}

TEST_F(UniConvTest, EmptyInputReturnsSuccess) {
    auto result = converter->ConvertEncodingFast("", "UTF-8", "UTF-16LE");
    EXPECT_TRUE(result.IsSuccess());
}

// ============================================================================
// 错误处理测试
// ============================================================================
TEST_F(UniConvTest, InvalidSourceEncodingFails) {
    auto result = converter->ConvertEncodingFast("test", "INVALID_ENCODING", "UTF-8");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(UniConvTest, InvalidTargetEncodingFails) {
    auto result = converter->ConvertEncodingFast("test", "UTF-8", "INVALID_ENCODING");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(UniConvTest, NullSourceEncodingFails) {
    auto result = converter->ConvertEncodingFast("test", nullptr, "UTF-8");
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(UniConvTest, NullTargetEncodingFails) {
    auto result = converter->ConvertEncodingFast("test", "UTF-8", nullptr);
    EXPECT_FALSE(result.IsSuccess());
}
