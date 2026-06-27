# gnutls三方库说明
## 功能简介
GnuTLS 是一个安全的通信库，实现了 SSL/TLS 协议及相关密码学功能。本包为 GnuTLS 3.8.4，裁剪了 p11-kit/DANE 等依赖，供 OHOS 交叉编译使用。依赖 nettle_3_9_1、libtasn1_4_19_0、gmp、zlib。

## 三方库版本
- 3.8.4

## 已适配功能
- 支持 TLS 1.2 / TLS 1.3 协议。
- 支持证书解析与验证（X.509）。
- 支持对称/非对称加密（AES、RSA、ECDSA、EdDSA 等）。
- 支持安全 renegotiation 和 session resumption。
- 支持多架构交叉编译（armeabi-v7a、arm64-v8a）。

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)
- 本构建禁用了默认信任存储（--with-default-trust-store-file=no），使用方需在运行时通过 gnutls_certificate_set_x509_trust_file() 等接口显式配置 CA 证书路径。
