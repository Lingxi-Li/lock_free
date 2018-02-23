# A C++14 Lock-Free Data Structure Library

This is a C++14 header-only template library of lock-free data structures.

## Requirements
- Tested C++14 implementations
  - Apple LLVM version 9.0.0 (clang-900.0.39.2) with `-std=c++14`
- 64-bit pointer because of the [atomic padding issue](https://stackoverflow.com/q/48947428/1348273)
- 128-bit atomic operation support for the data structures to be truely lock-free

## Repo Structure
~~~
lf/............Library header files
.gitignore.....Setup for vscode
README.md
~~~
