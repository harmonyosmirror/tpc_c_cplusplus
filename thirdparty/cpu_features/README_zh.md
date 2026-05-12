# cpu_features 三方库说明
## 功能简介
cpu_features是一个跨平台的C库，主要用于在运行时检测当前设备的CPU类型及其支持的指令集特性。

## 三方库版本
- v0.9.0

## 已适配功能
- arm64-v8a  能获取到 实现者的id FP,ASIMD,AES,SHA1,SHA2,CRC32,ATOMICS,SVE等特性支持信息
- armeabi-v7a 能获取到 实现者的id  NEON AES CRC32 SHA1 SHA2等特性的支持信息
- x86_64 能获取到 处理器制造商  AVX2 AES SSE4.2 FMA3 BMI2 AVX512F SHA POPCN等特性支持信息

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

+ [应用hap包集成](docs/hap_integrate.md)
