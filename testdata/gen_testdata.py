import codecs
import locale

# 精简多语言和符号测试数据
test_texts = {
    "cn": "你好\n",
    "en": "Hello\n",
    "emoji": "😀😂👍\n",
    "jp": "こんにちは\n",
    "special": "★♣♦\n",
}

def write_file(filename, content, encoding):
    with open(filename, "w", encoding=encoding, errors="replace") as f:
        f.write(content)

# UTF-8
write_file("input_utf8_cn.txt", test_texts["cn"], "utf-8")
write_file("input_utf8_en.txt", test_texts["en"], "utf-8")
write_file("input_utf8_emoji.txt", test_texts["emoji"], "utf-8")
write_file("input_utf8_jp.txt", test_texts["jp"], "utf-8")
write_file("input_utf8_special.txt", test_texts["special"], "utf-8")

# GBK
write_file("input_gbk_cn.txt", test_texts["cn"], "gbk")
write_file("input_gbk_en.txt", test_texts["en"], "gbk")

# UTF-16LE（无BOM）
with codecs.open("input_utf16le_cn.txt", "w", encoding="utf-16le") as f:
    f.write(test_texts["cn"])
with codecs.open("input_utf16le_en.txt", "w", encoding="utf-16le") as f:
    f.write(test_texts["en"])

# 本地编码
local_enc = locale.getpreferredencoding()
write_file("input_local_cn.txt", test_texts["cn"], local_enc)
write_file("input_local_en.txt", test_texts["en"], local_enc)

print("精简测试文件已生成！")