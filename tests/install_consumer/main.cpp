#include <UniConv/UniConv.h>

#include <string>
#include <string_view>

int main() {
    UniConv converter;
    const std::string input = "UniConv \xE4\xB8\xAD\xE6\x96\x87 \xF0\x9F\x98\x80";
    std::string utf16;
    std::string output;
    if (converter.ConvertEncodingFast(input, "UTF-8", "UTF-16LE", utf16) != ErrorCode::Success ||
        converter.ConvertEncodingFast(utf16, "UTF-16LE", "UTF-8", output) != ErrorCode::Success ||
        output != input) {
        return 1;
    }

    // Exercise iconv even in SIMD packages, plus the exported numeric API.
    const std::string gbk = "\xD6\xD0\xCE\xC4";
    if (converter.ConvertEncodingFast(gbk, "GBK", "UTF-8", output) != ErrorCode::Success ||
        output != "\xE4\xB8\xAD\xE6\x96\x87") {
        return 2;
    }
    const auto number = UniConv::ParseInt32(std::string_view{"12345"});
    return number.IsSuccess() && number.GetValue() == 12345 ? 0 : 3;
}
