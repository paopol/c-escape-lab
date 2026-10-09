# c-escape-lab

[English](README.md) | [简体中文](README.zh-CN.md)

[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Language: C](https://img.shields.io/badge/language-C-555555.svg)]()
[![single-header](https://img.shields.io/badge/single--header-yes-brightgreen.svg)]()

一个单头文件的 ANSI/VT 终端控制序列库：用 C 预处理器宏在**编译期**将控制序列拼接为字符串字面量，供 `printf`、`fputs` 等直接输出，无需链接任何库。

<p align="center">
  <img src="assets/demo.png" alt="c-escape-lab demo" width="520">
</p>

## 特性

- **单头文件，零依赖**：复制一个 `escape.h` 即可，不需要链接任何库。
- **编译期生成，零运行时开销**：所有宏展开为字符串字面量，无运行时格式化成本。
- **覆盖常用 CSI**：光标移动、擦除编辑、SGR 样式、模式开关、设备查询、滚动区域。
- **完整颜色支持**：ANSI 16 色、256 色（含 6×6×6 色立方体与灰度）、真彩色。
- **底层构造器**：`ESC` / `CSI` / `OSC` / `DCS` / `APC` 等可构造任意尚未封装的序列。
- **GCC / Clang 友好**：GNU C 为主，兼容 C++（需 `decltype`）。

## 安装

将 [`escape.h`](escape.h) 加入你的项目，或让编译器指向本仓库：

```sh
# 方式一：复制头文件
cp path/to/c-escape-lab/escape.h .

# 方式二：用 -I 指向仓库
cc -std=gnu11 -I path/to/c-escape-lab your.c -o your
```

编译器要求：GCC 或 Clang，建议 `-std=gnu11`（使用了 GNU 扩展；C++ 需要 `decltype` 支持）。Windows 用户可使用 MinGW-w64 的 `gcc`，命令形式相同。

## 快速开始

```c
#include <stdio.h>
#include "escape.h"

int main(void)
{
    /* 彩色文本：ESCAPE_COLORIZE_SSTR 会在文本后自动补上 SGR 复位 */
    printf("Hello, " ESCAPE_COLORIZE_SSTR("world", EST_CSI_SGR_BOLD, EST_CSI_SGR_FC_GREEN) "!\n");

    /* 256 色前景 + 显式复位 */
    printf(EST_CSI_SGR_FC_EXT_256_SSTR(196) "256-color" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");

    /* 真彩色前景 + 显式复位 */
    printf(EST_CSI_SGR_FC_EXT_TRUE_SSTR(255, 80, 20) "truecolor" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET) "\n");

    return 0;
}
```

将上面的代码保存为 `demo.c`（或任意 `.c` 文件），然后编译运行：

```sh
cc -std=gnu11 -Wall -Wextra -pedantic demo.c -o demo
./demo
```

更多主题示例（颜色、光标、擦除、模式、底层构造器）见 [`examples/`](examples/)；在该目录下执行 `make` 一键编译、`make run` 运行。

## 用法

### `_STR` 和 `_SSTR` 的区别

这是唯一必须掌握的概念：

- `*_STR`：参数是**已拼接好的字符串片段**，例如 `"5"`、`"1;10"`。
- `*_SSTR`：参数是**预处理 token**（数字、宏名），库会先用 `ESCAPE_S(...)` 把它字符串化。

```c
EST_CSI_CUU_STR("5")    /* "\x1b[5A" —— 参数是字符串 */
EST_CSI_CUU_SSTR(5)     /* "\x1b[5A" —— 参数是 token，结果相同 */

EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD)   /* "\x1b[1m" */
```

### 结果是编译期字面量，运行时数值用 `snprintf`

所有宏都展开为**字符串字面量**，不能直接传入运行时变量（例如 `int n`）。运行时数值的推荐写法：将格式符作为字符串片段传入 `_STR`，再由 `snprintf` 填充。

```c
char buf[32];
int n = 5;
snprintf(buf, sizeof buf, EST_CSI_CUU_STR("%d"), n);  /* buf = "\x1b[5A" */
fputs(buf, stdout);
```

原理：`EST_CSI_CUU_STR("%d")` 展开为 `"\x1b[%dA"`，随后由 `snprintf` 把 `%d` 替换成 `n`。

> **注意**：`ESCAPE_COLORIZE_*` 的 `text` 以及所有 `*_STR` 的参数都必须是**字符串字面量**（或能展开为字面量的宏），不能传 `char *` 变量——`ESCAPE_COLORIZE_STR(text, ...)` 展开为 `"..." text "..."` 形式的相邻字面量拼接，传变量会导致编译错误。运行时字符串可传 `"%s"` 占位符并配合 `printf`，或分三段 `fputs` 输出：

```c
/* 一行：printf + %s 占位符 */
printf(ESCAPE_COLORIZE_STR("%s", "31"), text);

/* 或分三段 fputs */
fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_FC_RED), stdout);
fputs(text, stdout);
fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
```

### 能力速览

| 分类 | 代表性宏 | 说明 |
| --- | --- | --- |
| 文本样式与颜色 | `ESCAPE_COLORIZE_SSTR(text, ...)`、`EST_CSI_SGR_SSTR(Ps)` | 粗体/斜体/颜色/复位；256 色、真彩色 |
| 光标移动与定位 | `EST_CSI_CUU_SSTR(n)`、`EST_CSI_CUP_SSTR(row, col)` | 上下左右、绝对行列、保存/恢复光标 |
| 擦除与编辑 | `EST_CSI_EL_SSTR(2)`、`EST_CSI_ED_SSTR(2)` | 擦行/擦屏、插入/删除字符与行 |
| 模式与查询 | `EST_CSI_SM_DEC_PRIVATE_MODE_STR("25")`、`EST_CSI_CPR_STR()` | 显示/隐藏光标等模式；设备状态查询 |
| 底层构造器 | `EST_CSI_STR(P, I, F)`、`EST_OSC_STR(S)` | 构造任意尚未封装的序列 |

完整的逐宏列表见 [API 参考](#api-参考)。

## 注意事项

1. **颜色/扩展序列取决于终端**：本库不检测 `TERM`/`COLORTERM`，也不会自动降级或禁用颜色。
2. **部分序列是终端扩展**：`CSI s/u`、左右边距、鼠标模式、同步输出、查询命令等，需结合目标终端文档测试。
3. **验证生成字节**：不确定时打印十六进制，确认以 `0x1b` 开头、并按预期包含 `[` 等引导符；不应仅凭终端渲染效果判断。
4. `ESCAPE_COLORIZE_STR(text)` 未传入任何 SGR 参数时不会产生样式：前缀退化为空参数复位（`ESC[m`），尾部仍会追加 `ESC[0m`）。

## API 参考

### 命名空间

| 前缀 | 用途 |
| --- | --- |
| `ESCAPE_` | 预处理器工具、断言、TODO、数值工具 |
| `EST_` | 序列类型、参数、最终字节与序列构造器 |
| `EST_CSI_` | CSI 序列及其参数 |
| `ESCAPE_INTRODUCER_` | 各类控制序列的引导符 |
| `ESC_*` | ESC 字节常量（十六进制/八进制/默认别名） |
| `ESTO_*` / `EST_*` | 序列类型枚举 |

### ESC 常量与引导符

ESC 字节有十六进制（`_HEX`）与八进制（`_OCT`）两套，`ESC_*_DEF` 默认指向 `_HEX` 版本：

| 十六进制 | 值 | 八进制 | 值 |
| --- | --- | --- | --- |
| `ESC_VAL_HEX` | `0x1b` | `ESC_VAL_OCT` | `033` |
| `ESC_RAW_HEX` | `\x1b` | `ESC_RAW_OCT` | `\033` |
| `ESC_CHR_HEX` | `'\x1b'` | `ESC_CHR_OCT` | `'\033'` |
| `ESC_STR_HEX` | `"\x1b"` | `ESC_STR_OCT` | `"\033"` |

引导符：

| 宏 | 展开 |
| --- | --- |
| `ESCAPE_INTRODUCER_ESC` | `ESC` |
| `ESCAPE_INTRODUCER_CSI` | `ESC [` |
| `ESCAPE_INTRODUCER_OSC` | `ESC ]` |
| `ESCAPE_INTRODUCER_DCS` | `ESC P` |
| `ESCAPE_INTRODUCER_APC` | `ESC _` |
| `ESCAPE_INTRODUCER_ST` | `ESC \` |
| `ESCAPE_INTRODUCER_SOS` | `ESC X` |
| `ESCAPE_INTRODUCER_PM` | `ESC ^` |

### 底层构造器

| 宏 | 形式 |
| --- | --- |
| `EST_ESC_STR(_I, F)` | `ESC _I F` |
| `EST_ESC_NOI_STR(F)` | `ESC F` |
| `EST_CSI_STR(_P, _I, F)` | `CSI _P _I F` |
| `EST_CSI_NOP_STR(_I, F)` | `CSI _I F`（省略参数段） |
| `EST_CSI_NOI_STR(_P, F)` | `CSI _P F`（省略中间段） |
| `EST_CSI_NOPI_STR(F)` | `CSI F`（省略参数与中间段） |
| `EST_OSC_STR(S)` | `OSC S ST` |
| `EST_DCS_STR(_P, _I, F, D)` | `DCS _P _I F D ST` |
| `EST_DCS_NOP_STR(_I, F, D)` | `DCS _I F D ST`（省略参数段） |
| `EST_DCS_NOI_STR(_P, F, D)` | `DCS _P F D ST`（省略中间段） |
| `EST_DCS_NOPI_STR(F, D)` | `DCS F D ST`（省略参数与中间段） |
| `EST_APC_STR(D)` | `APC D ST` |
| `EST_ST_STR()` | `ST` |
| `EST_SOS_STR(D)` / `EST_PM_STR(D)` | `SOS D ST` / `PM D ST` |

> 目前 `OSC` / `DCS` / `APC` / `ESC` 仅提供底层构造器，尚无语义包装（如设置窗口标题、`DECRQSS`）；单字符 ESC 语义包装、查询响应的读取与解析、终端能力检测也尚未提供。

### 类型枚举

`ESCAPE_SEQUENCE_TYPE`（配合 `ESCAPE_SEQUENCE_TYPE_ORDER`）用块偏移 `EST_BLOCK_OFFSET = 0x0100` 为各序列族预留编号区间：

| 族 | 序值 | 枚举名 | 取值 |
| --- | --- | --- | --- |
| ESC | `ESTO_ESC` | `EST_ESC` | `0x100` |
| CSI | `ESTO_CSI` | `EST_CSI` | `0x200` |
| OSC | `ESTO_OSC` | `EST_OSC` | `0x300` |
| DCS | `ESTO_DCS` | `EST_DCS` | `0x400` |
| APC | `ESTO_APC` | `EST_APC` | `0x500` |
| ST | `ESTO_ST` | `EST_ST` | `0x600` |
| SOS | `ESTO_SOS` | `EST_SOS` | `0x700` |
| PM | `ESTO_PM` | `EST_PM` | `0x800` |

`EST_CSI_TYPE` 从 `EST_CSI`（`0x200`）开始逐个列出所有 CSI 类型（`CUU`、`CUD` … `DECRQM`），以 `EST_CSI_END` / `EST_CSI_NUMS` 收尾。这些枚举目前主要用于预留编号与自描述，序列字符串仍由下面的宏生成。

### CSI 序列

表内 `_SSTR` / `_DEFAULT` 列标记对应版本是否存在（`✓` 有 / `—` 无）。`_DEFAULT_STR` 显式填入协议默认参数（光标移动类默认 `1`，擦除类默认 `0` 或 `1`，`TBC` 默认 `0`，`DECSCUSR`/`DECSCA` 默认 `0`）。

| 宏（`_STR` 形式） | `_SSTR` | `_DEFAULT` | 生成 | 说明 |
| --- | --- | --- | --- | --- |
| `EST_CSI_CUU_STR(Ps)` | ✓ | ✓ | `CSI Ps A` | 上移 |
| `EST_CSI_CUD_STR(Ps)` | ✓ | ✓ | `CSI Ps B` | 下移 |
| `EST_CSI_CUF_STR(Ps)` | ✓ | ✓ | `CSI Ps C` | 右移 |
| `EST_CSI_CUB_STR(Ps)` | ✓ | ✓ | `CSI Ps D` | 左移 |
| `EST_CSI_CNL_STR(Ps)` | ✓ | ✓ | `CSI Ps E` | 下移并到行首 |
| `EST_CSI_CPL_STR(Ps)` | ✓ | ✓ | `CSI Ps F` | 上移并到行首 |
| `EST_CSI_CHA_STR(Ps)` | ✓ | ✓ | `CSI Ps G` | 设置绝对列 |
| `EST_CSI_CUP_STR(Pr, Pc)` | ✓ | ✓ | `CSI Pr;Pc H` | 设置行、列 |
| `EST_CSI_HVP_STR(Pr, Pc)` | ✓ | ✓ | `CSI Pr;Pc f` | 水平垂直定位 |
| `EST_CSI_VPA_STR(Ps)` | ✓ | ✓ | `CSI Ps d` | 设置绝对行 |
| `EST_CSI_CHT_STR(Ps)` | ✓ | ✓ | `CSI Ps I` | 前移制表位 |
| `EST_CSI_CBT_STR(Ps)` | ✓ | ✓ | `CSI Ps Z` | 后移制表位 |
| `EST_CSI_SCP_STR()` | ✓ | — | `CSI s` | 保存光标（启用 `?69` 后被 DECSLRM 占用） |
| `EST_CSI_RCP_STR()` | ✓ | — | `CSI u` | 恢复光标（部分终端可能被扩展占用） |
| `EST_CSI_DECSC_STR()` | ✓ | — | `ESC 7` | DEC 保存光标 |
| `EST_CSI_DECRC_STR()` | ✓ | — | `ESC 8` | DEC 恢复光标 |
| `EST_CSI_ED_STR(Ps)` | ✓ | ✓ | `CSI Ps J` | 擦屏：`0` 到末尾、`1` 到开头、`2` 全屏、`3` 清滚动区 |
| `EST_CSI_EL_STR(Ps)` | ✓ | ✓ | `CSI Ps K` | 擦行：`0` 到行尾、`1` 到行首、`2` 整行 |
| `EST_CSI_ICH_STR(Ps)` | ✓ | ✓ | `CSI Ps @` | 插入空白字符 |
| `EST_CSI_DCH_STR(Ps)` | ✓ | ✓ | `CSI Ps P` | 删除字符 |
| `EST_CSI_ECH_STR(Ps)` | ✓ | ✓ | `CSI Ps X` | 擦除字符（光标不动） |
| `EST_CSI_IL_STR(Ps)` | ✓ | ✓ | `CSI Ps L` | 插入行 |
| `EST_CSI_DL_STR(Ps)` | ✓ | ✓ | `CSI Ps M` | 删除行 |
| `EST_CSI_SU_STR(Ps)` | ✓ | ✓ | `CSI Ps S` | 向上滚动 |
| `EST_CSI_SD_STR(Ps)` | ✓ | ✓ | `CSI Ps T` | 向下滚动 |
| `EST_CSI_REP_STR(Ps)` | ✓ | ✓ | `CSI Ps b` | 重复前一字符 |
| `EST_CSI_SGR_STR(Ps)` | ✓ | — | `CSI Ps m` | 选择图形再现（样式/颜色） |
| `EST_CSI_SM_STR(Ps)` | ✓ | — | `CSI Ps h` | 设置模式 |
| `EST_CSI_RM_STR(Ps)` | ✓ | — | `CSI Ps l` | 复位模式 |
| `EST_CSI_DSR_STR()` | — | — | `CSI 5 n` | 设备状态报告 |
| `EST_CSI_CPR_STR()` | — | — | `CSI 6 n` | 光标位置报告 |
| `EST_CSI_DA1_STR()` | — | — | `CSI 0 c` | 主设备属性 |
| `EST_CSI_DA2_STR()` | — | — | `CSI >0 c` | 次设备属性 |
| `EST_CSI_XTVERSION_STR()` | — | — | `CSI >0 q` | xterm 版本 |
| `EST_CSI_DECSTBM_STR(Pt, Pb)` | — | — | `CSI Pt;Pb r` | 设置上下边距 |
| `EST_CSI_DECSLRM_STR(Pl, Pr)` | — | — | `CSI Pl;Pr s` | 设置左右边距（需先 `?69h`） |
| `EST_CSI_TBC_STR(Ps)` | — | ✓ | `CSI Ps g` | 清制表位：`0` 当前、`3` 全部 |
| `EST_CSI_DECSTR_STR()` | — | — | `CSI ! p` | 软复位 |
| `EST_CSI_DECSCUSR_STR(Ps)` | — | ✓ | `CSI Ps SP q` | 光标样式（`0` 闪烁块（默认）、`1` 闪烁块、`2` 稳定块、`3` 闪烁下划线、`4` 稳定下划线、`5` 闪烁竖线、`6` 稳定竖线） |
| `EST_CSI_DECSCA_STR(Ps)` | — | ✓ | `CSI Ps " q` | 选择性擦除保护（`0` 可擦、`1` 保护、`2` 同 `0`） |
| `EST_CSI_DECRQM_STR(Ps)` | — | — | `CSI Ps $ p` | 请求模式（包装见下文） |

### SGR 样式与颜色

样式属性常量（配合 `EST_CSI_SGR_SSTR(Ps)` 使用）：

| 宏 | 值 | 含义 |
| --- | --- | --- |
| `EST_CSI_SGR_RESET` / `NORMAL` | 0 | 复位 |
| `EST_CSI_SGR_BOLD` / `INCREASED_INTENSITY` | 1 | 粗体/增强 |
| `EST_CSI_SGR_FAINT` / `DECREASED_INTENSITY` | 2 | 淡显/减弱 |
| `EST_CSI_SGR_ITALIC` | 3 | 斜体 |
| `EST_CSI_SGR_UNDERLINE` | 4 | 下划线 |
| `EST_CSI_SGR_SLOW_BLINK` | 5 | 慢闪 |
| `EST_CSI_SGR_RAPID_BLINK` | 6 | 快闪 |
| `EST_CSI_SGR_REVERSE_VIDEO` / `SWAP_FORE_BACK` | 7 | 反显 |
| `EST_CSI_SGR_HIDE` / `CONCEAL` | 8 | 隐藏 |
| `EST_CSI_SGR_STRIKE` / `CROSSED_OUT` | 9 | 删除线 |
| `EST_CSI_SGR_DOUBLY_UNDERLINE` | 21 | 双下划线 |
| `EST_CSI_SGR_NOT_BOLD` / `NORMAL_INTENSITY` | 22 | 取消粗体 |
| `EST_CSI_SGR_NOT_ITALIC` | 23 | 取消斜体 |
| `EST_CSI_SGR_NOT_UNDERLINE` | 24 | 取消下划线 |
| `EST_CSI_SGR_NOT_BLINKING` | 25 | 取消闪烁 |
| `EST_CSI_SGR_PROPORTIONAL_SPACING` | 26 | 比例间距 |
| `EST_CSI_SGR_NOT_REVERSED` | 27 | 取消反显 |
| `EST_CSI_SGR_REVEAL` / `NOT_HIDDEN` | 28 | 取消隐藏 |
| `EST_CSI_SGR_NOT_STRIKE` / `NOT_CROSSED_OUT` | 29 | 取消删除线 |
| `EST_CSI_SGR_NOT_PROPORTIONAL_SPACING` | 50 | 取消比例间距 |

颜色常量（按类别）：

| 类别 | 宏前缀 | 值 |
| --- | --- | --- |
| 前景色 | `EST_CSI_SGR_FC_*` | `30–37`（黑红绿黄蓝品青白）、`38` 扩展、`39` 默认 |
| 背景色 | `EST_CSI_SGR_BC_*` | `40–47`、`48` 扩展、`49` 默认 |
| 下划线色 | `EST_CSI_SGR_UC_*` | `58` 扩展、`59` 默认 |
| 亮前景色 | `EST_CSI_SGR_BFC_*` | `90–97` |
| 亮背景色 | `EST_CSI_SGR_BBC_*` | `100–107` |

扩展颜色构造器：

| 宏 | 生成 |
| --- | --- |
| `EST_CSI_SGR_COLOR_EXT_STR(m1, m2, c)` | `CSI m1;m2;c m` |
| `EST_CSI_SGR_256_COLOR_EXT_STR(m, n)` | `CSI m;5;n m` |
| `EST_CSI_SGR_TRUE_COLOR_EXT_STR(m, r, g, b)` | `CSI m;2;r;g;b m` |
| `EST_CSI_SGR_FC_EXT_256_STR(n)` / `_SSTR(n)` | `CSI 38;5;n m` |
| `EST_CSI_SGR_BC_EXT_256_STR(n)` / `_SSTR(n)` | `CSI 48;5;n m` |
| `EST_CSI_SGR_UC_EXT_256_STR(n)` / `_SSTR(n)` | `CSI 58;5;n m` |
| `EST_CSI_SGR_FC_EXT_TRUE_STR(r, g, b)` / `_SSTR(r, g, b)` | `CSI 38;2;r;g;b m` |
| `EST_CSI_SGR_BC_EXT_TRUE_STR(r, g, b)` / `_SSTR(r, g, b)` | `CSI 48;2;r;g;b m` |
| `EST_CSI_SGR_UC_EXT_TRUE_STR(r, g, b)` / `_SSTR(r, g, b)` | `CSI 58;2;r;g;b m` |

> `_STR` 版本只接受字符串片段（如 `"196"`、`"255"`）；token 或数字字面量请用 `_SSTR`。

256 色辅助宏：

| 宏 | 说明 |
| --- | --- |
| `EST_CSI_SGR_256_COLOR_*` | 基础色索引 `0..7`、亮色 `8..15` |
| `EST_CSI_SGR_256_COLOR_CUBE_INDEX(r, g, b)` | 6×6×6 色立方体索引 `16..231` |
| `EST_CSI_SGR_256_COLOR_CUBE_LEVEL_R/G/B(index)` | 由索引反解各通道等级 `0..5` |
| `EST_CSI_SGR_256_COLOR_CUBE_R/G/B(level)` | 等级 → 真实 RGB 分量（查表 `{0, 95, 135, 175, 215, 255}`） |
| `EST_CSI_SGR_256_COLOR_GRAYSCALE_INDEX(v)` | 灰度值 → 索引 `232..255` |
| `EST_CSI_SGR_256_COLOR_GRAYSCALE_GRAY/R/G/B(index)` | 索引 → 灰度值 |
| `EST_CSI_SGR_256_COLOR_CUBE_*_IN_RANGE(...)` / `GRAYSCALE_*_IN_RANGE(...)` | 各范围校验宏 |

### 模式与查询

模式参数常量（用于 `SM`/`RM`）：

| 宏 | 值 | 说明 |
| --- | --- | --- |
| `EST_CSI_SM_RM_IRM` | 4 | 插入模式（ANSI 标准） |
| `EST_CSI_SM_RM_LNM` | 20 | 换行模式（ANSI 标准） |
| `EST_CSI_SM_RM_DECCKM` | 1 | 光标键模式 |
| `EST_CSI_SM_RM_DECCOLM` | 3 | 132 列模式 |
| `EST_CSI_SM_RM_DECSCNM` | 5 | 反显屏幕 |
| `EST_CSI_SM_RM_DECOM` | 6 | 原点模式 |
| `EST_CSI_SM_RM_DECAWM` | 7 | 自动换行 |
| `EST_CSI_SM_RM_DECTCEM` | 25 | 显示光标 |
| `EST_CSI_SM_RM_MODE_47` | 47 | 备用屏幕 |
| `EST_CSI_SM_RM_DECLRMM` | 69 | 左右边距模式 |
| `EST_CSI_SM_RM_MODE_1000/1002/1003/1004/1006` | 1000/1002/1003/1004/1006 | 鼠标报告与焦点事件 |
| `EST_CSI_SM_RM_MODE_1047/1048/1049` | 1047/1048/1049 | 备用屏幕与保存光标 |
| `EST_CSI_SM_RM_MODE_2004` | 2004 | 括号粘贴 |
| `EST_CSI_SM_RM_MODE_2026` | 2026 | 同步输出 |

语义包装：

| 宏 | 生成 |
| --- | --- |
| `EST_CSI_SM_ANSI_STANDARD_MODE_STR(Ps)` | `CSI Ps h` |
| `EST_CSI_SM_DEC_PRIVATE_MODE_STR(Ps)` | `CSI ? Ps h` |
| `EST_CSI_RM_ANSI_STANDARD_MODE_STR(Ps)` | `CSI Ps l` |
| `EST_CSI_RM_DEC_PRIVATE_MODE_STR(Ps)` | `CSI ? Ps l` |
| `EST_CSI_DECRQM_ANSI_STANDARD_MODE_STR(Ps)` | `CSI Ps $ p` |
| `EST_CSI_DECRQM_DEC_PRIVATE_MODE_STR(Ps)` | `CSI ? Ps $ p` |

这些语义包装目前只提供 `_STR` 版本。DEC 私有模式中的 `?` 只出现在参数列表开头：多参数写成一个已拼接好的字符串，如 `EST_CSI_SM_DEC_PRIVATE_MODE_STR("25;2004")`，不要在每个参数前重复 `?`。

### 通用工具宏

| 类别 | 宏 | 说明 |
| --- | --- | --- |
| 字符串化 / 粘合 | `ESCAPE_S` / `ESCAPE_C` | 两级展开后字符串化 / token 粘合 |
| 变参分派 | `ESCAPE_X` / `ESCAPE_X_ARG_CNT` | 按参数个数选择宏（0–7） |
| 拼接 | `ESCAPE_JOIN` / `ESCAPE_JOINS` | 拼接字符串参数 / 先字符串化再拼接 |
| 分号拼接 | `ESCAPE_JOIN_SEMICOLON` / `ESCAPE_JOINS_SEMICOLON` | 同上，分隔符固定为 `;` |
| RGB | `ESCAPE_RGB` / `ESCAPE_SRGB` | 生成 `r;g;b` |
| 数值 | `ESCAPE_MIN` / `ESCAPE_MAX` / `ESCAPE_CLAMP` / `ESCAPE_IN_RANGE` / `ESCAPE_ROUND_DIV` | 常用数值操作 |
| 诊断 | `ESCAPE_ASSERT` / `ESCAPE_TODO` | 输出到 `stderr` 并 `abort()` |
| 其他 | `ESCAPE_EMPTY` / `ESCAPE_APPLY` / `ESCAPE_TYPEOF` | 空操作 / 应用宏 / 类型推导 |

`ESCAPE_SAFE_*` 版本会先保存参数，避免同一参数被重复求值（依赖 GNU statement expression）；默认别名 `ESCAPE_MIN`、`ESCAPE_MAX` 等指向 `UNSAFE` 版本，带副作用的表达式需谨慎。

### 兼容性宏

| 宏 | 说明 |
| --- | --- |
| `ESCAPE_COMPAT_HAS_VA_OPT` | 是否可用 `__VA_OPT__` |
| `ESCAPE_COMPAT_COMMA_VA_ARGS` / `ESCAPE_COMMA_VA_ARGS` | 空变参逗号兼容（`__VA_OPT__` 或 `##__VA_ARGS__`） |
| `ESCAPE_COMPAT_TYPEOF` / `ESCAPE_TYPEOF` | `typeof` / `decltype` 兼容 |
| `ESCAPE_DEBUG` | 调试开关，默认 `1`（当前仅预留） |

## 贡献指南

欢迎提交 Issue 和 Pull Request。新增宏请遵循现有命名规范（`ESCAPE_*` / `EST_*` / `EST_CSI_*`），并同步更新本文档的 API 参考部分。

## 许可证

[MIT](LICENSE) © 2026 paopol

## 参考资料

- [ECMA-48](https://www.ecma-international.org/publications-and-standards/standards/ecma-48/)：控制功能编码的基础规范。
- [Xterm Control Sequences](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html)：xterm 扩展和具体终端行为参考。
