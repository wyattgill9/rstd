#pragma once

#include <expected>
#include <concepts>
#include <stdexcept>
#include <functional>
#include <utility>
#include <type_traits>
#include <string>

template<class T>
class Option;

struct NoneTag;

template<class T, class E>
class Result {
    using value_type = T;
    using error_type = E;

public:
    constexpr Result() = delete;

    constexpr Result(std::expected<T, E> exp) : data_(std::move(exp)) {}

    template<class U>
        requires std::convertible_to<U, T>
    constexpr Result(std::expected<U, E> exp) : data_(exp.has_value()
        ? std::expected<T, E>(std::in_place, static_cast<T>(*exp))
        : std::expected<T, E>(std::unexpect, exp.error())) {}

    [[nodiscard]] static constexpr auto ok(T value) -> Result {
        return Result(std::expected<T, E>(std::in_place, std::move(value)));
    }

    [[nodiscard]] static constexpr auto err(E error) -> Result {
        return Result(std::expected<T, E>(std::unexpect, std::move(error)));
    }

    [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
        return data_.has_value();
    }

    [[nodiscard]] constexpr auto is_err() const noexcept -> bool {
        return !data_.has_value();
    }

    template<std::predicate<const T&> P>
    [[nodiscard]] constexpr auto is_ok_and(P&& pred) const -> bool {
        return is_ok() && std::invoke(std::forward<P>(pred), *data_);
    }

    template<std::predicate<const E&> P>
    [[nodiscard]] constexpr auto is_err_and(P&& pred) const -> bool {
        return is_err() && std::invoke(std::forward<P>(pred), data_.error());
    }

    [[nodiscard]] constexpr auto ok_value() && -> Option<T>;
    [[nodiscard]] constexpr auto err_value() && -> Option<E>;

    [[nodiscard]] constexpr auto unwrap() && -> T {
        if (is_err()) throw std::runtime_error("called unwrap on Err value");
        return std::move(*data_);
    }

    [[nodiscard]] constexpr auto unwrap() const& -> const T& {
        if (is_err()) throw std::runtime_error("called unwrap on Err value");
        return *data_;
    }

    [[nodiscard]] constexpr auto unwrap_err() && -> E {
        if (is_ok()) throw std::runtime_error("called unwrap_err on Ok value");
        return std::move(data_.error());
    }

    [[nodiscard]] constexpr auto unwrap_err() const& -> const E& {
        if (is_ok()) throw std::runtime_error("called unwrap_err on Ok value");
        return data_.error();
    }

    [[nodiscard]] constexpr auto expect(std::string_view msg) && -> T {
        if (is_err()) throw std::runtime_error(std::string(msg));
        return std::move(*data_);
    }

    [[nodiscard]] constexpr auto expect(std::string_view msg) const& -> const T& {
        if (is_err()) throw std::runtime_error(std::string(msg));
        return *data_;
    }

    [[nodiscard]] constexpr auto expect_err(std::string_view msg) && -> E {
        if (is_ok()) throw std::runtime_error(std::string(msg));
        return std::move(data_.error());
    }

    [[nodiscard]] constexpr auto expect_err(std::string_view msg) const& -> const E& {
        if (is_ok()) throw std::runtime_error(std::string(msg));
        return data_.error();
    }

    [[nodiscard]] constexpr auto unwrap_or(T default_value) const& -> T {
        if (is_ok()) return *data_;
        return default_value;
    }

    [[nodiscard]] constexpr auto unwrap_or(T default_value) && -> T {
        if (is_ok()) return std::move(*data_);
        return default_value;
    }

    [[nodiscard]] constexpr auto unwrap_or_default() const& -> T
        requires std::default_initializable<T>
    {
        if (is_ok()) return *data_;
        return T{};
    }

    [[nodiscard]] constexpr auto unwrap_or_default() && -> T
        requires std::default_initializable<T>
    {
        if (is_ok()) return std::move(*data_);
        return T{};
    }

    template<std::invocable<E> F>
        requires std::convertible_to<std::invoke_result_t<F, E>, T>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& f) const& -> T {
        if (is_ok()) return *data_;
        return std::invoke(std::forward<F>(f), data_.error());
    }

    template<std::invocable<E> F>
        requires std::convertible_to<std::invoke_result_t<F, E>, T>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& f) && -> T {
        if (is_ok()) return std::move(*data_);
        return std::invoke(std::forward<F>(f), std::move(data_.error()));
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto map(F&& f) && -> Result<std::invoke_result_t<F, T>, E> {
        using U = std::invoke_result_t<F, T>;
        if (is_ok()) return Result<U, E>::ok(std::invoke(std::forward<F>(f), std::move(*data_)));
        return Result<U, E>::err(std::move(data_.error()));
    }

    template<std::invocable<const T&> F>
    [[nodiscard]] constexpr auto map(F&& f) const& -> Result<std::invoke_result_t<F, const T&>, E> {
        using U = std::invoke_result_t<F, const T&>;
        if (is_ok()) return Result<U, E>::ok(std::invoke(std::forward<F>(f), *data_));
        return Result<U, E>::err(data_.error());
    }

    template<std::invocable<E> F>
    [[nodiscard]] constexpr auto map_err(F&& f) && -> Result<T, std::invoke_result_t<F, E>> {
        using F2 = std::invoke_result_t<F, E>;
        if (is_err()) return Result<T, F2>::err(std::invoke(std::forward<F>(f), std::move(data_.error())));
        return Result<T, F2>::ok(std::move(*data_));
    }

    template<std::invocable<const E&> F>
    [[nodiscard]] constexpr auto map_err(F&& f) const& -> Result<T, std::invoke_result_t<F, const E&>> {
        using F2 = std::invoke_result_t<F, const E&>;
        if (is_err()) return Result<T, F2>::err(std::invoke(std::forward<F>(f), data_.error()));
        return Result<T, F2>::ok(*data_);
    }

    template<class U, std::invocable<T> F>
        requires std::convertible_to<std::invoke_result_t<F, T>, U>
    [[nodiscard]] constexpr auto map_or(U default_value, F&& f) && -> U {
        if (is_ok()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return default_value;
    }

    template<class U, std::invocable<const T&> F>
        requires std::convertible_to<std::invoke_result_t<F, const T&>, U>
    [[nodiscard]] constexpr auto map_or(U default_value, F&& f) const& -> U {
        if (is_ok()) return std::invoke(std::forward<F>(f), *data_);
        return default_value;
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto map_or_default(F&& f) && -> std::invoke_result_t<F, T>
        requires std::default_initializable<std::invoke_result_t<F, T>>
    {
        using U = std::invoke_result_t<F, T>;
        if (is_ok()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return U{};
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto map_or_default(F&& f) const& -> std::invoke_result_t<F, const T&>
        requires std::default_initializable<std::invoke_result_t<F, const T&>>
    {
        using U = std::invoke_result_t<F, const T&>;
        if (is_ok()) return std::invoke(std::forward<F>(f), *data_);
        return U{};
    }

    template<std::invocable<E> D, std::invocable<T> F>
        requires std::same_as<std::invoke_result_t<D, E>, std::invoke_result_t<F, T>>
    [[nodiscard]] constexpr auto map_or_else(D&& default_fn, F&& f) && -> std::invoke_result_t<F, T> {
        if (is_ok()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return std::invoke(std::forward<D>(default_fn), std::move(data_.error()));
    }

    template<std::invocable<const E&> D, std::invocable<const T&> F>
        requires std::same_as<std::invoke_result_t<D, const E&>, std::invoke_result_t<F, const T&>>
    [[nodiscard]] constexpr auto map_or_else(D&& default_fn, F&& f) const& -> std::invoke_result_t<F, const T&> {
        if (is_ok()) return std::invoke(std::forward<F>(f), *data_);
        return std::invoke(std::forward<D>(default_fn), data_.error());
    }

    template<class U>
    [[nodiscard]] constexpr auto operator&(Result<U, E> other) const& -> Result<U, E> {
        if (is_ok()) return other;
        return Result<U, E>::err(data_.error());
    }

    template<class U>
    [[nodiscard]] constexpr auto operator&(Result<U, E> other) && -> Result<U, E> {
        if (is_ok()) return other;
        return Result<U, E>::err(std::move(data_.error()));
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto and_then(F&& f) && {
        using ResultType = std::invoke_result_t<F, T>;
        if (is_ok()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return ResultType::err(std::move(data_.error()));
    }

    template<std::invocable<const T&> F>
    [[nodiscard]] constexpr auto and_then(F&& f) const& {
        using ResultType = std::invoke_result_t<F, const T&>;
        if (is_ok()) return std::invoke(std::forward<F>(f), *data_);
        return ResultType::err(data_.error());
    }

    template<class F2>
    [[nodiscard]] constexpr auto operator|(Result<T, F2> other) const& -> Result<T, F2> {
        if (is_ok()) return Result<T, F2>::ok(*data_);
        return other;
    }

    template<class F2>
    [[nodiscard]] constexpr auto operator|(Result<T, F2> other) && -> Result<T, F2> {
        if (is_ok()) return Result<T, F2>::ok(std::move(*data_));
        return other;
    }

    template<std::invocable<E> F>
    [[nodiscard]] constexpr auto or_else(F&& f) && {
        using ResultType = std::invoke_result_t<F, E>;
        if (is_ok()) return ResultType::ok(std::move(*data_));
        return std::invoke(std::forward<F>(f), std::move(data_.error()));
    }

    template<std::invocable<const E&> F>
    [[nodiscard]] constexpr auto or_else(F&& f) const& {
        using ResultType = std::invoke_result_t<F, const E&>;
        if (is_ok()) return ResultType::ok(*data_);
        return std::invoke(std::forward<F>(f), data_.error());
    }

    template<std::invocable<const T&> F>
    constexpr auto inspect(F&& f) const& -> const Result& {
        if (is_ok()) std::invoke(std::forward<F>(f), *data_);
        return *this;
    }

    template<std::invocable<const T&> F>
    constexpr auto inspect(F&& f) && -> Result {
        if (is_ok()) std::invoke(std::forward<F>(f), *data_);
        return std::move(*this);
    }

    template<std::invocable<const E&> F>
    constexpr auto inspect_err(F&& f) const& -> const Result& {
        if (is_err()) std::invoke(std::forward<F>(f), data_.error());
        return *this;
    }

    template<std::invocable<const E&> F>
    constexpr auto inspect_err(F&& f) && -> Result {
        if (is_err()) std::invoke(std::forward<F>(f), data_.error());
        return std::move(*this);
    }

    [[nodiscard]] constexpr auto flatten() &&
        requires requires { typename T::value_type; typename T::error_type; } &&
                 std::same_as<typename T::error_type, E>
    {
        using Inner = typename T::value_type;
        if (is_ok()) return std::move(*data_);
        return Result<Inner, E>::err(std::move(data_.error()));
    }

    template<class U = T>
        requires requires { typename U::value_type; }
    [[nodiscard]] constexpr auto transpose() && -> Option<Result<typename U::value_type, E>>;

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return is_ok();
    }

    [[nodiscard]] constexpr auto operator==(const Result& other) const -> bool
        requires std::equality_comparable<T> && std::equality_comparable<E>
    {
        return data_ == other.data_;
    }

    [[nodiscard]] constexpr auto operator!=(const Result& other) const -> bool
        requires std::equality_comparable<T> && std::equality_comparable<E>
    {
        return data_ != other.data_;
    }

    [[nodiscard]] constexpr auto as_std_expected() const& -> const std::expected<T, E>& {
        return data_;
    }

    [[nodiscard]] constexpr auto as_std_expected() && -> std::expected<T, E> {
        return std::move(data_);
    }

private:
    std::expected<T, E> data_;

    template<class, class>
    friend class Result;
};

template<class T>
struct OkValue {
    T value;

    template<class E>
    [[nodiscard]] constexpr operator Result<T, E>() const& {
        return Result<T, E>::ok(value);
    }

    template<class E>
    [[nodiscard]] constexpr operator Result<T, E>() && {
        return Result<T, E>::ok(std::move(value));
    }
};

template<class E>
struct ErrValue {
    E error;

    template<class T>
    [[nodiscard]] constexpr operator Result<T, E>() const& {
        return Result<T, E>::err(error);
    }

    template<class T>
    [[nodiscard]] constexpr operator Result<T, E>() && {
        return Result<T, E>::err(std::move(error));
    }
};

template<class T>
[[nodiscard]] constexpr auto Ok(T value) -> OkValue<T> {
    return OkValue<T>{std::move(value)};
}

template<class E>
[[nodiscard]] constexpr auto Err(E error) -> ErrValue<E> {
    return ErrValue<E>{std::move(error)};
}

#include "option.hh"

template<class T, class E>
constexpr auto Result<T, E>::ok_value() && -> Option<T> {
    if (is_ok()) return Option<T>::some(std::move(*data_));
    return Option<T>::none();
}

template<class T, class E>
constexpr auto Result<T, E>::err_value() && -> Option<E> {
    if (is_err()) return Option<E>::some(std::move(data_.error()));
    return Option<E>::none();
}

template<class T, class E>
template<class U>
    requires requires { typename U::value_type; }
constexpr auto Result<T, E>::transpose() && -> Option<Result<typename U::value_type, E>> {
    using Inner = typename U::value_type;
    if (is_ok()) {
        if (data_->is_some()) {
            return Option<Result<Inner, E>>::some(Result<Inner, E>::ok(std::move(*data_).unwrap()));
        }
        return Option<Result<Inner, E>>::none();
    }
    return Option<Result<Inner, E>>::some(Result<Inner, E>::err(std::move(data_.error())));
}

template<class T>
template<class E>
constexpr auto Option<T>::ok_or(E error) && -> Result<T, E> {
    if (is_some()) return Result<T, E>::ok(std::move(*data_));
    return Result<T, E>::err(std::move(error));
}

template<class T>
template<std::invocable F>
constexpr auto Option<T>::ok_or_else(F&& f) && -> Result<T, std::invoke_result_t<F>> {
    using E = std::invoke_result_t<F>;
    if (is_some()) return Result<T, E>::ok(std::move(*data_));
    return Result<T, E>::err(std::invoke(std::forward<F>(f)));
}
