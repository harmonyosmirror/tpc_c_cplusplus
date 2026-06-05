# NanoCBOR 鸿蒙适配说明

## 基本信息

| 字段 | 内容 |
|------|------|
| 库名称 | NanoCBOR |
| 版本 | 0623c45（master 分支，commit 0623c45686f6bc296c35416f3a41a71b148122d7） |
| 许可证 | CC0-1.0 |
| 上游地址 | https://github.com/bergzand/NanoCBOR |
| 源码包 | https://github.com/bergzand/NanoCBOR/archive/0623c45686f6bc296c35416f3a41a71b148122d7.tar.gz |

## 功能概述

NanoCBOR 是一个面向资源受限设备（嵌入式、IoT）的极小体积 CBOR（Concise Binary Object Representation，RFC 7049）编解码库，针对 32 位架构优化，同时兼容 8 位和 16 位平台。其解码器在 Cortex-M0+ 上仅需 600-800 字节 Flash 空间。NanoCBOR 也是 libcose 的必选依赖。

主要功能：
- CBOR 数据解码（整数、字节串、文本串、数组、映射、标签、简单值、浮点数）
- CBOR 数据编码（整数、字节串、文本串、数组、映射、标签、简单值、浮点数）
- 嵌套容器（数组/映射）的递进式解码
- 零动态内存分配，所有缓冲区由调用方提供
- 仅依赖字节序转换函数

## 迁移说明

### 构建方式

- 构建工具：Meson/Ninja（上游原生）→ 鸿蒙化编译使用 lycium custom 模式，直接使用 OHOS NDK clang 编译源文件
- 目标架构：armeabi-v7a、arm64-v8a

> **构建方式选择原因**：NanoCBOR 上游使用 Meson 构建系统，未提供 CMakeLists.txt。鸿蒙化编译直接使用 NDK clang 编译 `decoder.c` 和 `encoder.c` 两个源文件并链接为共享库，无需 Meson 依赖。

### 依赖库

| 依赖 | 说明 |
|------|------|
| 无外部依赖 | 仅需字节序转换函数（可使用编译器内置 `__builtin_bswap32`/`__builtin_bswap64`） |

### 补丁说明

**nanocbor_ohos.patch**：修复 `_skip_limited()` 函数中嵌套容器跳过时 `remaining` 计数器多次递减的问题

- **原因**：上游 `_skip_limited()` 在跳过嵌套容器（数组/映射）内容时，直接在当前解码器上调用 `_advance()`，导致内层元素的跳过操作会反复递减 `it->remaining`。从调用者视角，整个嵌套容器应只算一个元素，但实际被计为多个，造成后续解码提前到达末尾。
- **修复方案**：为嵌套容器创建子解码器（`sub`），在内层独立解码跳过，外层 `it->remaining` 仅递减一次，确保计数语义正确。

### 编译命令

```bash
cd lycium
./build.sh nanocbor
```

### 产物位置

编译成功后，产物位于：

```
lycium/usr/nanocbor/<ARCH>/lib/libnanocbor.so
lycium/usr/nanocbor/<ARCH>/include/nanocbor/
```

产物动态依赖：
```
NEEDED: libc.so
```

### 头文件

| 头文件 | 说明 |
|--------|------|
| `nanocbor/nanocbor.h` | 主头文件，包含编解码全部 API 声明 |

## 注意事项

1. **字节序转换**：NanoCBOR 需要字节序转换函数。在鸿蒙设备上可使用编译器内置函数 `__builtin_bswap32` 和 `__builtin_bswap64`，无需额外配置。
2. **浮点解码**：解码器支持半精度（float16）、单精度（float32）和双精度（float64）浮点数，启用浮点解码会增加约 200 字节 Flash 占用。
3. **版本说明**：NanoCBOR 上游未发布正式版本号，当前使用 master 分支 commit 0623c45（2025-11-14）。

