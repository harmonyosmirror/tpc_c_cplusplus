# libsmbclient集成到应用hap

本库是在RK3568开发板上基于OpenHarmony3.2 Release版本的镜像验证的，**当前已完成 Lycium 交叉编译与 `client/` 闭包打包验证**；HAP 集成及 SMB 连通性**尚需在目标 OpenHarmony 设备上自行验证**。如果是从未使用过RK3568，可以先查看[润和RK3568开发板标准系统快速上手](https://gitee.com/openharmony-sig/knowledge_demo_temp/tree/master/docs/rk3568_helloworld)。

## 开发环境

- [开发环境准备](../../../docs/hap_integrate_environment.md)

## 编译三方库

- 下载本仓库

  ```shell
  git clone https://gitee.com/openharmony-sig/tpc_c_cplusplus.git --depth=1
  ```

- 三方库目录结构

  ```shell
  tpc_c_cplusplus/thirdparty/libsmbclient   # 三方库 libsmbclient 的目录结构如下
  ├── docs                                 # 三方库相关文档
  ├── HPKBUILD                             # 构建脚本
  ├── SHA512SUM                            # 三方库校验文件
  ├── README.OpenSource                    # 说明三方库源码的下载地址、版本、license 等信息
  ├── README_zh.md
  ├── cross-answers-ohos-armv7.txt         # arm32 交叉 configure 答案
  ├── cross-answers-ohos-aarch64.txt       # arm64 交叉 configure 答案
  ├── patches/                             # OHOS 交叉编译补丁
  └── scripts/                             # 打包 client/ 集成目录的脚本
  ```

- 在 lycium 目录下编译三方库

  编译环境的搭建参考[准备三方库构建环境](../../../lycium/README.md#1编译环境准备)。

  构建机还需：`python3`、`perl`（含 `Parse::Yapp` / `yapp`）、`flex`、`pkg-config`，以及本机宿主编译器（`gcc`/`cc`）。

  ```shell
  cd lycium
  ./build.sh libsmbclient
  ```

  `./build.sh` 会按依赖自动先编：`zlib`、`gmp`、`nettle_3_9_1`、`libtasn1_4_19_0`、`gnutls`，无需手动逐个编译。

- 三方库头文件及生成的库

  在 lycium 目录下会生成 `usr` 目录。HAP 集成请使用 **`client/`** 子目录（最小运行时闭包），而非全量 `waf install` 树：

  ```shell
  lycium/usr/libsmbclient/arm64-v8a/client/lib/       # 平铺 .so（约 91 个）
  lycium/usr/libsmbclient/arm64-v8a/client/include/   # libsmbclient.h
  lycium/usr/libsmbclient/armeabi-v7a/client/          # 32 位同理
  ```

  全量安装树 `lycium/usr/libsmbclient/<ARCH>/`（含 `bin/`、`lib/private/` 等）仅作调试参考。

- [测试三方库](#测试三方库)

## 应用中使用三方库

- 在 IDE 的 cpp 目录下新增 `thirdparty` 目录，将对应架构的 **`client/`** 内容拷贝进去，供编译链接使用：

  ```text
  entry/src/main/cpp/thirdparty/libsmbclient/${OHOS_ARCH}/
  ├── lib/          # client/lib/ 下全部 .so（约 91 个）
  └── include/      # client/include/libsmbclient.h
  ```

- 将 **`client/lib/` 下全部 `.so`** 同时拷贝到 HAP 原生库目录，供运行时加载（路径随工程模块名可能略有不同，以下为常见布局）：

  ```text
  entry/src/main/libs/${OHOS_ARCH}/   # 与 libentry.so 等同目录部署全部依赖 .so
  ```

- 在 cpp 目录下的 `CMakeLists.txt` 中添加类似配置（**示例，未在设备上完整验证**）：

  ```cmake
  set(LIBSMBCLIENT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libsmbclient/${OHOS_ARCH})

  target_include_directories(entry PUBLIC ${LIBSMBCLIENT_ROOT}/include)

  target_link_libraries(entry PUBLIC ${LIBSMBCLIENT_ROOT}/lib/libsmbclient.so)
  ```

  说明：

  - `libsmbclient.so` 依赖 Samba 内部库及 gnutls/zlib 等，**91 个 `.so` 须全部打进 HAP**，不能只部署主库。
  - 运行时请保证上述依赖 `.so` 与模块原生库位于动态链接器可搜索的路径（通常即 `libs/${OHOS_ARCH}/`）；若加载失败，用 `hdc shell hilog` 或 `LD_DEBUG=libs` 排查 `NEEDED` 解析。

## 测试三方库

本库当前**已接入 Lycium 自动化测试**。Samba 上游的 `waf test` / torture 等用例面向完整服务器栈，与本配方裁剪后的 **libsmbclient 客户端库**场景不匹配，故不在 CI 中运行。

HPKCHECK 提供以下自动化测试项：

- `smbclient --version` / `--help` 基础命令验证
- `smbcquotas` / `smbget` 工具可用性检查
- `libsmbclient.so` 库文件与符号链接完整性检查
- `smbclient` 参数解析功能验证

完整功能验证可通过 HAP 集成进行，将上文「应用中使用三方库」一节中的 `client/lib/`、`client/include/` 打入 HAP，在应用中调用 `smbc_init`、`smbc_opendir` 等 API 访问 SMB 共享。此路径使用去版本化的 `client/lib/` 闭包，与方式一的库路径不同。

## 参考资料

- [润和RK3568开发板标准系统快速上手](https://gitee.com/openharmony-sig/knowledge_demo_temp/tree/master/docs/rk3568_helloworld)
- [OpenHarmony三方库地址](https://gitee.com/openharmony-tpc)
- [OpenHarmony知识体系](https://gitee.com/openharmony-sig/knowledge)
- [通过DevEco Studio开发一个NAPI工程](https://gitee.com/openharmony-sig/knowledge_demo_temp/blob/master/docs/napi_study/docs/hello_napi.md)
- [Samba 发布说明](https://www.samba.org/samba/download/)
