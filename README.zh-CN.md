# c-escape-lab

[English](README.md) | 简体中文

> 文档说明：本文档由 AI 协助起草，并已根据当前头文件实现完成核对。

一个以 C 预处理器宏为核心的终端控制序列库。当前实现集中在
[`escape.h`](escape.h)：它把 ANSI/VT 风格的控制序列拼接成字符串字面量，便于
直接嵌入 `printf`、`fputs` 或其他输出接口。

该接口采用单头文件形式，直接包含 `escape.h` 即可使用。

## 目录

- [c-escape-lab](#c-escape-lab)
  - [目录](#目录)
  - [项目定位](#项目定位)
    - [当前提供的能力](#当前提供的能力)
    - [当前不提供的能力](#当前不提供的能力)
  - [快速开始](#快速开始)
  - [核心展开模型](#核心展开模型)
    - [两种输入约定](#两种输入约定)
    - [一条序列如何生成](#一条序列如何生成)
  - [API 总览](#api-总览)
    - [命名空间](#命名空间)
    - [序列分类](#序列分类)
  - [CSI 常用接口](#csi-常用接口)
    - [光标移动与位置](#光标移动与位置)
    - [擦除、编辑与滚动](#擦除编辑与滚动)
    - [模式设置与查询](#模式设置与查询)
    - [设备状态查询](#设备状态查询)
  - [颜色接口](#颜色接口)
    - [SGR 基础样式和颜色](#sgr-基础样式和颜色)
    - [256 色和真彩色](#256-色和真彩色)
    - [文本着色](#文本着色)
  - [基础序列构造器](#基础序列构造器)
  - [通用工具宏](#通用工具宏)
  - [兼容性与限制](#兼容性与限制)
  - [验证与项目状态](#验证与项目状态)
    - [当前建议的验证方式](#当前建议的验证方式)
    - [文件说明](#文件说明)
  - [参考资料](#参考资料)

## 项目定位

### 当前提供的能力

- 单头文件接口，不需要链接额外的库。
- 使用 `ESCAPE_*` 工具宏拼接、字符串化和展开可变参数。
- 使用 `EST_*`（Escape Sequence Type）命名空间描述控制序列类型和参数。
- 覆盖一批常用 CSI 序列：光标移动、擦除编辑、SGR 样式、模式开关、设备查询、
  滚动区域、制表位和部分终端扩展。
- 提供 ANSI 16 色、256 色索引、6×6×6 色立方体和灰度阶梯相关常量/换算宏。
- 提供 ESC、CSI、OSC、DCS、APC、ST、SOS、PM 的底层字符串构造器。

### 当前不提供的能力

- 不负责向终端写入、刷新或读取响应；生成字符串后仍需由调用方处理 I/O。
- 不检测终端能力，也不自动回退 256 色、真彩色或终端扩展。
- 不解析 DSR、CPR、DA、DECRQM 等查询响应。
- 不保证所有终端都支持每一条扩展序列。

## 快速开始

以下程序只依赖 `escape.h`，会输出橙色文字并恢复默认样式：

```c
#include <stdio.h>
#include "escape.h"

int main(void)
{
    printf(
        EST_CSI_SGR_FC_EXT_TRUE_SSTR(255, 80, 20)
        "orange text"
        EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
        "\n"
    );
    return 0;
}
```

在支持 GCC 或 Clang 的环境中，例如：

```sh
cc -std=gnu11 -Wall -Wextra -pedantic demo.c -o demo
./demo
```

Windows 下可以使用 MinGW-w64 的 `gcc`；PowerShell 中的命令形式相同。终端不支持
颜色时，会按照自身行为处理控制序列；本库不会过滤或模拟不支持的控制序列。

## 核心展开模型

### 两种输入约定

这是使用本项目时最重要的区别：

- `*_STR` 接收已经是字符串片段的内容，例如 `"5"`、`"1;10"`。
- `*_SSTR` 使用 `ESCAPE_S(...)` 把预处理器 token 转成字符串，例如 `5`、
  `EST_CSI_SGR_RESET`。

```c
/* 两者生成相同的字符串：ESC [ 5 A */
EST_CSI_CUU_STR("5")
EST_CSI_CUU_SSTR(5)

/* 两者生成相同的字符串：ESC [ 0 m */
EST_CSI_SGR_STR("0")
EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
```

`*_STR` 宏的参数不是运行时变量，不能把 `int count` 直接传给
`EST_CSI_CUU_STR`。需要运行时参数时，应使用 `snprintf` 或自行构造字符串；本
项目当前只做编译期字符串拼接。

### 一条序列如何生成

CSI 的通用形式是：

```text
ESC [ P* I* F
```

其中 `P` 是参数字节，`I` 是中间字节，`F` 是最终字节。例如：

```c
EST_CSI_CUP_SSTR(10, 20)        /* "\x1b[10;20H" */
EST_CSI_ED_SSTR(2)              /* "\x1b[2J" */
EST_CSI_DECSCUSR_STR("2")       /* "\x1b[2 q" */
```

宏展开结果是相邻字符串字面量，可以直接与普通字符串拼接：

```c
fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "bold" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
```

## API 总览

### 命名空间

| 前缀 | 用途 |
| --- | --- |
| `ESCAPE_` | 预处理器工具、断言、TODO 处理和数值工具 |
| `EST_` | 控制序列类型、参数、最终字节和序列构造器 |
| `EST_CSI_` | CSI 序列及其参数 |
| `ESCAPE_INTRODUCER_` | 各类控制序列引导符 |

### 序列分类

头文件定义了 `enum ESCAPE_SEQUENCE_TYPE`，并为以下类型预留了编号区间：

| 类型 | 引导/终止形式 | 当前用途 |
| --- | --- | --- |
| ESC | `ESC` | 单字符 ESC 序列 |
| CSI | `ESC [` | 光标、样式、模式和查询等 |
| OSC | `ESC ] ... ST` | 操作系统命令的底层拼接 |
| DCS | `ESC P ... ST` | 设备控制字符串的底层拼接 |
| APC | `ESC _ ... ST` | 应用程序命令的底层拼接 |
| ST | `ESC \\` | 字符串终止符 |
| SOS / PM | `ESC X` / `ESC ^` | 底层字符串形式 |

## CSI 常用接口

以下宏都生成字符串字面量；带 `_DEFAULT_STR` 的宏显式填入协议默认参数。

### 光标移动与位置

| 宏 | 生成形式 | 说明 |
| --- | --- | --- |
| `EST_CSI_CUU_STR/Ps` | `CSI Ps A` | 上移 |
| `EST_CSI_CUD_STR/Ps` | `CSI Ps B` | 下移 |
| `EST_CSI_CUF_STR/Ps` | `CSI Ps C` | 右移 |
| `EST_CSI_CUB_STR/Ps` | `CSI Ps D` | 左移 |
| `EST_CSI_CNL_STR/Ps` | `CSI Ps E` | 下移并到行首 |
| `EST_CSI_CPL_STR/Ps` | `CSI Ps F` | 上移并到行首 |
| `EST_CSI_CHA_STR/Ps` | `CSI Ps G` | 设置绝对列 |
| `EST_CSI_CUP_STR/Pr,Pc` | `CSI Pr;Pc H` | 设置行、列 |
| `EST_CSI_HVP_STR/Pr,Pc` | `CSI Pr;Pc f` | 水平垂直定位 |
| `EST_CSI_VPA_STR/Ps` | `CSI Ps d` | 设置绝对行 |
| `EST_CSI_CHT_STR/Ps` / `EST_CSI_CBT_STR/Ps` | `CSI Ps I/Z` | 前/后移制表位 |
| `EST_CSI_SCP_STR()` / `EST_CSI_RCP_STR()` | `CSI s/u` | 保存/恢复光标 |
| `EST_CSI_DECSC_STR()` / `EST_CSI_DECRC_STR()` | `ESC 7/8` | DEC 保存/恢复光标 |

对应的 `_SSTR` 版本已为主要单参数和双参数接口提供，例如
`EST_CSI_CUP_SSTR(10, 20)`。

### 擦除、编辑与滚动

| 宏 | 生成形式 | 常用参数 |
| --- | --- | --- |
| `EST_CSI_ED_STR/Ps` | `CSI Ps J` | `0` 到末尾，`1` 到开头，`2` 全屏，`3` 清滚动区 |
| `EST_CSI_EL_STR/Ps` | `CSI Ps K` | `0` 到行尾，`1` 到行首，`2` 整行 |
| `EST_CSI_ICH_STR/Ps` | `CSI Ps @` | 插入空白字符 |
| `EST_CSI_DCH_STR/Ps` | `CSI Ps P` | 删除字符 |
| `EST_CSI_ECH_STR/Ps` | `CSI Ps X` | 擦除字符但不移动光标 |
| `EST_CSI_IL_STR/Ps` / `EST_CSI_DL_STR/Ps` | `CSI Ps L/M` | 插入/删除行 |
| `EST_CSI_SU_STR/Ps` / `EST_CSI_SD_STR/Ps` | `CSI Ps S/T` | 向上/向下滚动 |
| `EST_CSI_REP_STR/Ps` | `CSI Ps b` | 重复前一个字符 |
| `EST_CSI_TBC_STR/Ps` | `CSI Ps g` | 清除制表位，常用 `0` 或 `3` |
| `EST_CSI_DECSTBM_STR/Pt,Pb` | `CSI Pt;Pb r` | 设置上下滚动边界 |
| `EST_CSI_DECSLRM_STR/Pl,Pr` | `CSI Pl;Pr s` | 设置左右边界，需先启用 `?69` |

### 模式设置与查询

```c
EST_CSI_SM_DEC_PRIVATE_MODE_STR("25")  /* ESC [ ? 25 h，显示光标 */
EST_CSI_RM_DEC_PRIVATE_MODE_STR("25")  /* ESC [ ? 25 l，隐藏光标 */
EST_CSI_SM_ANSI_STANDARD_MODE_STR("4") /* ESC [ 4 h */
EST_CSI_DECRQM_DEC_PRIVATE_MODE_STR("25") /* ESC [ ? 25 $ p */
```

这些语义包装当前提供 `_STR` 版本，没有对应的 `_SSTR` 版本。模式宏中的 `?` 只放在整个参数列表的开头；多个参数应写成一个已拼好的参数
字符串，例如 `EST_CSI_SM_DEC_PRIVATE_MODE_STR("25;2004")`，而不是在每个参数
前重复 `?`。

### 设备状态查询

| 宏 | 请求序列 | 预期响应示例 |
| --- | --- | --- |
| `EST_CSI_DSR_STR()` | `CSI 5 n` | `CSI 0 n` 或 `CSI 3 n` |
| `EST_CSI_CPR_STR()` | `CSI 6 n` | `CSI row;column R` |
| `EST_CSI_DA1_STR()` | `CSI 0 c` | `CSI ? ... c` |
| `EST_CSI_DA2_STR()` | `CSI > 0 c` | `CSI > ... c` |
| `EST_CSI_XTVERSION_STR()` | `CSI > 0 q` | `DCS > \| ... ST` |

这些宏只生成请求，不会读取或解析响应。

## 颜色接口

### SGR 基础样式和颜色

SGR 使用 `EST_CSI_SGR_*` 参数常量和 `EST_CSI_SGR_SSTR(Ps)` 构造器：

```c
EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD)
EST_CSI_SGR_SSTR(EST_CSI_SGR_FC_RED)
EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
```

常量包括重置、粗体、淡化、斜体、下划线、闪烁、反显、隐藏、删除线，以及基础
前景色 `EST_CSI_SGR_FC_*`、背景色 `EST_CSI_SGR_BC_*`、亮前景色 `BFC_*` 和亮
背景色 `BBC_*`。

### 256 色和真彩色

扩展颜色的格式是 `CSI 38/48/58;5;n m` 或 `CSI 38/48/58;2;r;g;b m`：

```c
/* 256 色索引 196 的前景色 */
EST_CSI_SGR_FC_EXT_256_STR("196")

/* RGB(255, 0, 0) 的前景色 */
EST_CSI_SGR_FC_EXT_TRUE_SSTR(255, 0, 0)

/* 以字符串片段提供 RGB 参数 */
EST_CSI_SGR_TRUE_COLOR_EXT_STR("38", "255", "0", "0")
```

`EST_CSI_SGR_FC_EXT_TRUE_STR(r, g, b)` 接收可参与字面量拼接的字符串片段或数字
字面量。对于预处理器 token，使用 `EST_CSI_SGR_FC_EXT_TRUE_SSTR(255, 0, 0)`；
该 `_SSTR` 版本会分别将每个分量字符串化后再生成序列。对于 256 色索引，使用
`EST_CSI_SGR_FC_EXT_256_SSTR(196)`。

256 色辅助宏的范围如下：

- 基础/亮色：索引 `0..15`，对应 `EST_CSI_SGR_256_COLOR_*`。
- 6×6×6 色立方体：索引 `16..231`，每个通道等级 `0..5`；使用
  `EST_CSI_SGR_256_COLOR_CUBE_INDEX(r, g, b)` 计算索引。
- 灰度：索引 `232..255`，使用 `EST_CSI_SGR_256_COLOR_GRAYSCALE_INDEX(v)`
  根据近似灰度值计算索引。

这些宏只计算/拼接颜色参数，不验证输入是否在范围内；调用方应使用头文件提供的
`*_IN_RANGE` 宏或自行校验。

### 文本着色

`ESCAPE_COLORIZE_STR(text, ...)` 接收字符串片段形式的 SGR 参数，
`ESCAPE_COLORIZE_SSTR(text, ...)` 将预处理器 token 字符串化。两者都会在文本后
追加 SGR 重置序列：

```c
ESCAPE_COLORIZE_STR("warning", "31")
ESCAPE_COLORIZE_SSTR("success", EST_CSI_SGR_BOLD, EST_CSI_SGR_FC_GREEN)
```

## 基础序列构造器

需要构造头文件尚未提供语义包装的序列时，可以直接使用：

```c
EST_CSI_STR("?25", "", "h")       /* CSI ?25h */
EST_OSC_STR("0;my title")          /* OSC 0;my title ST */
EST_DCS_STR("", ">", "|", "1.0")  /* DCS >|1.0 ST */
EST_APC_STR("payload")             /* APC payload ST */
EST_ST_STR()                        /* ST */
```

通用构造器的参数是字符串片段：`EST_CSI_STR(P, I, F)` 分别对应参数、中间和
最终字节；`EST_DCS_STR(P, I, F, D)` 还包含数据部分。`EST_CSI_NOP_STR`、
`EST_CSI_NOI_STR` 和 `EST_CSI_NOPI_STR` 用于省略不需要的部分。

## 通用工具宏

头文件还包含一组与控制序列无关的预处理器工具：

| 类别 | 主要宏 | 作用 |
| --- | --- | --- |
| 字符串化/连接 | `ESCAPE_S`, `ESCAPE_C` | 两级展开后字符串化或 token 粘合 |
| 参数展开 | `ESCAPE_X`, `ESCAPE_X_ARG_CNT` | 按参数数量选择宏，当前支持 0 到 7 个参数 |
| 拼接 | `ESCAPE_JOIN`, `ESCAPE_JOINS` | 分别拼接字符串参数或先字符串化后拼接 |
| 范围/数值 | `ESCAPE_MIN`, `ESCAPE_MAX`, `ESCAPE_CLAMP`, `ESCAPE_IN_RANGE`, `ESCAPE_ROUND_DIV` | 常用数值操作 |
| 诊断 | `ESCAPE_ASSERT`, `ESCAPE_TODO` | 输出诊断信息到 `stderr`，触发时调用 `abort` |

`ESCAPE_SAFE_*` 版本会先保存参数，避免同一参数被宏重复求值；它们使用 GNU
statement expression 扩展。默认别名 `ESCAPE_MIN`、`ESCAPE_MAX` 等指向
`UNSAFE` 版本，调用带副作用的表达式时应谨慎。

## 兼容性与限制

1. 这是预处理器驱动的接口，不是运行时格式化库；宏参数必须能够在编译期展开。
2. `ESCAPE_S` 只能字符串化 token，不能把运行时整数转换成文本。
3. `ESCAPE_SAFE_*` 和 `ESCAPE_TYPEOF` 的可用性取决于编译器。GCC/Clang 的 GNU
   C 模式是当前最稳妥的使用方式；C++ 还需要支持 `decltype`，而安全宏仍依赖
   statement expression 扩展。
4. 空可变参数兼容逻辑优先使用 `__VA_OPT__`，否则在 GCC/Clang 下使用
    `##__VA_ARGS__` 扩展；其他编译器需要调用方手动定义兼容宏。
5. 终端控制序列不是跨终端完全一致的协议。尤其是 `CSI s/u`、左右边距、鼠标、
   同步输出和查询命令，应结合目标终端文档测试。
6. 颜色是否生效由终端和输出环境决定。本库不会检查 `TERM`、`COLORTERM`，也不
   会自动禁用颜色。
7. `ESCAPE_ASSERT` 和 `ESCAPE_TODO` 会向 `stderr` 输出诊断信息；触发后调用 `abort`。

## 验证与项目状态

### 当前建议的验证方式

使用一个只包含 `escape.h` 的最小程序进行编译：

```sh
cc -std=gnu11 -Wall -Wextra -pedantic -fsyntax-only demo.c
```

也可以通过打印序列的十六进制字节，确认结果以 `0x1b` 开头，并按预期包含 `[`
或其他引导符。不要把终端渲染结果当作唯一验证手段。

### 文件说明

| 文件 | 状态 |
| --- | --- |
| [`escape.h`](escape.h) | 当前主要实现：工具宏、序列类型和 CSI 构造器 |
| [`README.md`](README.md) | 英文项目入口文档和当前 API 使用说明 |
| [`README.zh-CN.md`](README.zh-CN.md) | 中文项目入口文档 |

以下接口不属于当前 API：OSC 语义包装、ESC 单字符语义包装、DCS 语义包装、
运行时 I/O、终端能力检测和响应解析。

## 参考资料

- [ECMA-48](https://www.ecma-international.org/publications-and-standards/standards/ecma-48/)：控制功能编码的基础规范。
- [Xterm Control Sequences](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html)：xterm 扩展和具体终端行为参考。
