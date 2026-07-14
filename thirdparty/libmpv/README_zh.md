# libmpv三方库说明

## 功能简介

libmpv 是 mpv 媒体播放器的客户端库，提供 C API 供应用集成音视频播放能力。本仓库将 mpv v0.41.0 与 OpenHarmony 适配补丁集成进 Lycium/TPC 构建体系，产出 **libmpv.so** 及 libmpv client 头文件。

## 三方库版本

- v0.41.0

## 已适配功能

- libmpv 共享库构建（`-Dlibmpv=true`）
- OHOS 硬解嵌入（`-Dohos-avcodec-embed=true`）
- OHOS EGL 渲染（`-Degl-ohos=enabled`）
- Native Image 输出（`-Dohos-native-image=enabled`）
- OHOS 音频输出（`-Dohaudio=true`）
- Lua 脚本支持（`-Dlua=enabled`）
- Vulkan 支持（`-Dvulkan=enabled`）
- 单元测试（`-Dtests=true`，供 HPKCHECK 在设备上运行）
- 链接策略：`libass`、`libplacebo` 等依赖静态链入 `libmpv.so`；`DT_NEEDED` 中仅保留 FFmpeg 相关动态库

## 使用约束

- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式

- [应用hap包集成](docs/hap_integrate.md)
