#include <cstddef>
#include <string>

#include <UniConv/UniConv.h>

extern "C" std::size_t uniconv_cpp17_layout_signature() {
    UniConv converter;
    std::string output;
    const std::string input = "cpp17";
    const auto error = converter.ConvertEncodingFast(input, "UTF-8", "UTF-8", output);
    return error == ErrorCode::Success
        ? sizeof(UniConv) ^ (alignof(UniConv) << 1U)
        : 0U;
}
