# Native SO 模块加载 Crash 问题 FAQ

## Q：运行时报错 `Cannot read property 'XXX' of undefined`，应用发生 crash，该如何定位与修复？

**A：** 报错形如 `Cannot read property 'XXX' of undefined`，其中 `XXX` 为 native 导出到 ArkTS 的某个函数名（示例中用 `XXX` 代指，实际可以是任意导出函数）。该问题的根因、定位方法与修复建议如下。

---

## 前置知识（零基础读者请先阅读）

> 以下概念贯穿全文，建议先理解再阅读后续内容。

| 术语 | 通俗解释 |
|:-----|---------|
| **ArkTS** | HarmonyOS 应用层的开发语言（类似 TypeScript），应用的业务逻辑用它编写 |
| **native 模块** | 用 C/C++ 编写的原生代码，编译后生成 `.so` 文件，供 ArkTS 调用。两者之间的"桥梁"是 **NAPI**（Node-API），native 模块通过 NAPI 将函数导出给 ArkTS 使用 |
| **`.so` 文件** | Linux/类 Unix 系统（含 HarmonyOS）下的动态链接库文件，类似于 Windows 的 `.dll`。一个 so 可以依赖其他 so，就像拼图一样需要所有碎片齐全才能拼好 |
| **HAP 包** | Harmony Ability Package，HarmonyOS 应用的安装包格式。应用最终打成一个 `.hap` 文件，其中 `libs` 目录存放所有 `.so` 库 |
| **`libs` 目录** | HAP 包内存放 native 库的文件夹，路径形如 `libs/{架构}/`（如 `libs/arm64-v8a/`） |
| **soname** | "shared object name"的缩写，即"共享对象名称"。每个 so 库内部可嵌入一个 soname 标识。**运行时系统是按 soname 来查找依赖库的**，而不是按文件名。这是本文档最核心的概念 |
| **CMake** | 一个常用的 C/C++ 构建工具，用于编译 native 模块、生成 `.so` 文件 |
| **llvm-readelf** | LLVM 工具链中的一个命令行工具，用于读取 ELF 文件（即 `.so`）的信息，包括它的依赖库列表和 soname |

---

### a. Crash 产生的原因

> **一句话总结：** native 模块没加载起来 → 导出的函数变成了 `undefined` → ArkTS 调用时找不到函数 → 应用 crash。

按以下因果链理解：

**第一步：报错的直接原因**

- 运行时报错 `Cannot read property 'XXX' of undefined`，表示 native 导出到 ArkTS 的函数 `XXX` 为 `undefined`（未定义）。
- 这意味着 **native 模块根本没有成功加载**，导致它本应导出的函数全部不存在，ArkTS 调用时即触发 crash。

**第二步：为什么 native 模块会加载失败？**

- native 模块在正常情况下是可以加载成功的。
- 但一个 `.so` 库能正常加载的**前提条件**是：**它所依赖的所有 `.so` 库都已成功加载**（可以理解为"牵一发而动全身"）。
- 只要其中任意一个依赖库缺失或加载失败，该 native 模块也会随之加载失败。

**第三步：为什么依赖库会找不到？—— soname 不一致**

这是最常见的根因，核心在于 **soname** 这个概念：

1. **CMake 编译时**：链接的 so 库名通常**不带版本号**，例如 `libfoo.so`。
2. **so 加载运行时**：系统实际查找的库名是 **`soname`** 指定的名称，例如 `libfoo.so.1`（带版本号后缀）。
3. **问题所在**：如果打包进 HAP 包的 so 库**文件名不是按 soname 命名的**（即编译产物未携带正确的版本号后缀），运行时系统按 soname 去查找时就会**找不到对应库**，从而导致 native 模块加载失败，最终触发上述 crash。

> 📌 **举例说明：** 假设库 `libfoo` 的 soname 是 `libfoo.so.1`。如果 CMake 编译产物文件名为 `libfoo.so`（无版本号），直接打入 HAP 包，运行时系统按 `libfoo.so.1` 去查找，但包内只有 `libfoo.so` → **找不到 → 加载失败 → crash**。

---

### b. 问题定位方法

> **一句话总结：** 查依赖 → 看 HAP 包里有什么 → 对比差异 → 重点查 soname。

> **🛠️ 工具准备：配置 `llvm-readelf` 环境变量**
>
> `llvm-readelf` 不是系统自带的命令，需要手动将 LLVM 工具链路径添加到系统环境变量后才能在命令行直接使用。配置步骤如下：
>
> 1. **找到工具路径**：LLVM 工具随 DevEco Studio（HarmonyOS 官方 IDE）安装时附带，位于 DevEco Studio 安装目录下的 `sdk\default\openharmony\native\llvm\bin` 文件夹中。例如：
>    - 默认安装路径示例：`D:\DevEco Studio\sdk\default\openharmony\native\llvm\bin`
>    - 该目录下包含 `llvm-readelf.exe`（Windows）。如果安装时修改了路径，请在自定义路径下查找。
> 2. **添加到环境变量**：将上述 `llvm/bin` 的完整路径添加到系统的 `PATH` 环境变量中。
>    - **Windows**：`系统设置` → 搜索"环境变量" → 编辑 `Path` → 新建 → 粘贴 `llvm\bin` 的完整路径 → 确定。
>    - **Linux/Mac**：在 `~/.bashrc` 或 `~/.zshrc` 中添加 `export PATH=$PATH:/path/to/llvm/bin`，然后执行 `source` 使其生效。
> 3. **验证是否生效**：打开一个新的命令行窗口，输入 `llvm-readelf --version`，若正常输出版本号，说明配置成功，可以开始后续步骤。
>
> > 💡 如果不想配置环境变量，也可以在执行命令时直接使用工具的完整路径，例如：`"D:\DevEco Studio\sdk\default\openharmony\native\llvm\bin\llvm-readelf.exe" -d libnative.so`。

该场景下，建议按以下步骤排查 native so（即 native 工程编译产出的 so）的依赖完整性：

| 步骤 | 操作 | 命令/方式 | 说明 |
|:---:|------|---------|------|
| 1 | 查询 native so 的所有依赖 | `llvm-readelf.exe -d libnative.so` | 输出中的 `NEEDED` 字段列出了该 so 运行时所需依赖的全部库 |
| 2 | 解压 HAP 包，查看 `libs` 目录下的所有 so 库 | 将 `.hap` 后缀改为 `.zip` 后解压，或用解压工具打开 | 确认实际打包进去的 so 库有哪些 |
| 3 | 对比解压出的 so 库文件名与 native so 依赖的库名 | 人工对比步骤 1 和步骤 2 的结果 | 找出名称不一致（缺失或版本号不匹配）的库 |
| 4 | 对名称不一致的库，查询其 so 信息 | `llvm-readelf -d libxxx.so` | 重点看输出中的 `SONAME` 字段，确认该库的 soname 到底是什么 |
| 5 | **重点确认：库的文件名是否与其 `soname` 一致** | 对比文件名与 `SONAME` 字段值 | soname 是运行时实际查找的库名，**文件名与 soname 不一致即为问题根因** |

> 📌 **关键提示：** 编译产物文件名 ≠ 运行时查找的 soname。若发现 HAP 包内的 so 文件名与其 soname 不一致，即为问题根因，请进入下方"c. 问题修复建议"进行处理。

---

### c. 问题修复建议

> **一句话总结：** 让 HAP 包里每个 so 的文件名都和它的 soname 对上，再把缺的依赖补齐。

1. **确保打包产物文件名与 soname 一致**
   - 在打包 HAP 前，确认 `libs` 目录下的每个 so 库**文件名均与其 `soname` 完全一致**（含版本号后缀）。
   - 若文件名与 soname 不一致，需在打包阶段将文件重命名为 soname 对应的名称，再打入 HAP 包。
   - 例如：so 的 soname 为 `libfoo.so.1`，则 HAP 包内的文件名应为 `libfoo.so.1`，而非 `libfoo.so`。

2. **核对依赖完整性**
   - 使用 `llvm-readelf -d` 逐个确认 native so 及其依赖库的 `NEEDED` 列表，确保所有依赖库均已正确打包到 HAP 的 `libs` 目录中，避免遗漏。

3. **回归验证**
   - 修复后重新打包 HAP 并运行应用，确认：
     - native 模块加载成功；
     - 导出函数可被 ArkTS 正常调用；
     - crash 不再复现。
