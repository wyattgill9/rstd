#pragma once

#include <format>
#include <stdexcept>
#include <string_view>
#include <string>

#include "macros.hh"

namespace rstd {

[[noreturn]] constexpr fn expect_failed(std::string_view msg) noexcept(false) {
    throw std::runtime_error(std::string(msg));
}

[[noreturn]] constexpr fn unwrap_failed(std::string_view msg) noexcept(false) {
    throw std::runtime_error(std::string(msg));
}

template<typename E>
[[noreturn]] constexpr fn unwrap_failed(std::string_view msg, const E& e) noexcept(false) {
    throw std::runtime_error(std::format("{1} {0}", msg, e));
}

};
