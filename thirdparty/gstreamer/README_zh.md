# gstreamer三方库说明
## 功能简介
gstreamer是一个流媒体传输框架

## 三方库版本
- v1.26.8

## 已适配功能
- 支持音频播放、音频录制、视频播放
- 音频支持AAC、MPEG(MP3)、Flac、Vorbis、AMR(amrnb、amrwb)、G711mu、APE、G711a、ALAC、AC3、WMA(V1、V2、PRO)、GSM_MS、opus、DTS解码能力
- 视频硬解码支持WMV3、MJPEG、MPEG2、MPEG4、H.263、AVC(H.264)、HEVC(H.265)解码能力
- 硬解码器需要 OpenHarmony API 26 及以上版本

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式
+ [应用hap包集成](docs/hap_integrate.md)