# libcose三方库说明

## 功能简介

libcose 是一个纯 C99 实现的 COSE（CBOR Object Signing and Encryption，RFC 8152）标准库，主要面向资源受限设备（嵌入式、IoT）。其核心设计目标是零动态内存分配，所有缓冲区均由调用方提供。

## 三方库版本

- ea1fed8（master 分支，commit ea1fed87d6ca9b478f8bed323af97e6b192c0a6d）

## 已适配功能

- Ed25519 签名与验签（EdDSA）
- X25519 密钥交换
- ChaCha20-Poly1305 AEAD 认证加密
- HKDF-SHA512 密钥派生
- 支持外部载荷（External Payload）和附加认证数据（AAD）
- 编译时可配置最大签名数、接收方数、头部数和消息大小
- 使用 Monocypher 3.1.3 作为密码学后端（纯 C 实现，以独立动态库方式链接）

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)

## 构建指导

在仓库根目录执行 lycium 构建脚本：

```bash
cd lycium
./build.sh libcose
```

构建依赖 `nanocbor`、`monocypher` 和 `CUnit`。lycium 构建脚本会先解析并构建这些依赖，再构建 `libcose`。


## 使用说明

调用 `libcose` 的密钥生成、签名或加密接口前，需要通过 `cose_crypt_set_rng()` 注册随机数回调。HarmonyOS 设备上建议使用 `getrandom()` 系统调用或 `/dev/urandom` 提供随机源，详细说明见[应用hap包集成](docs/hap_integrate.md#随机数回调注册)。


## 集成方式

- [应用hap包集成](docs/hap_integrate.md)
