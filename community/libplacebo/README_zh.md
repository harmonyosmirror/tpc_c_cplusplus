# libplacebo 三方库说明

## 功能简介

libplacebo 是 mpv 使用的 GPU 视频/图像渲染库，提供色彩管理、缩放、去交错、色调映射等着色器能力。本适配在 OpenHarmony 上启用 OpenGL 后端，产出静态库 `libplacebo.a`。

## 三方库版本

- v7.360.0

## 已适配功能

- OpenGL 后端静态库交叉编译（armeabi-v7a / arm64-v8a / x86_64）
- 关闭 D3D11、demos、tests 等非必要组件

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)
