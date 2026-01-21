#pragma once

#include <optional>
#include <concepts>
#include <stdexcept>
#include <functional>
#include <utility>
#include <type_traits>
#include <string>
#include <tuple>

template<class T, class E>
class Result;

struct NoneTag {};
inline constexpr NoneTag none_tag{};

template<class T>
class Option {
    using value_type = T;

public:
    constexpr Option() noexcept : data_(std::nullopt) {}
    constexpr Option(NoneTag) noexcept : data_(std::nullopt) {}
    constexpr Option(T value) : data_(std::move(value)) {}
    constexpr Option(std::nullopt_t) noexcept : data_(std::nullopt) {}
    constexpr Option(std::optional<T> opt) : data_(std::move(opt)) {}

    [[nodiscard]] static constexpr auto some(T value) -> Option {
        return Option(std::move(value));
    }

    [[nodiscard]] static constexpr auto none() -> Option {
        return Option();
    }

    [[nodiscard]] constexpr auto is_some() const noexcept -> bool {
        return data_.has_value();
    }

    [[nodiscard]] constexpr auto is_none() const noexcept -> bool {
        return !data_.has_value();
    }

    template<std::predicate<const T&> P>
    [[nodiscard]] constexpr auto is_some_and(P&& pred) const -> bool {
        return is_some() && std::invoke(std::forward<P>(pred), *data_);
    }

    template<std::predicate<const T&> P>
    [[nodiscard]] constexpr auto is_none_or(P&& pred) const -> bool {
        return is_none() || std::invoke(std::forward<P>(pred), *data_);
    }

    [[nodiscard]] constexpr auto unwrap() && -> T {
        if (is_none()) throw std::runtime_error("called unwrap on None value");
        return std::move(*data_);
    }

    [[nodiscard]] constexpr auto unwrap() const& -> const T& {
        if (is_none()) throw std::runtime_error("called unwrap on None value");
        return *data_;
    }

    [[nodiscard]] constexpr auto expect(std::string_view msg) && -> T {
        if (is_none()) throw std::runtime_error(std::string(msg));
        return std::move(*data_);
    }

    [[nodiscard]] constexpr auto expect(std::string_view msg) const& -> const T& {
        if (is_none()) throw std::runtime_error(std::string(msg));
        return *data_;
    }

    [[nodiscard]] constexpr auto unwrap_or(T default_value) const& -> T {
        if (is_some()) return *data_;
        return default_value;
    }

    [[nodiscard]] constexpr auto unwrap_or(T default_value) && -> T {
        if (is_some()) return std::move(*data_);
        return default_value;
    }

    [[nodiscard]] constexpr auto unwrap_or_default() const& -> T
        requires std::default_initializable<T>
    {
        if (is_some()) return *data_;
        return T{};
    }

    [[nodiscard]] constexpr auto unwrap_or_default() && -> T
        requires std::default_initializable<T>
    {
        if (is_some()) return std::move(*data_);
        return T{};
    }

    template<std::invocable F>
        requires std::convertible_to<std::invoke_result_t<F>, T>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& f) const& -> T {
        if (is_some()) return *data_;
        return std::invoke(std::forward<F>(f));
    }

    template<std::invocable F>
        requires std::convertible_to<std::invoke_result_t<F>, T>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& f) && -> T {
        if (is_some()) return std::move(*data_);
        return std::invoke(std::forward<F>(f));
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto map(F&& f) && -> Option<std::invoke_result_t<F, T>> {
        using U = std::invoke_result_t<F, T>;
        if (is_some()) return Option<U>::some(std::invoke(std::forward<F>(f), std::move(*data_)));
        return Option<U>::none();
    }

    template<std::invocable<const T&> F>
    [[nodiscard]] constexpr auto map(F&& f) const& -> Option<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (is_some()) return Option<U>::some(std::invoke(std::forward<F>(f), *data_));
        return Option<U>::none();
    }

    template<class U, std::invocable<T> F>
        requires std::convertible_to<std::invoke_result_t<F, T>, U>
    [[nodiscard]] constexpr auto map_or(U default_value, F&& f) && -> U {
        if (is_some()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return default_value;
    }

    template<class U, std::invocable<const T&> F>
        requires std::convertible_to<std::invoke_result_t<F, const T&>, U>
    [[nodiscard]] constexpr auto map_or(U default_value, F&& f) const& -> U {
        if (is_some()) return std::invoke(std::forward<F>(f), *data_);
        return default_value;
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto map_or_default(F&& f) && -> std::invoke_result_t<F, T>
        requires std::default_initializable<std::invoke_result_t<F, T>>
    {
        using U = std::invoke_result_t<F, T>;
        if (is_some()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return U{};
    }

    template<std::invocable D, std::invocable<T> F>
        requires std::same_as<std::invoke_result_t<D>, std::invoke_result_t<F, T>>
    [[nodiscard]] constexpr auto map_or_else(D&& default_fn, F&& f) && -> std::invoke_result_t<F, T> {
        if (is_some()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return std::invoke(std::forward<D>(default_fn));
    }

    template<std::invocable D, std::invocable<const T&> F>
        requires std::same_as<std::invoke_result_t<D>, std::invoke_result_t<F, const T&>>
    [[nodiscard]] constexpr auto map_or_else(D&& default_fn, F&& f) const& -> std::invoke_result_t<F, const T&> {
        if (is_some()) return std::invoke(std::forward<F>(f), *data_);
        return std::invoke(std::forward<D>(default_fn));
    }

    template<class U>
    [[nodiscard]] constexpr auto operator&(Option<U> other) const -> Option<U> {
        if (is_some()) return other;
        return Option<U>::none();
    }

    template<std::invocable<T> F>
    [[nodiscard]] constexpr auto and_then(F&& f) && {
        using ResultOpt = std::invoke_result_t<F, T>;
        if (is_some()) return std::invoke(std::forward<F>(f), std::move(*data_));
        return ResultOpt{};
    }

    template<std::invocable<const T&> F>
    [[nodiscard]] constexpr auto and_then(F&& f) const& {
        using ResultOpt = std::invoke_result_t<F, const T&>;
        if (is_some()) return std::invoke(std::forward<F>(f), *data_);
        return ResultOpt{};
    }

    [[nodiscard]] constexpr auto operator|(Option other) const& -> Option {
        if (is_some()) return *this;
        return other;
    }

    [[nodiscard]] constexpr auto operator|(Option other) && -> Option {
        if (is_some()) return std::move(*this);
        return other;
    }

    template<std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, Option>
    [[nodiscard]] constexpr auto or_else(F&& f) const& -> Option {
        if (is_some()) return *this;
        return std::invoke(std::forward<F>(f));
    }

    template<std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, Option>
    [[nodiscard]] constexpr auto or_else(F&& f) && -> Option {
        if (is_some()) return std::move(*this);
        return std::invoke(std::forward<F>(f));
    }

    [[nodiscard]] constexpr auto operator^(Option other) && -> Option {
        if (is_some() && other.is_none()) return std::move(*this);
        if (is_none() && other.is_some()) return std::move(other);
        return Option::none();
    }

    template<std::predicate<const T&> P>
    [[nodiscard]] constexpr auto filter(P&& pred) && -> Option {
        if (is_some() && std::invoke(std::forward<P>(pred), *data_)) {
            return std::move(*this);
        }
        return Option::none();
    }

    template<std::predicate<const T&> P>
    [[nodiscard]] constexpr auto filter(P&& pred) const& -> Option {
        if (is_some() && std::invoke(std::forward<P>(pred), *data_)) {
            return *this;
        }
        return Option::none();
    }

    [[nodiscard]] constexpr auto flatten() &&
        requires requires { typename T::value_type; } &&
                 std::same_as<T, Option<typename T::value_type>>
    {
        using Inner = typename T::value_type;
        if (is_some()) return std::move(*data_);
        return Option<Inner>::none();
    }

    template<std::invocable<const T&> F>
    constexpr auto inspect(F&& f) const& -> const Option& {
        if (is_some()) std::invoke(std::forward<F>(f), *data_);
        return *this;
    }

    template<std::invocable<const T&> F>
    constexpr auto inspect(F&& f) && -> Option {
        if (is_some()) std::invoke(std::forward<F>(f), *data_);
        return std::move(*this);
    }

    constexpr auto insert(T value) -> T& {
        data_ = std::move(value);
        return *data_;
    }

    constexpr auto get_or_insert(T value) -> T& {
        if (is_none()) data_ = std::move(value);
        return *data_;
    }

    constexpr auto get_or_insert_default() -> T&
        requires std::default_initializable<T>
    {
        if (is_none()) data_ = T{};
        return *data_;
    }

    template<std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, T>
    constexpr auto get_or_insert_with(F&& f) -> T& {
        if (is_none()) data_ = std::invoke(std::forward<F>(f));
        return *data_;
    }

    [[nodiscard]] constexpr auto replace(T value) -> Option {
        auto old = std::move(*this);
        data_ = std::move(value);
        return old;
    }

    [[nodiscard]] constexpr auto take() -> Option {
        auto old = std::move(*this);
        data_ = std::nullopt;
        return old;
    }

    template<std::predicate<T&> P>
    [[nodiscard]] constexpr auto take_if(P&& pred) -> Option {
        if (is_some() && std::invoke(std::forward<P>(pred), *data_)) {
            return take();
        }
        return Option::none();
    }

    template<class U>
    [[nodiscard]] constexpr auto zip(Option<U> other) && -> Option<std::pair<T, U>> {
        if (is_some() && other.is_some()) {
            return Option<std::pair<T, U>>::some({std::move(*data_), std::move(other).unwrap()});
        }
        return Option<std::pair<T, U>>::none();
    }

    template<class U, std::invocable<T, U> F>
    [[nodiscard]] constexpr auto zip_with(Option<U> other, F&& f) && -> Option<std::invoke_result_t<F, T, U>> {
        using R = std::invoke_result_t<F, T, U>;
        if (is_some() && other.is_some()) {
            return Option<R>::some(std::invoke(std::forward<F>(f), std::move(*data_), std::move(other).unwrap()));
        }
        return Option<R>::none();
    }

    [[nodiscard]] constexpr auto unzip() &&
        requires requires { typename std::tuple_element_t<0, T>; typename std::tuple_element_t<1, T>; }
    {
        using A = std::tuple_element_t<0, T>;
        using B = std::tuple_element_t<1, T>;
        if (is_some()) {
            auto [a, b] = std::move(*data_);
            return std::pair{Option<A>::some(std::move(a)), Option<B>::some(std::move(b))};
        }
        return std::pair{Option<A>::none(), Option<B>::none()};
    }

    template<class E>
    [[nodiscard]] constexpr auto ok_or(E error) && -> Result<T, E>;

    template<std::invocable F>
    [[nodiscard]] constexpr auto ok_or_else(F&& f) && -> Result<T, std::invoke_result_t<F>>;

    [[nodiscard]] constexpr auto value() & -> T& {
        if (is_none()) throw std::runtime_error("called value on None");
        return *data_;
    }

    [[nodiscard]] constexpr auto value() const& -> const T& {
        if (is_none()) throw std::runtime_error("called value on None");
        return *data_;
    }

    [[nodiscard]] constexpr auto value() && -> T {
        if (is_none()) throw std::runtime_error("called value on None");
        return std::move(*data_);
    }

    [[nodiscard]] constexpr auto ptr() noexcept -> T* {
        if (is_some()) return &*data_;
        return nullptr;
    }

    [[nodiscard]] constexpr auto ptr() const noexcept -> const T* {
        if (is_some()) return &*data_;
        return nullptr;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return is_some();
    }

    [[nodiscard]] constexpr auto operator==(const Option& other) const -> bool
        requires std::equality_comparable<T>
    {
        return data_ == other.data_;
    }

    [[nodiscard]] constexpr auto operator!=(const Option& other) const -> bool
        requires std::equality_comparable<T>
    {
        return data_ != other.data_;
    }

    [[nodiscard]] constexpr auto operator==(std::nullopt_t) const noexcept -> bool {
        return is_none();
    }

    [[nodiscard]] constexpr auto operator<=>(const Option& other) const
        requires std::three_way_comparable<T>
    {
        return data_ <=> other.data_;
    }

    [[nodiscard]] constexpr auto as_std_optional() const& -> const std::optional<T>& {
        return data_;
    }

    [[nodiscard]] constexpr auto as_std_optional() && -> std::optional<T> {
        return std::move(data_);
    }

private:
    std::optional<T> data_;

    template<class>
    friend class Option;
};

template<class T>
Option(T) -> Option<T>;

inline constexpr auto None = none_tag;

template<class T>
[[nodiscard]] constexpr auto Some(T value) -> Option<T> {
    return Option<T>::some(std::move(value));
}
