#ifndef LF_ARG_PACK_HPP
#define LF_ARG_PACK_HPP

#include <tuple>
#include <utility>

#define LF_ARG_PACK(Ts, Is) ::lf::arg_pack_impl::arg_pack<::std::tuple<Ts...>, ::std::index_sequence<Is...>>
#define LF_UNPACK(args, Is) ::std::get<Is>(::std::move(args.vs))...

namespace lf {

namespace arg_pack_impl {

template <typename Tuple, typename Indices>
struct arg_pack {
  Tuple vs;
  Indices is;
};

} // namespace arg_pack_impl

template <typename... Ts>
auto pack(Ts&&... args) noexcept {
  std::tuple<Ts&&...> vs(std::forward<Ts>(args)...);
  std::index_sequence_for<Ts...> is;
  return arg_pack_impl::arg_pack<decltype(vs), decltype(is)>{ std::move(vs), is };
}

} // namespace lf

#endif // LF_ARG_PACK_HPP
