# utility

This header provides general utilities.

## Synopsis

~~~C++
// Shorthand of memory order semantics
constexpr auto rlx = std::memory_order_relaxed;
constexpr auto rel = std::memory_order_release;
constexpr auto acq = std::memory_order_acquire;
constexpr auto eat = std::memory_order_consume;
constexpr auto cst = std::memory_order_seq_cst;
constexpr auto acq_rel = std::memory_order_acq_rel;

// Typed operator new().
template <typename T>
T* allocate();

// Typed no-throw operator new().
template <typename T>
T* try_allocate() noexcept;

// Alias of operator delete().
inline
void deallocate(void* p) noexcept;

// Checks valid range and gets its size.
template <typename FwdIt>
std::size_t range_extent(FwdIt first, FwdIt last);

// Constructs T(std::forward<Us>(us)...) at p.
template <typename T, typename... Us>
void construct(T* p, Us&&... us);
~~~
