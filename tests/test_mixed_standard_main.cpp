#include <cstddef>

extern "C" std::size_t uniconv_cpp17_layout_signature();
extern "C" std::size_t uniconv_cpp20_layout_signature();

int main() {
    const auto cpp17_signature = uniconv_cpp17_layout_signature();
    const auto cpp20_signature = uniconv_cpp20_layout_signature();
    return cpp17_signature != 0U && cpp17_signature == cpp20_signature ? 0 : 1;
}
