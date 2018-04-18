## Reference

### Conventions

- Constructors/destructors are not thread-safe.
  The destructing thread should be the last and only referencing thread.
- All components reside in namespace `lf`
  which is omitted in this Reference for brevity.
- Components in header, say `lf/foo/bar.hpp`, reside in namespace `lf::foo`.
- Concerning exception safety, assumes that move and
  swap operations do not throw.

The library provides ready-to-use lock-free [data structures](#data-structures)
as well as low-level [utilities](#utilities) to help build your own.

### Data Structures

- [Stack](lf/stack.md#header-lfstackhpp)

### Utilities

- [Memory](lf/memory.md#header-lfmemoryhpp)
- [Split Reference Counts](lf/split_ref.md#header-lfsplit_refhpp)
- [Utility](lf/utility.md#header-lfutilityhpp)
