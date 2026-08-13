# stlport如何集成到应用hap

本库是在RK3568开发板上基于OpenHarmony3.2 Release版本的镜像验证的，如果是从未使用过RK3568，可以先查看[润和RK3568开发板标准系统快速上手](https://gitee.com/openharmony-sig/knowledge_demo_temp/tree/master/docs/rk3568_helloworld)。

## 准备应用工程

此三方库原库编译需要依赖gcc-5.4.6版本过低，但OH工具链提供的版本较高，无法编译，故此脚本不进行编译，只需要下载需要移植的源码即可。

### 准备应用开发环境

- [开发环境准备](../../../docs/hap_integrate_environment.md)

### 增加构建脚本及配置文件

本库的HPKBUILD构建脚本仅负责下载源码，不执行编译。详见[HPKBUILD](../HPKBUILD)。

### 准备三方库源码

- 下载本仓库

  ```shell
  git clone https://gitcode.com/CPF-ApplicationTPC/tpc_c_cplusplus.git --depth=1
  ```

- 三方库目录结构

  ```shell
  tpc_c_cplusplus/community/stlport     #三方库stlport的目录结构如下
  ├── docs                              #三方库相关文档的文件夹
  ├── HPKBUILD                          #构建脚本
  ├── SHA512SUM                         #三方库校验文件
  ├── OAT.xml                           #OAT开源审查文本文件
  ├── README.OpenSource                 #说明三方库源码的下载地址，版本，license等信息
  ├── README_zh.md                      #三方库简介
  ```

- 在lycium目录下下载三方库源码

  编译环境的搭建参考[准备三方库构建环境](../../../lycium/README.md#1编译环境准备)

  ```shell
  cd lycium
  ./build.sh stlport
  ```

  > 注：此三方库不进行实际编译，build.sh仅完成源码下载。

## 应用中使用三方库

由于此三方库不进行编译，仅提供源码供移植参考。如需在应用中使用，可参考源码进行手动移植适配。

## 编译工程

此三方库不进行编译，无需编译工程。

## 运行效果

此三方库仅提供源码下载，不进行编译和运行测试，无运行效果截图。

## 参考资料

- [润和RK3568开发板标准系统快速上手](https://gitcode.com/openharmony-sig/knowledge_demo_temp/tree/master/docs/rk3568_helloworld)
- [OpenHarmony三方库地址](https://gitcode.com/cpf-applicationTPC)
- [OpenHarmony知识体系](https://gitcode.com/openharmony-sig/knowledge)
- [通过DevEco Studio开发一个NAPI工程](https://gitcode.com/openharmony-sig/knowledge_demo_temp/blob/master/docs/napi_study/docs/hello_napi.md)