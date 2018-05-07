## Header `lf/allocator.hpp`

This header provides a fixed-capacity allocator.

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

Specifies the node structure that is allocated/deallocated.

`val` is user payload.
Its initialization/uninitialization is not managed by allocator.
Specifically, it is not initialized as part of node allocation,
and should be uninitialized by user before deallocating the node.

`next` is used internally by the allocator, and is exposed for reuse opportunity by user.
It is initialized in an allocated node,
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

--------------------------------------------------------------------------------

~~~C++
~allocator();
~~~

Automatically deallocates all allocated nodes.
It is therefore unnecessary to manually deallocate allocated nodes before the destructor call.
Note that user is still required to uninitialize `val` in the allocated nodes.

--------------------------------------------------------------------------------

~~~C++
void reset(std::size_t capacity);
~~~

Resets capacity of the allocator.
This is semantically equivalent to uninitializing the current allocator,
and initializing a new one with the specified capacity.
Not thread-safe.
Provides strong exception safety.

--------------------------------------------------------------------------------

~~~C++
node* try_allocate() noexcept;
~~~

Tries to allocate a new node.
Returns `nullptr` if failed.
May fail spuriously.

--------------------------------------------------------------------------------

~~~C++
void deallocate(node* p) noexcept;
~~~

Returns node `p` to the allocator.
`p->next` should be uninitialized before the call.
