#include "../lf/arg_pack.hpp"

#include "common.hpp"

#include <utility>

namespace arg_pack_impl {

auto cnt = 0;

struct stru {
  stru() noexcept { ++cnt; }
  stru(const stru&) noexcept { ++cnt; }
  stru(stru&&) noexcept { }
};

void func(stru, stru) {}

template <typename... Ts, std::size_t... Is,
          typename... Us, std::size_t... Js>
void func(LF_ARG_PACK(Ts, Is) ap1, LF_ARG_PACK(Us, Js) ap2) {
  assert(cnt == 4);
  func(LF_UNPACK_ARGS(ap1, Is));
  assert(cnt == 6);
  func(LF_UNPACK_ARGS(ap2, Js));
  assert(cnt == 7);
}

} // namespace arg_pack_impl

LF_TEST_BEGIN(arg_pack)
  using namespace arg_pack_impl;
  stru a, b; // 2
  const stru c, d; // 4
  assert(cnt == 4);
  func(lf::pack_args(a, c), lf::pack_args(std::move(b), std::move(d)));
  assert(cnt == 7);
LF_TEST_END
