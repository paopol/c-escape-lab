# Examples

Each file is a self-contained program that demonstrates one area of
[`escape.h`](../escape.h). Build them all with `make`, run with `make run`, or
build a single one with `make <name>`.

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
make          # build all examples
make run      # build and run all examples
make clean    # remove built files
```

Build with Clang via `make CC=clang`.
