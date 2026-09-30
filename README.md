# c-escape-lab

[English](README.md) | [简体中文](README.zh-CN.md)

> Documentation note: This README was drafted with AI assistance and checked against the current header implementation.

A terminal control-sequence library built around C preprocessor macros. The implementation
is concentrated in [`escape.h`](escape.h): it composes ANSI/VT-style control sequences
as string literals for use with `printf`, `fputs`, or another output API.

The interface is header-only. Include `escape.h` directly.

## Contents

- [c-escape-lab](#c-escape-lab)
  - [Contents](#contents)
  - [Project Scope](#project-scope)
    - [What is currently provided](#what-is-currently-provided)
    - [What is not currently provided](#what-is-not-currently-provided)
  - [Quick Start](#quick-start)
  - [Expansion Model](#expansion-model)
    - [Two input conventions](#two-input-conventions)
    - [How a sequence is built](#how-a-sequence-is-built)
  - [API Overview](#api-overview)
    - [Namespaces](#namespaces)
    - [Sequence families](#sequence-families)
  - [Common CSI APIs](#common-csi-apis)
    - [Cursor movement and position](#cursor-movement-and-position)
    - [Erasing, editing, and scrolling](#erasing-editing-and-scrolling)
    - [Mode control and queries](#mode-control-and-queries)
    - [Device-status queries](#device-status-queries)
  - [Color APIs](#color-apis)
    - [SGR styles and basic colors](#sgr-styles-and-basic-colors)
    - [256-color and truecolor](#256-color-and-truecolor)
  - [Low-Level Sequence Builders](#low-level-sequence-builders)
  - [General Utility Macros](#general-utility-macros)
  - [Compatibility and Limitations](#compatibility-and-limitations)
  - [Verification and Project Status](#verification-and-project-status)
    - [Recommended verification](#recommended-verification)
    - [Files](#files)
  - [References](#references)

## Project Scope

### What is currently provided

- A single-header interface with no library link step.
- `ESCAPE_*` utility macros for token stringizing, concatenation, and variadic expansion.
- The `EST_*` (Escape Sequence Type) namespace for sequence types, parameters, and builders.
- Common CSI sequences for cursor movement, erasing/editing, SGR styling, mode control,
  device queries, scrolling regions, tab stops, and selected terminal extensions.
- Constants and conversion macros for ANSI colors, 256-color indexes, the 6x6x6 color
  cube, and grayscale levels.
- Low-level string builders for ESC, CSI, OSC, DCS, APC, ST, SOS, and PM sequences.

### What is not currently provided

- Terminal output, flushing, response reading, or response parsing. The caller owns I/O.
- Terminal capability detection or automatic fallback for 256-color, truecolor, or extensions.
- Parsing for DSR, CPR, DA, DECRQM, or other query responses.
- A guarantee that every extension sequence is supported by every terminal.

## Quick Start

The following program only depends on `escape.h`, prints colored text, and restores the
normal SGR state:

```c
#include <stdio.h>
#include "escape.h"

int main(void)
{
    printf(
        EST_CSI_SGR_FC_EXT_TRUE_STR(ESCAPE_SRGB(255, 80, 20))
        "orange text"
        EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
        "\n"
    );
    return 0;
}
```

With GCC or Clang, for example:

```sh
cc -std=gnu11 -Wall -Wextra -pedantic demo.c -o demo
./demo
```

On Windows, MinGW-w64 `gcc` works with the same command form. A terminal that does not
support colors displays the control sequence according to its own behavior; this library
does not filter or emulate unsupported control sequences.

## Expansion Model

### Two input conventions

The most important distinction in this project is:

- `*_STR` accepts string fragments such as `"5"` or `"1;10"`.
- `*_SSTR` applies `ESCAPE_S(...)` to stringify a preprocessor token such as `5` or
  `EST_CSI_SGR_RESET`.

```c
/* Both produce: ESC [ 5 A */
EST_CSI_CUU_STR("5")
EST_CSI_CUU_SSTR(5)

/* Both produce: ESC [ 0 m */
EST_CSI_SGR_STR("0")
EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
```

`*_STR` parameters are not runtime variables. Do not pass an `int count` directly to
`EST_CSI_CUU_STR`; use `snprintf` or construct a runtime buffer yourself. The current
project performs compile-time string composition only.

### How a sequence is built

The general CSI form is:

```text
ESC [ P* I* F
```

`P` contains parameter bytes, `I` contains intermediate bytes, and `F` is the final byte:

```c
EST_CSI_CUP_SSTR(10, 20)        /* "\x1b[10;20H" */
EST_CSI_ED_SSTR(2)              /* "\x1b[2J" */
EST_CSI_DECSCUSR_STR("2")       /* "\x1b[2 q" */
```

The expansion is a sequence of adjacent string literals, so it can be combined with
ordinary string literals:

```c
fputs(EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD) "bold" EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET), stdout);
```

## API Overview

### Namespaces

| Prefix | Purpose |
| --- | --- |
| `ESCAPE_` | Preprocessor utilities, assertions, TODO handling, and numeric helpers |
| `EST_` | Sequence types, parameters, final bytes, and sequence builders |
| `EST_CSI_` | CSI sequences and their parameters |
| `ESCAPE_INTRODUCER_` | Introducers for the supported sequence families |

### Sequence families

`escape.h` defines `enum ESCAPE_SEQUENCE_TYPE` and reserves ranges for these families:

| Type | Introducer/terminator form | Current use |
| --- | --- | --- |
| ESC | `ESC` | Single-character ESC sequences |
| CSI | `ESC [` | Cursor, styling, mode, and query sequences |
| OSC | `ESC ] ... ST` | Low-level operating-system-command composition |
| DCS | `ESC P ... ST` | Low-level device-control-string composition |
| APC | `ESC _ ... ST` | Low-level application-command composition |
| ST | `ESC \\` | String terminator |
| SOS / PM | `ESC X` / `ESC ^` | Low-level string forms |

## Common CSI APIs

All of the following macros produce string literals. Macros ending in `_DEFAULT_STR`
explicitly insert the protocol's common default parameter.

### Cursor movement and position

| Macro | Produced form | Meaning |
| --- | --- | --- |
| `EST_CSI_CUU_STR/Ps` | `CSI Ps A` | Move up |
| `EST_CSI_CUD_STR/Ps` | `CSI Ps B` | Move down |
| `EST_CSI_CUF_STR/Ps` | `CSI Ps C` | Move right |
| `EST_CSI_CUB_STR/Ps` | `CSI Ps D` | Move left |
| `EST_CSI_CNL_STR/Ps` | `CSI Ps E` | Move down and to the line beginning |
| `EST_CSI_CPL_STR/Ps` | `CSI Ps F` | Move up and to the line beginning |
| `EST_CSI_CHA_STR/Ps` | `CSI Ps G` | Set the absolute column |
| `EST_CSI_CUP_STR/Pr,Pc` | `CSI Pr;Pc H` | Set row and column |
| `EST_CSI_HVP_STR/Pr,Pc` | `CSI Pr;Pc f` | Horizontal and vertical position |
| `EST_CSI_VPA_STR/Ps` | `CSI Ps d` | Set the absolute row |
| `EST_CSI_CHT_STR/Ps` / `EST_CSI_CBT_STR/Ps` | `CSI Ps I/Z` | Move to the next/previous tab stop |
| `EST_CSI_SCP_STR()` / `EST_CSI_RCP_STR()` | `CSI s/u` | Save/restore the cursor |
| `EST_CSI_DECSC_STR()` / `EST_CSI_DECRC_STR()` | `ESC 7/8` | DEC save/restore cursor |

`_SSTR` variants are available for the main single-parameter and two-parameter builders,
for example `EST_CSI_CUP_SSTR(10, 20)`.

### Erasing, editing, and scrolling

| Macro | Produced form | Common parameters |
| --- | --- | --- |
| `EST_CSI_ED_STR/Ps` | `CSI Ps J` | `0` to end, `1` to beginning, `2` whole display, `3` scrollback |
| `EST_CSI_EL_STR/Ps` | `CSI Ps K` | `0` to line end, `1` to line beginning, `2` whole line |
| `EST_CSI_ICH_STR/Ps` | `CSI Ps @` | Insert blank characters |
| `EST_CSI_DCH_STR/Ps` | `CSI Ps P` | Delete characters |
| `EST_CSI_ECH_STR/Ps` | `CSI Ps X` | Erase characters without moving the cursor |
| `EST_CSI_IL_STR/Ps` / `EST_CSI_DL_STR/Ps` | `CSI Ps L/M` | Insert/delete lines |
| `EST_CSI_SU_STR/Ps` / `EST_CSI_SD_STR/Ps` | `CSI Ps S/T` | Scroll up/down |
| `EST_CSI_REP_STR/Ps` | `CSI Ps b` | Repeat the previous character |
| `EST_CSI_TBC_STR/Ps` | `CSI Ps g` | Clear tab stops; commonly `0` or `3` |
| `EST_CSI_DECSTBM_STR/Pt,Pb` | `CSI Pt;Pb r` | Set the top and bottom margins |
| `EST_CSI_DECSLRM_STR/Pl,Pr` | `CSI Pl;Pr s` | Set the left and right margins; requires `?69` first |

### Mode control and queries

```c
EST_CSI_SM_DEC_PRIVATE_MODE_STR("25")  /* ESC [ ? 25 h, show cursor */
EST_CSI_RM_DEC_PRIVATE_MODE_STR("25")  /* ESC [ ? 25 l, hide cursor */
EST_CSI_SM_ANSI_STANDARD_MODE_STR("4") /* ESC [ 4 h */
EST_CSI_DECRQM_DEC_PRIVATE_MODE_STR("25") /* ESC [ ? 25 $ p */
```

These semantic wrappers currently provide `_STR` variants only; corresponding `_SSTR`
variants are not implemented. The `?` in a private-mode sequence appears once at the
start of the complete parameter list. For multiple modes, pass one composed parameter
string such as `EST_CSI_SM_DEC_PRIVATE_MODE_STR("25;2004")`; do not repeat `?` before
each parameter.

### Device-status queries

| Macro | Request | Example response |
| --- | --- | --- |
| `EST_CSI_DSR_STR()` | `CSI 5 n` | `CSI 0 n` or `CSI 3 n` |
| `EST_CSI_CPR_STR()` | `CSI 6 n` | `CSI row;column R` |
| `EST_CSI_DA1_STR()` | `CSI 0 c` | `CSI ? ... c` |
| `EST_CSI_DA2_STR()` | `CSI > 0 c` | `CSI > ... c` |
| `EST_CSI_XTVERSION_STR()` | `CSI > 0 q` | `DCS > \| ... ST` |

These macros only create the request; they do not read or parse the response.

## Color APIs

### SGR styles and basic colors

SGR uses `EST_CSI_SGR_*` parameter constants with the `EST_CSI_SGR_SSTR(Ps)` builder:

```c
EST_CSI_SGR_SSTR(EST_CSI_SGR_BOLD)
EST_CSI_SGR_SSTR(EST_CSI_SGR_FC_RED)
EST_CSI_SGR_SSTR(EST_CSI_SGR_RESET)
```

Constants cover reset, bold, faint, italic, underline, blinking, reverse video, conceal,
strikeout, basic foreground colors (`EST_CSI_SGR_FC_*`), background colors
(`EST_CSI_SGR_BC_*`), bright foreground colors (`BFC_*`), and bright background colors
(`BBC_*`).

### 256-color and truecolor

Extended colors use either `CSI 38/48/58;5;n m` or `CSI 38/48/58;2;r;g;b m`:

```c
/* Foreground color at 256-color index 196 */
EST_CSI_SGR_FC_EXT_256_STR("196")

/* Foreground RGB(255, 0, 0) */
EST_CSI_SGR_FC_EXT_TRUE_STR(ESCAPE_SRGB(255, 0, 0))

/* RGB parameters already supplied as a string fragment */
EST_CSI_SGR_FC_EXT_TRUE_STR("255;0;0")
```

The argument must be a string literal or a macro that participates in literal
concatenation, not a runtime `char *` variable. For numeric tokens, use
`EST_CSI_SGR_FC_EXT_256_SSTR(196)`.

The 256-color helpers use these ranges:

- Base and bright colors: indexes `0..15`, named by `EST_CSI_SGR_256_COLOR_*`.
- 6x6x6 color cube: indexes `16..231`, with each channel level in `0..5`; calculate an
  index with `EST_CSI_SGR_256_COLOR_CUBE_INDEX(r, g, b)`.
- Grayscale: indexes `232..255`; calculate an index with
  `EST_CSI_SGR_256_COLOR_GRAYSCALE_INDEX(v)`.

These macros compose or calculate color parameters but do not validate input ranges.
Use the provided `*_IN_RANGE` macros or validate inputs in the caller.

## Low-Level Sequence Builders

For a sequence without a semantic wrapper in the header, use the low-level builders:

```c
EST_CSI_STR("?25", "", "h")       /* CSI ?25h */
EST_OSC_STR("0;my title")          /* OSC 0;my title ST */
EST_DCS_STR("", ">", "|", "1.0")  /* DCS >|1.0 ST */
EST_APC_STR("payload")             /* APC payload ST */
EST_ST_STR()                        /* ST */
```

These builders accept string fragments. `EST_CSI_STR(P, I, F)` represents parameter,
intermediate, and final bytes; `EST_DCS_STR(P, I, F, D)` also takes a data section.
`EST_CSI_NOP_STR`, `EST_CSI_NOI_STR`, and `EST_CSI_NOPI_STR` omit unused sections.

## General Utility Macros

The header also includes preprocessor utilities unrelated to terminal sequences:

| Category | Main macros | Purpose |
| --- | --- | --- |
| Stringizing/concatenation | `ESCAPE_S`, `ESCAPE_C` | Expand before stringizing or paste tokens |
| Argument dispatch | `ESCAPE_X`, `ESCAPE_X_ARG_CNT` | Select a macro by argument count, currently 0 through 7 |
| Joining | `ESCAPE_JOIN`, `ESCAPE_JOINS` | Join string arguments, or stringify before joining |
| Ranges/numeric helpers | `ESCAPE_MIN`, `ESCAPE_MAX`, `ESCAPE_CLAMP`, `ESCAPE_IN_RANGE`, `ESCAPE_ROUND_DIV` | Common numeric operations |
| Diagnostics | `ESCAPE_ASSERT`, `ESCAPE_TODO` | Write a diagnostic to `stderr` and abort |

`ESCAPE_SAFE_*` variants first store their arguments, avoiding repeated evaluation. They
use the GNU statement-expression extension. The default aliases `ESCAPE_MIN`,
`ESCAPE_MAX`, and similar macros point to their `UNSAFE` variants, so expressions with
side effects require care.

## Compatibility and Limitations

1. This is a preprocessor-driven interface, not a runtime formatting library; macro
   arguments generally need to be expandable at compile time.
2. `ESCAPE_S` stringizes tokens; it cannot convert a runtime integer to text.
3. `ESCAPE_SAFE_*` and `ESCAPE_TYPEOF` depend on compiler support. GCC/Clang GNU C is
   the most reliable current environment. C++ additionally needs `decltype` support,
   while the safe macros still depend on the statement-expression extension.
4. Empty variadic-argument compatibility prefers `__VA_OPT__`, then uses the
   `##__VA_ARGS__` extension on GCC/Clang. Other compilers require a manual definition of
   the compatibility macro.
5. Terminal control sequences are not completely uniform across terminals. Test
   `CSI s/u`, margins, mouse modes, synchronized output, and query commands against the
   target terminal.
6. Whether colors work depends on the terminal and output environment. The library does
   not inspect `TERM` or `COLORTERM` and does not disable color automatically.
7. `ESCAPE_ASSERT` and `ESCAPE_TODO` write diagnostics to `stderr` and terminate the
  process with `abort()` when triggered.

## Verification and Project Status

### Recommended verification

Compile a minimal program that includes only `escape.h`:

```sh
cc -std=gnu11 -Wall -Wextra -pedantic -fsyntax-only demo.c
```

You can also print the generated sequence as hexadecimal bytes and confirm that it starts
with `0x1b` and contains the expected introducer such as `[`. Terminal rendering alone
is not a sufficient validation method.

### Files

| File | Status |
| --- | --- |
| [`escape.h`](escape.h) | Current implementation: utility macros, sequence types, and CSI builders |
| `README.md` | English project entry point and current API guide |
| `README.zh-CN.md` | Simplified Chinese project entry point |

The following interfaces are not part of the current API: OSC semantic wrappers, ESC
single-character wrappers, DCS semantic wrappers, runtime I/O, terminal capability
detection, and response parsing.

## References

- [ECMA-48](https://www.ecma-international.org/publications-and-standards/standards/ecma-48/): the foundational control-function encoding standard.
- [Xterm Control Sequences](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html): xterm extensions and terminal-specific behavior.
