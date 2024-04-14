# tscpp

**tscpp** is an alternative TypeScript compiler.

It currently transpiles to C++.

## Build instructions

This project is only tested to compile with the `clang` version 16 on macOS.
You will need to install it along with `meson` and `ninja`. For faster
rebuilds you may install `ccache` as well.

### Setup:

    meson setup build

### Build:

    ninja -C build

After building, the `tscpp` executable will be found in `./build/src`.

### Usage:

    ./meta/compile tests/hello-world.ts
    ./a.out

## Goals

1. Compatibility with TypeScript
2. Executable performance
3. Compile performance
4. Fun!

## Compatibility

`tscpp` wants to be able to compile more or less any TypeScript file into
binary. This is a high goal post, but it should theoretically be possible,
so why not aim for the stars?

## Executable performance

JavaScript programs are generally quite slow in comparison to those written in
systems programming languages like C, C++, Rust etc. However, TypeScript has
type annotations, it just doesn't optimize the code around them. What if we were
to optimize generated code around those type annotations, would the language still
be slow?

## Compile performance

Many modern web projects take ages to compile, for some projects it may
take even more time to compile than for modern C++ projects. `tscpp` aims
to be the fastest TypeScript compiler.
