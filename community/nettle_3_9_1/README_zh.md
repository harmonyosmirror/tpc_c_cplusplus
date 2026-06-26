# nettle_3_9_1三方库说明
## 功能简介
Nettle 是一个轻量级的密码学库，提供底层密码学原语（哈希、对称加密、公钥加密、MAC 等）。nettle_3_9_1 是 Nettle 3.9.1 的独立 Lycium 包，与 community/nettle（仍为 3.4.1）并存，使用不同的 pkgname 和安装前缀，避免牵动其它依赖旧版 nettle 的三方库。

## 三方库版本
- 3.9.1

## 已适配功能
- 提供哈希算法（SHA-1、SHA-2、SHA-3、MD5 等）。
- 提供对称加密算法（AES、ARCFOUR、Camellia、3DES 等）。
- 提供公钥加密算法（RSA、DSA、ECDSA、EdDSA 等）。
- 提供 MAC 算法（HMAC、CMAC、Poly1305 等）。
- 提供 Hogweed 库（公钥原语高层接口）。
- 支持多架构交叉编译（armeabi-v7a、arm64-v8a）。

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式
+ [应用hap包集成](docs/hap_integrate.md)
