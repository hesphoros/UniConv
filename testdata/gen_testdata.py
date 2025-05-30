# gen_testdata.py
import codecs

text = "你好，世界！编码测试。\nHello, world! Encoding test.\n"

# UTF-8
with open("input_utf8.txt", "w", encoding="utf-8") as f:
    f.write(text)

# GBK
with open("input_gbk.txt", "w", encoding="gbk") as f:
    f.write(text)

# local（假如你的系统本地编码为GB2312/GBK，和GBK一致，否则可用locale.getpreferredencoding()）
with open("input_local.txt", "w", encoding="gbk") as f:
    f.write(text)

# UTF-16LE 无BOM
with codecs.open("input_utf16le.txt", "w", encoding="utf-16le") as f:
    f.write(text)

# UTF-16BE 无BOM
with codecs.open("input_utf16be.txt", "w", encoding="utf-16be") as f:
    f.write(text)

print("所有标准测试文件已生成！")