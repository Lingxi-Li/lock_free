# A C++17 Lock-Free Data Structure Library

This is a library for lock-free data structures with the following features.

- Written purely in standard C++17 with no platform-dependent code.
- Header-only.
- Designed for low worst-case latency.
- Designed for exception safety.
- Provides first-class support, like emplace construction, for aggregate types.

### Contents

- [Requirements](#requirements)
- [Repository Structure](#repository-structure)
- [Build](#build)
- [Unit Test](#unit-test)
- [Reference Doc](#reference-doc)

### Requirements

- A decent C++17 implementation. Tested:
  - MacPorts clang 6.0.0
  - MacPorts gcc 7.3.0
- 64-bit pointer to avoid [padding atomic][4].
- [Lock-free 128-bit atomic](#build).
- [Catch2][6] for unit test (contained).

### Repository Structure

~~~
doc.............Reference doc
lf..............Library headers
test
  unit_test
~~~

### Build

The C++ standard does not require atomics, other than [`std::atomic_flag`][8], to be lock-free.
Lock-free 128-bit atomic, in particular, demands both CPU and C++ implementation support,
plus proper build setup.

- All modern x86-64 CPUs provide support<sup>[[r][1]]</sup>.
- C++ implementation supportability
  - MacPorts clang 6.0.0 is verified to provide support.
  - Supportability is unclear for MacPorts gcc 7.3.0.
  - It is [said][1] that MSVC does not provide support.
- For clang/gcc, compile with [`-mcx16`][3].

[Here][2] are some notes for verifying a genuine lock-free build.
That said, [`is_lock_free()`][5] test is unreliable in practice.

### Unit Test

Build all source files in `test/unit_test` to get the unit test executable.
For example, using MacPorts clang 6.0.0, the following command line
builds the test executable `bin/unit_test`.

~~~
clang++-mp-6.0 test/unit_test/*.cpp -o bin/unit_test -std=c++17 -mcx16
~~~

Note that gcc may additionally need `-latomic`.

The test executable uses Catch2 and supports various [command line arguments][7].
There is a dedicated test case named `foo bar` for each header `lf/foo/bar.hpp`.
Executing with no argument runs all test cases.

### Reference Doc

The library is still under early development.
Reference for finished components can be found [here](doc/readme.md#reference).

Planned lock-free structures:

- [X] Stack
- [ ] Queue
- [ ] Atomic shared pointer
- [ ] Thread pool

- [ ] Fixed-capacity allocator
- [ ] Fixed-capacity stack
- [ ] Fixed-capacity queue
- [ ] Fixed-capacity thread pool

[1]:https://stackoverflow.com/a/38991835/1348273
[2]:https://stackoverflow.com/q/49848793/1348273
[3]:https://gcc.gnu.org/onlinedocs/gcc-7.3.0/gcc/x86-Options.html#x86-Options
[4]:https://stackoverflow.com/q/48947428/1348273
[5]:http://en.cppreference.com/w/cpp/atomic/atomic/is_lock_free
[6]:https://github.com/catchorg/Catch2/blob/master/README.md#top
[7]:https://github.com/catchorg/Catch2/blob/master/docs/command-line.md#top
[8]:http://en.cppreference.com/w/cpp/atomic/atomic_flag
