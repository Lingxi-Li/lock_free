## Header `lf/allocator.hpp`

This header provides a fixed-capacity allocator.
It implements a kind of memory pool that preallocates required memory resources from OS.
Allocations and deallocations from the allocator are then lock-free.

- [Synopsis](#synopsis)
- [Details](#details)

### Synopsis

~~~C++
template <typename T>
class allocator {
public:
  struct node {
    T val;
    std::atomic<node*> next;
  };

  allocator() noexcept = default;
  explicit allocator(std::size_t capacity);
  ~allocator();

  allocator(const allocator&) = delete;
  allocator& operator=(const allocator&) = delete;

  void reset(std::size_t capacity);
  template <typename F, typename... Args>
  void reset(std::size_t capacity, F&& f, Args&&... args);

  node* try_allocate() noexcept;
  void deallocate(node* p) noexcept;
};
~~~

### Details

~~~C++
struct node {
  T val;
  std::atomic<node*> next;
};
~~~

Specifies the node structure that is allocated/deallocated by the allocator.

`val` is the user payload.
The allocator does not manage its initialization/uninitialization.
Specifically, it is not initialized as part of node allocation,
and should be uninitialized by user before node deallocation.

`next` is used internally by the allocator, and is exposed for reuse opportunity by user.
Its initialization/uninitialization is managed by the allocator.
Specifically, it is initialized in an allocated node,
and does not need uninitialization before deallocating the node.

--------------------------------------------------------------------------------

~~~C++
allocator() noexcept = default;
~~~

Initializes a zero-capacity allocator.

--------------------------------------------------------------------------------

~~~C++
explicit allocator(std::size_t capacity);
~~~

Initializes an allocator of a specified capacity.
For non-zero capacity, preallocates required memory resources from OS.

--------------------------------------------------------------------------------

~~~C++
~allocator();
~~~

Deallocates allocated nodes, and returns preallocated memory resources to OS, if any.
It is unnecessary to deallocate allocated nodes before the destructor call.
However, user is still required to first uninitialize `val` in the allocated nodes.

--------------------------------------------------------------------------------

~~~C++
void reset(std::size_t capacity);

template <typename F, typename... Args>
void reset(std::size_t capacity, F&& f, Args&&... args);
~~~

Resets allocator capacity.

The first overload is semantically equivalent to uninitializing the current allocator,
and initializing a new one with the specified capacity.
Reseting to zero capacity returns preallocated memory resources to OS, if any.
`val` in allocated nodes should be uninitialized before the call.
The method itself provides strong exception safety.
However, the requirement of first uninitializing `val` may pose an issue.
For example, one may write the code

~~~C++
uninit_val(...);
alloc.reset(...);
~~~

If the `reset()` call threw, `uninit_val()` would have already been called.
The second overload is introduced to solve this issue.
What it does is similar to

~~~C++
std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
reset(capacity);
~~~

The invocation is intended to perform `val` uninitialization, and assumed to not throw.
The method then provides strong exception safety.
That is, if it threw, the invocation would not have happened.

Both overloads are non-thread-safe.

--------------------------------------------------------------------------------

~~~C++
node* try_allocate() noexcept;
~~~

Tries to allocate a node.
Returns `nullptr` on failure.
May fail spuriously.

--------------------------------------------------------------------------------

~~~C++
void deallocate(node* p) noexcept;
~~~

Returns node `p` to the allocator.
`p->next` should be uninitialized first.
