# tscpp

**tscpp** is a TypeScript to C++ transpiler

## Build instructions

### Setup:

    meson setup build

### Build:

    ninja -C build

### Run:

    ./build/src/tscpp tests/hello-world.ts -o - | clang-format
