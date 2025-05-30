#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# 生成正确的UTF-8测试文件
text = "测试文本Hello World 123"

# 写入UTF-8文件（无BOM）
with open('input_utf8.txt', 'w', encoding='utf-8') as f:
    f.write(text)

print("UTF-8文件已生成")

# 验证文件内容
with open('input_utf8.txt', 'rb') as f:
    data = f.read()
    print(f"文件大小: {len(data)} 字节")
    print(f"十六进制: {' '.join(f'{b:02x}' for b in data)}")
