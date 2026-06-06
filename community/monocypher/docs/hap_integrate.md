# Monocypher 集成到应用hap

本库是在 OpenHarmony 设备上基于 arm64-v8a 架构完成编译验证的。Monocypher 是一个小型密码学库，设计目标是易于部署。本适配将 Monocypher 编译为 OpenHarmony 独立动态库，供 libcose 等三方库动态链接使用。

## 开发环境

- [开发环境准备](../../../docs/hap_integrate_environment.md)

## 编译三方库

- 下载本仓库

  ```shell
  git clone https://gitcode.com/CPF-ApplicationTPC/tpc_c_cplusplus.git --depth=1
  ```

- 三方库目录结构

  ```shell
  tpc_c_cplusplus/community/monocypher     # 三方库Monocypher的目录结构如下
  ├── docs                                 # 三方库相关文档
  ├── HPKBUILD                             # 构建脚本
  ├── HPKCHECK                             # 测试脚本
  ├── SHA512SUM                            # 三方库校验文件
  ├── README.OpenSource                    # 三方库源码、版本、License等信息
  ├── README.md                            # 英文简介
  └── README_zh.md                         # 中文简介
  ```

- 构建方式

  | 项目 | 说明 |
  |------|------|
  | 构建工具 | lycium custom 模式 |
  | 目标架构 | armeabi-v7a、arm64-v8a |
  | 源文件 | `src/monocypher.c`、`src/optional/monocypher-ed25519.c` |
  | 输出库 | `libmonocypher.so` |

  Monocypher 源码规模较小，本适配直接使用 OHOS NDK clang 编译核心源文件和 Ed25519 可选源文件，并链接为共享库，避免在依赖方工程中重复编译或静态嵌入源码。

- 构建依赖

  | 依赖 | 说明 |
  |------|------|
  | 无外部依赖 | Monocypher 源码自包含 |

- 在 lycium 目录下编译三方库

  编译环境的搭建参考[准备三方库构建环境](../../../lycium/README.md#1编译环境准备)。

  ```shell
  cd lycium
  ./build.sh monocypher
  ```


- [测试三方库](#测试三方库)

## 应用中使用三方库

- 在 IDE 的 `cpp` 目录下新增 `thirdparty` 目录，将编译生成的库和头文件拷贝到该目录下。建议目录结构如下：

  ```shell
  cpp/thirdparty/monocypher/<OHOS_ARCH>/include/monocypher.h
  cpp/thirdparty/monocypher/<OHOS_ARCH>/include/optional/monocypher-ed25519.h
  cpp/thirdparty/monocypher/<OHOS_ARCH>/lib/libmonocypher.so
  ```

- 在最外层（`cpp` 目录下）`CMakeLists.txt` 中添加如下语句：

  ```shell
  target_include_directories(entry PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/monocypher/${OHOS_ARCH}/include
  )

  target_link_libraries(entry PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/monocypher/${OHOS_ARCH}/lib/libmonocypher.so
  )
  ```

- 如果依赖方需要使用 Ed25519 可选接口，需要包含如下头文件：

  ```c
  #include "monocypher.h"
  #include "optional/monocypher-ed25519.h"
  ```

## 测试三方库

Monocypher 当前 `HPKCHECK` 以产物存在性检查为主，[准备三方库测试环境](../../../lycium/README.md#3ci环境准备)。

在 lycium 目录下执行：

```shell
./test.sh monocypher
```

测试脚本会检查以下产物是否存在：

```shell
usr/monocypher/<ARCH>/lib/libmonocypher.so
usr/monocypher/<ARCH>/include/monocypher.h
usr/monocypher/<ARCH>/include/optional/monocypher-ed25519.h
```

也可以手动执行产物检查：

```shell
test -f usr/monocypher/arm64-v8a/lib/libmonocypher.so
test -f usr/monocypher/arm64-v8a/include/monocypher.h
test -f usr/monocypher/arm64-v8a/include/optional/monocypher-ed25519.h
```

## 注意事项

1. Monocypher 作为独立库编译，依赖方应通过 `-I<monocypher include>` 和 `-L<monocypher lib> -lmonocypher` 链接。
2. libcose 使用 Monocypher 的 Ed25519 接口，因此本适配同时编译 `monocypher-ed25519.c`。
3. Monocypher 使用 BSD-2-Clause 许可证，依赖方 README.OpenSource 中如需声明三方依赖，应同步列出该许可证信息。

## 参考资料

- [OpenHarmony三方库地址](https://gitee.com/openharmony-tpc)
- [OpenHarmony知识体系](https://gitee.com/openharmony-sig/knowledge)
- [通过DevEco Studio开发一个NAPI工程](https://gitee.com/openharmony-sig/knowledge_demo_temp/blob/master/docs/napi_study/docs/hello_napi.md)
- [Monocypher upstream](https://github.com/LoupVaillant/Monocypher)
