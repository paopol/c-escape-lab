# Examples

Each file is a self-contained program that demonstrates one area of
[`escape.h`](../escape.h). Build all with `make`, run all with `make run`, or
target one with `make <name>` / `make run-<name>`.

| File | Demonstrates |
| --- | --- |
| [`demo.c`](demo.c) | Overview: title, styles, base-16 / 256 / truecolor |
| [`colors.c`](colors.c) | Text styles and colors (SGR, 16 / 256 / truecolor) |
| [`cursor.c`](cursor.c) | Cursor movement, positioning, save / restore |
| [`erase.c`](erase.c) | Erasing, editing, and scrolling |
| [`modes.c`](modes.c) | Mode control (SM / RM / DECRQM) and device-status queries |
| [`lowlevel.c`](lowlevel.c) | Low-level builders (ESC / CSI / OSC / DCS / APC / ST) |

## Build and run

```sh
make              # build all examples
make <name>       # build one example, e.g. `make colors`
make run          # build and run all examples
make run-<name>   # build and run one example, e.g. `make run-colors`
make clean        # remove built files
```

Build with Clang via `make CC=clang`.
