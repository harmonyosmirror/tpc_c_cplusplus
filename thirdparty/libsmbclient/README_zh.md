# libsmbclient三方库说明
## 功能简介
libsmbclient 是 Samba 提供的 SMB/CIFS 客户端共享库（C API），用于在应用中访问 Windows 或 Samba 文件共享。提供目录列举、文件读写、属性查询、扩展属性操作等功能，支持 SMBv1（NT1）至 SMBv3.11 协议。

## 三方库版本
- 4.24.2

## 已适配功能
- 支持 SMB/CIFS 目录浏览（工作组、服务器、共享、目录、文件）。
- 支持文件打开、读写、定位、关闭操作。
- 支持文件/目录属性查询（stat、fstat、statvfs）。
- 支持扩展属性读写（DOS 属性、安全描述符）。
- 支持 Kerberos/GSSAPI 认证和 NTLMv2 认证。
- 支持目录变更通知（smbc_notify）。
- 支持多架构交叉编译（armeabi-v7a、arm64-v8a）。

## 使用约束
- [IDE和SDK版本](../../docs/constraint.md)

## 集成方式
+ [应用hap包集成](docs/hap_integrate.md)
