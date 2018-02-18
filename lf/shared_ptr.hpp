namespace lf {

template <typename T>
class shared_ptr {
public:
  // copy control
  shared_ptr(const shared_ptr& p);
  shared_ptr(shared_ptr&& p) noexcept;
 ~shared_ptr();
  shared_ptr& operator=(shared_ptr p) noexcept;
  friend void swap(shared_ptr& a, shared_ptr& b) noexcept;

  // construct
  shared_ptr();
  shared_ptr(T* p);

  // modifier
  void reset(T* p = nullptr);

  // observer
  T* get() const;
  T& operator*() const;
  T* operator->() const;
  explicit operator bool() const;
};

} // namespace lf
