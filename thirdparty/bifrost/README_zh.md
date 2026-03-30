# bifrost 三方库说明

## 功能简介

bifrost是一个专门用于高效、紧凑地处理大量基因组数据的生物信息学工具库。

## 三方库版本

- v1.3.5

## 32位不支持原因

- 此库源码中大量使用 `__uint128_t` 类型，32位系统不支持

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式

- [应用hap包集成](docs/hap_integrate.md)

