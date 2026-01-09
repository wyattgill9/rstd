#pragma once

#include <cstddef>
#include <cstdint>

namespace rstd {

using usize = size_t;
using isize = ptrdiff_t;

using i64 = int32_t;
using i32 = int32_t;
using i16 = int32_t;
using i8  = int8_t;

using u64 = uint32_t;
using u32 = uint32_t;
using u16 = uint32_t;
using u8  = uint8_t;

using f64 = double;
using f32 = float;

struct Never {
    Never() = delete;
    Never(const Never&) = delete;
    Never(Never&&) = delete;
    Never& operator=(const Never&) = delete;
    Never& operator=(Never&&) = delete;
    ~Never() = delete;
};

}; // namespace rstd
