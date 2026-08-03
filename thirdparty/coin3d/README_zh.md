# coin3d三方库说明

## 功能简介
Coin 是一个基于 OpenGL 的高层级 3D 图形库，提供场景图（scene graph）功能，是 Open Inventor 的开源实现，用于 3D 场景渲染。该库为 OpenHarmony 平台进行了适配，支持 EGL 后端渲染及文字渲染功能。

## 三方库版本
- v4.0.8

## 已适配功能
- 基于 OpenGL/EGL 的 3D 场景图渲染
- Open Inventor API 兼容
- 文字渲染（SoText3），依赖 freetype2_coin3d 和 glu
- 支持 armeabi-v7a、arm64-v8a、x86_64 三架构

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式
+ [应用Hap包集成](docs/hap_integrate.md)
