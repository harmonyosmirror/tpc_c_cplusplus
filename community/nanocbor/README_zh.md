# NanoCBOR三方库说明

## 功能简介

NanoCBOR 是一个面向资源受限设备（嵌入式、IoT）的极小体积 CBOR（Concise Binary Object Representation，RFC 7049）编解码库，针对 32 位架构优化，同时兼容 8 位和 16 位平台。其解码器在 Cortex-M0+ 上仅需 600-800 字节 Flash 空间。NanoCBOR 也是 libcose 的必选依赖。

## 三方库版本

- 0623c45（master 分支，commit 0623c45686f6bc296c35416f3a41a71b148122d7）

## 已适配功能

- CBOR 数据解码（整数、字节串、文本串、数组、映射、标签、简单值、浮点数）
- CBOR 数据编码（整数、字节串、文本串、数组、映射、标签、简单值、浮点数）
- 嵌套容器（数组/映射）的递进式解码
- 零动态内存分配，所有缓冲区由调用方提供
- 仅依赖字节序转换函数（可使用 `__builtin_bswap32`/`__builtin_bswap64`）

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式

- [应用hap包集成](docs/hap_integrate.md)
