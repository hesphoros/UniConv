#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成 UniConv 测试所需的各种编码格式文件
"""

import os

# 确保目录存在
os.makedirs("testdata", exist_ok=True)
os.makedirs("testdata/output", exist_ok=True)

# 测试文本
text = "测试文本Hello World 123"

# 1. UTF-8 文件（无 BOM）
with open("testdata/input_utf8.txt", "wb") as f:
    f.write(text.encode('utf-8'))

# 2. GBK 文件
with open("testdata/input_gbk.txt", "wb") as f:
    f.write(text.encode('gbk'))

# 3. UTF-16LE 文件（有 BOM）
with open("testdata/input_utf16le.txt", "wb") as f:
    f.write(b'\xFF\xFE')  # UTF-16LE BOM
    f.write(text.encode('utf-16le'))

# 4. UTF-16BE 文件（有 BOM）
with open("testdata/input_utf16be.txt", "wb") as f:
    f.write(b'\xFE\xFF')  # UTF-16BE BOM
    f.write(text.encode('utf-16be'))

# 5. 本地编码文件（GB2312，与GBK兼容）
with open("testdata/input_local.txt", "wb") as f:
    f.write(text.encode('gbk'))

print("✅ 所有测试文件已生成成功！")
print(f"📝 测试文本: {text}")

# 验证生成的文件
print("\n🔍 文件验证:")
for filename, encoding in [
    ("input_utf8.txt", "utf-8"),
    ("input_gbk.txt", "gbk"),
    ("input_utf16le.txt", "utf-16"),
    ("input_utf16be.txt", "utf-16"),
    ("input_local.txt", "gbk")
]:
    filepath = f"testdata/{filename}"
    if os.path.exists(filepath):
        with open(filepath, 'rb') as f:
            data = f.read()
        print(f"  {filename}: {len(data)} 字节")
        print(f"    前10字节: {' '.join(f'{b:02x}' for b in data[:10])}")
    else:
        print(f"  {filename}: ❌ 文件不存在")
