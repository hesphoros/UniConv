#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成 UniConv 测试所需的各种编码格式文件（含/不含 BOM）
"""

import os

os.makedirs("testdata", exist_ok=True)
os.makedirs("testdata/output", exist_ok=True)

text = "测试文本 Hello World 123"

# (文件名, 编码, BOM)
encodings = [
    ("input_utf8.txt",       "utf-8",      b""),
    ("input_utf8_bom.txt",   "utf-8",      b"\xEF\xBB\xBF"),
    ("input_gbk.txt",        "gbk",        b""),
    ("input_gb2312.txt",     "gb2312",     b""),
    ("input_utf16le.txt",    "utf-16le",   b"\xFF\xFE"),
    ("input_utf16le_nobom.txt", "utf-16le", b""),          # 新增无 BOM
    ("input_utf16be.txt",    "utf-16be",   b"\xFE\xFF"),
    ("input_utf16be_nobom.txt", "utf-16be", b""),          # 新增无 BOM
]

for fname, encoding, bom in encodings:
    try:
        with open(f"testdata/{fname}", "wb") as f:
            if bom:
                f.write(bom)
            f.write(text.encode(encoding))
        print(f"生成 {fname}")
    except Exception as e:
        print(f"❌ 生成 {fname} 时失败: {e}")

print("✅ 所有测试文件已生成！")

# 可选：验证文件内容
print("\n🔍 文件验证:")
for fname, encoding, bom in encodings:
    filepath = f"testdata/{fname}"
    if os.path.exists(filepath):
        with open(filepath, 'rb') as f:
            data = f.read()
        print(f"  {fname}: {len(data)} 字节，前12字节: {' '.join(f'{b:02x}' for b in data[:12])}")
    else:
        print(f"  {fname}: ❌ 文件不存在")