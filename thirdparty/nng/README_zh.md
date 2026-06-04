# nng三方库说明
## 功能简介
nng（nanomsg-next-generation）是一个轻量级消息库，提供了简单的高级API用于创建分布式应用程序。它是nanomsg库的重写版本，具有更好的可靠性和性能，支持多种通信模式（如请求/回复、发布/订阅、推送/拉取等），适用于嵌入式、桌面和服务器等多种场景。

## 三方库版本
- 1.11

## 已适配功能
- 提供多种通信模式的轻量级消息传递功能
- 支持TCP、IPC等多种传输协议
- 支持Zero Copy等高性能特性

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式
+ [应用hap包集成](docs/hap_integrate.md)