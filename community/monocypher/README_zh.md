# Monocypher三方库说明

## 功能简介

Monocypher 是一个小型密码学库，设计目标是易于部署。本适配将 Monocypher 作为 OpenHarmony 独立动态库编译，供依赖库动态链接使用。

## 三方库版本

- 3.1.3

## 已适配功能

- Monocypher 核心密码学能力
- libcose 需要的可选 Ed25519 支持
- 动态库产物：libmonocypher.so

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)

## 构建指导

在仓库根目录执行 lycium 构建脚本：

```bash
cd lycium
./build.sh monocypher
```

编译成功后，产物位于：

```text
lycium/usr/monocypher/<ARCH>/lib/libmonocypher.so
lycium/usr/monocypher/<ARCH>/include/monocypher.h
lycium/usr/monocypher/<ARCH>/include/optional/monocypher-ed25519.h
```

## 集成方式

- [应用hap包集成](docs/hap_integrate.md)
