# c-escape-lab example programs.
#
# Targets:
#   make             Build all examples.
#   make <name>      Build one example, e.g. `make colors`.
#   make run         Build and run all examples.
#   make run-<name>  Build and run one example, e.g. `make run-colors`.
#   make clean       Remove built files.
#
# Toolchain overrides:
#   make CC=clang
#   make CFLAGS="-std=c11 -O2"

CC       := gcc
CFLAGS   ?= -std=gnu11 -Wall -Wextra -pedantic
CPPFLAGS += -I..

# Executable suffix (.exe on Windows) and a shell-appropriate remove command.
ifeq ($(OS),Windows_NT)
    EXE := .exe
    RM  := del /Q
    RUN :=
else
    EXE :=
    RM  := rm -f
    RUN := ./
endif

DEMOS := demo colors cursor erase modes lowlevel
BINS  := $(addsuffix $(EXE), $(DEMOS))

.PHONY: all run clean

all: $(BINS)

$(BINS): %$(EXE): %.c ../escape.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

# On Windows, alias the bare name to the .exe so `make colors` keeps the
# ../escape.h dependency (instead of falling back to the built-in %.c rule).
ifeq ($(EXE),.exe)
    .PHONY: $(DEMOS)
    $(DEMOS): %: %$(EXE)
endif

# Build and run a single example: `make run-colors`.
run-%: %$(EXE)
	@echo === $* === && $(RUN)$<

run: $(addprefix run-,$(DEMOS))

clean:
	-$(RM) $(BINS)
