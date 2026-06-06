# libcose 集成到应用hap

本库是在 OpenHarmony 设备上基于 arm64-v8a 架构完成编译和运行验证的。libcose 是 COSE（CBOR Object Signing and Encryption，RFC 8152）的 C99 实现，适用于资源受限设备。本适配使用 Monocypher 作为密码学后端，并以独立动态库方式链接。

## 开发环境

- [开发环境准备](../../../docs/hap_integrate_environment.md)

## 编译三方库

- 下载本仓库

  ```shell
  git clone https://gitcode.com/CPF-ApplicationTPC/tpc_c_cplusplus.git --depth=1
  ```

- 三方库目录结构

  ```shell
  tpc_c_cplusplus/thirdparty/libcose     # 三方库libcose的目录结构如下
  ├── docs                               # 三方库相关文档
  ├── HPKBUILD                           # 构建脚本
  ├── HPKCHECK                           # 测试脚本
  ├── README.OpenSource                  # 三方库源码、版本、License等信息
  ├── README.md                          # 英文简介
  ├── README_zh.md                       # 中文简介
  └── libcose_ohos.patch                 # OpenHarmony适配补丁
  ```

- 构建依赖

  | 依赖 | 说明 |
  |------|------|
  | nanocbor | 轻量 CBOR 编解码引擎，提供 `libnanocbor.so` |
  | monocypher | 密码学后端，提供 `libmonocypher.so` |
  | CUnit | 设备侧测试框架，提供 `libcunit.so.1.0.1`（soname `libcunit.so.1`） |

- 在 lycium 目录下编译三方库

  编译环境的搭建参考[准备三方库构建环境](../../../lycium/README.md#1编译环境准备)。

  ```shell
  cd lycium
  ./build.sh libcose
  ```

  执行上述命令时，lycium 会根据 `HPKBUILD` 中的 `depends` 配置先构建 `nanocbor`、`CUnit`、`monocypher`，再构建 `libcose`。


- 三方库头文件及生成的库

  在 lycium 目录下会生成 `usr` 目录，该目录下存在已编译完成的 32 位和 64 位三方库。

  ```shell
  usr/libcose/armeabi-v7a
  usr/libcose/arm64-v8a
  ```

  主要产物如下：

  ```shell
  usr/libcose/<ARCH>/lib/libcose.so
  usr/libcose/<ARCH>/include/
  usr/libcose/<ARCH>/tests/libcose_test
  ```

- [测试三方库](#测试三方库)

## 应用中使用三方库

- 在 IDE 的 `cpp` 目录下新增 `thirdparty` 目录，将编译生成的库和头文件拷贝到该目录下。建议目录结构如下：

  ```shell
  cpp/thirdparty/libcose/<OHOS_ARCH>/include/
  cpp/thirdparty/libcose/<OHOS_ARCH>/lib/libcose.so
  cpp/thirdparty/nanocbor/<OHOS_ARCH>/include/
  cpp/thirdparty/nanocbor/<OHOS_ARCH>/lib/libnanocbor.so
  cpp/thirdparty/monocypher/<OHOS_ARCH>/include/
  cpp/thirdparty/monocypher/<OHOS_ARCH>/lib/libmonocypher.so
  ```

- 在最外层（`cpp` 目录下）`CMakeLists.txt` 中添加如下语句：

  ```shell
  target_include_directories(entry PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libcose/${OHOS_ARCH}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nanocbor/${OHOS_ARCH}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/monocypher/${OHOS_ARCH}/include
  )

  target_link_libraries(entry PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libcose/${OHOS_ARCH}/lib/libcose.so
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nanocbor/${OHOS_ARCH}/lib/libnanocbor.so
      ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/monocypher/${OHOS_ARCH}/lib/libmonocypher.so
  )
  ```

- 使用 `libcose` 时还需要为随机数注入回调，具体方式见[随机数回调注册](#随机数回调注册)。

### 随机数回调注册

`libcose` 不直接绑定具体系统随机源，而是通过 `cose_crypt_set_rng()` 由应用侧注入随机数回调。该回调会在生成密钥、nonce、签名密钥等随机材料时被调用。回调函数返回 `0` 表示成功，返回非 `0` 表示失败。

应用应在调用 `libcose` 的加密、签名或密钥生成接口前完成注册。HarmonyOS 设备上建议优先使用 `getrandom()` 系统调用提供随机源；如果目标环境无法使用 `getrandom()`，也可以从 `/dev/urandom` 读取随机字节后在回调中填充输出缓冲区。不建议在正式业务中使用 `rand()`、固定数组或可预测数据作为随机源。

## 测试三方库

三方库的测试使用原库自带的 CUnit 用例来做测试，[准备三方库测试环境](../../../lycium/README.md#3ci环境准备)。

`libcose` 构建后会生成设备侧测试二进制：

```shell
usr/libcose/<ARCH>/tests/libcose_test
```

> **注意**：`libcose_test` 仅在 CUnit 可用时才会编译。如果该文件不存在，请检查 `usr/CUnit/<ARCH>/lib/` 下是否有 `libcunit.so`——HPKBUILD 以此判断 CUnit 是否可用。若 CUnit 依赖缺失，`libcose` 库本身仍可正常使用，但无法运行设备侧 CUnit 测试。

以 64 位 OpenHarmony 设备为例，可按以下步骤在设备侧验证：

> **注意**：CUnit 的动态库文件名包含版本号（如 `libcunit.so.1.0.1`），该版本号取决于 CUnit 上游发布版本。推送前请先确认实际产物名称：`ls usr/CUnit/arm64-v8a/lib/libcunit.so*`，以下命令以 `1.0.1` 为例，请替换为实际版本号。

```shell
hdc shell 'rm -rf /data/local/tmp/libcose_verify; mkdir -p /data/local/tmp/libcose_verify/lib /data/local/tmp/libcose_verify/tests'

hdc file send usr/libcose/arm64-v8a/tests/libcose_test /data/local/tmp/libcose_verify/tests/libcose_test
hdc file send usr/libcose/arm64-v8a/lib/libcose.so /data/local/tmp/libcose_verify/lib/libcose.so
hdc file send usr/nanocbor/arm64-v8a/lib/libnanocbor.so /data/local/tmp/libcose_verify/lib/libnanocbor.so
hdc file send usr/monocypher/arm64-v8a/lib/libmonocypher.so /data/local/tmp/libcose_verify/lib/libmonocypher.so
hdc file send usr/CUnit/arm64-v8a/lib/libcunit.so.1.0.1 /data/local/tmp/libcose_verify/lib/libcunit.so.1.0.1

hdc shell 'cd /data/local/tmp/libcose_verify/lib && ln -sf libcunit.so.1.0.1 libcunit.so.1 && ln -sf libcunit.so.1.0.1 libcunit.so'
hdc shell 'chmod 755 /data/local/tmp/libcose_verify/tests/libcose_test'
hdc shell 'LD_LIBRARY_PATH=/data/local/tmp/libcose_verify/lib /data/local/tmp/libcose_verify/tests/libcose_test; echo EXIT:$?'
```

测试通过时，CUnit 汇总中应显示：

```text
tests     22     22     22      0        0
asserts  123    123    123      0      n/a
EXIT:0
```

## 适配说明

HPKBUILD 中通过 `libcose_ohos.patch` 修改上游 Makefile，修复两个问题：

1. 上游 `libcose.so` 链接规则遗漏 `$(LDFLAGS)`，导致 Monocypher 动态链接参数无法进入最终 so。
2. 上游使用 `-Wl,$(LIB_NANOCBOR)` 会把宿主机绝对路径写入 `DT_NEEDED`，设备侧无法正确查找 `libnanocbor.so`。适配后改为 `-L$(LIB_NANOCBOR_PATH) -lnanocbor`。

## 注意事项

1. `libcose` 默认使用 Monocypher 后端，支持 Ed25519、X25519、ChaCha20-Poly1305 和 HKDF-SHA512。
2. Monocypher 后端不支持 ECDSA P-256/P-384/P-521、AES-GCM/CCM 等 Mbed TLS 后端能力。
3. 默认 `COSE_MSGSIZE_MAX=512` 字节，超大载荷场景需通过编译宏调整。
4. `libcose` 上游长期未发布新 tag，当前使用 master 分支 commit `ea1fed8`。

## 参考资料

- [OpenHarmony三方库地址](https://gitee.com/openharmony-tpc)
- [OpenHarmony知识体系](https://gitee.com/openharmony-sig/knowledge)
- [通过DevEco Studio开发一个NAPI工程](https://gitee.com/openharmony-sig/knowledge_demo_temp/blob/master/docs/napi_study/docs/hello_napi.md)
- [libcose upstream](https://github.com/bergzand/libcose)
