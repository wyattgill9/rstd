#pragma once

#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "macros.hh"

namespace rstd {

template <typename T, typename E>
class Result;

template<typename T>
class Option;

struct None_t {
    constexpr explicit None_t() = default;
};
static inline constexpr None_t None{};

template<typename T>
constexpr inline fn Some(const T& val) -> Option<T> {
    return Option<T>(val);
}

template<typename T>
constexpr inline fn Some(T&& val) -> Option<T> {
    return Option<T>(std::forward<T>(val));
}

template <typename T>
class Option {
private:
    alignas(T) unsigned char storage_[sizeof(T)];
    bool has_value_;

    constexpr fn ptr() -> T* {
        return std::launder(reinterpret_cast<T*>(storage_));
    }

    constexpr fn ptr() const -> const T* {
        return std::launder(reinterpret_cast<const T*>(storage_));
    }

    constexpr fn destroy() -> void {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }

public:
    // Constructors
    constexpr Option() noexcept : has_value_(false) {}

    constexpr Option(None_t) noexcept : has_value_(false) {}

    constexpr Option(T&& val) : has_value_(true) {
        new (storage_) T(std::move(val));
    }

    constexpr Option(const T& val) : has_value_(true) {
        new (storage_) T(val);
    }

    // Copy constructor
    constexpr Option(const Option& other) : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(*other.ptr());
        }
    }

    // Move constructor
    constexpr Option(Option&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(std::move(*other.ptr()));
            other.destroy();
        }
    }

    // Destructor
    constexpr ~Option() { destroy(); }

    // Copy assignment
    constexpr fn operator=(const Option& other) -> Option& {
        if (this != &other) {
            destroy();
            has_value_ = other.has_value_;
            if (has_value_) {
                new (storage_) T(*other.ptr());
            }
        }
        return *this;
    }

    // Move assignment
    constexpr fn operator=(Option&& other) noexcept -> Option& {
        if (this != &other) {
            destroy();
            has_value_ = other.has_value_;
            if (has_value_) {
                new (storage_) T(std::move(*other.ptr()));
                other.destroy();
            }
        }
        return *this;
    }

    // Assignment from None
    constexpr fn operator=(None_t) noexcept -> Option& {
        destroy();
        return *this;
    }

    /// Returns true if the option is a Some value.
    [[nodiscard]] constexpr fn is_some() const noexcept -> bool {
        return has_value_;
    }

    /// Returns true if the option is a None value.
    [[nodiscard]] constexpr fn is_none() const noexcept -> bool {
        return !has_value_;
    }

    /// Returns true if the option is Some and the value matches a predicate.
    template <typename F>
        requires std::predicate<F, const T&>
    [[nodiscard]] constexpr fn is_some_and(F&& f) const -> bool {
        return has_value_ && std::invoke(std::forward<F>(f), *ptr());
    }

    /// Returns true if the option is None or the value matches a predicate.
    template <typename F>
        requires std::predicate<F, const T&>
    [[nodiscard]] constexpr fn is_none_or(F&& f) const -> bool {
        return !has_value_ || std::invoke(std::forward<F>(f), *ptr());
    }

    /// Converts from Option<T>& to Option<const T*>.
    [[nodiscard]] constexpr fn as_ref() const noexcept -> Option<const T*> {
        if (has_value_) {
            return Some(ptr());
        }
        return None;
    }

    /// Converts from Option<T>& to Option<T*>.
    [[nodiscard]] constexpr fn as_mut() noexcept -> Option<T*> {
        if (has_value_) {
            return Some(ptr());
        }
        return None;
    }

    /// Returns the contained Some value, consuming the self value.
    /// Throws if the value is None.
    [[nodiscard]] constexpr fn expect(const char* msg) && -> T {
        if (UNLIKELY(!has_value_)) {
            throw std::runtime_error(msg);
        }
        T val = std::move(*ptr());
        destroy();
        return val;
    }

    /// Returns the contained Some value, consuming the self value.
    /// Throws if the value is None.
    [[nodiscard]] constexpr fn unwrap() && -> T {
        return std::move(*this).expect("called `Option::unwrap()` on a `None` value");
    }

    /// Returns the contained Some value or a provided default.
    template <typename U>
        requires std::convertible_to<U, T>
    [[nodiscard]] constexpr fn unwrap_or(U&& default_val) && -> T {
        if (has_value_) {
            T val = std::move(*ptr());
            destroy();
            return val;
        }
        return static_cast<T>(std::forward<U>(default_val));
    }

    /// Returns the contained Some value or computes it from a closure.
    template <typename F>
        requires std::invocable<F> && std::convertible_to<std::invoke_result_t<F>, T>
    [[nodiscard]] constexpr fn unwrap_or_else(F&& f) && -> T {
        if (has_value_) {
            T val = std::move(*ptr());
            destroy();
            return val;
        }
        return std::invoke(std::forward<F>(f));
    }

    /// Returns the contained Some value or a default.
    [[nodiscard]] constexpr fn unwrap_or_default() && -> T
        requires std::default_initializable<T>
    {
        if (has_value_) {
            T val = std::move(*ptr());
            destroy();
            return val;
        }
        return T{};
    }

    /// Returns the contained Some value without checking.
    /// Undefined behavior if the value is None.
    [[nodiscard]] constexpr fn unwrap_unchecked() && noexcept -> T {
        T val = std::move(*ptr());
        destroy();
        return val;
    }

    /// Maps an Option<T> to Option<U> by applying a function.
    template <typename F>
        requires std::invocable<F, T>
    [[nodiscard]] constexpr fn map(F&& f) && -> Option<std::invoke_result_t<F, T>> {
        using U = std::invoke_result_t<F, T>;
        if (has_value_) {
            U result = std::invoke(std::forward<F>(f), std::move(*ptr()));
            destroy();
            return Some(std::move(result));
        }
        return None;
    }

    /// Calls a function with a reference to the contained value if Some.
    template <typename F>
        requires std::invocable<F, const T&>
    [[nodiscard]] constexpr fn inspect(F&& f) && -> Option {
        if (has_value_) {
            std::invoke(std::forward<F>(f), *ptr());
        }
        return std::move(*this);
    }

    /// Returns the provided default result or applies a function.
    template <typename U, typename F>
        requires std::invocable<F, T> && std::convertible_to<std::invoke_result_t<F, T>, U>
    [[nodiscard]] constexpr fn map_or(U&& default_val, F&& f) && -> U {
        if (has_value_) {
            U result = std::invoke(std::forward<F>(f), std::move(*ptr()));
            destroy();
            return result;
        }
        return std::forward<U>(default_val);
    }

    /// Applies a function or computes a default.
    template <typename D, typename F>
        requires std::invocable<D> && std::invocable<F, T>
    [[nodiscard]] constexpr fn map_or_else(D&& default_fn, F&& f) &&
        -> std::common_type_t<std::invoke_result_t<D>, std::invoke_result_t<F, T>> {
        if (has_value_) {
            auto result = std::invoke(std::forward<F>(f), std::move(*ptr()));
            destroy();
            return result;
        }
        return std::invoke(std::forward<D>(default_fn));
    }

    /// Transforms Option<T> into Result<T, E>, mapping Some(v) to Ok(v).
    template <typename E>
    [[nodiscard]] constexpr fn ok_or(E&& err) && -> Result<T, std::decay_t<E>>;

    /// Transforms Option<T> into Result<T, E>, mapping Some(v) to Ok(v).
    template <typename F>
        requires std::invocable<F>
    [[nodiscard]] constexpr fn ok_or_else(F&& f) && -> Result<T, std::invoke_result_t<F>>;

    /// Returns None if the option is None, otherwise returns optb.
    template <typename U>
    [[nodiscard]] constexpr fn and_(Option<U> optb) && -> Option<U> {
        if (has_value_) {
            destroy();
            return std::move(optb);
        }
        return None;
    }

    /// Returns None if the option is None, otherwise calls f with the value.
    template <typename F>
        requires std::invocable<F, T>
    [[nodiscard]] constexpr fn and_then(F&& f) && -> std::invoke_result_t<F, T> {
        if (has_value_) {
            auto result = std::invoke(std::forward<F>(f), std::move(*ptr()));
            destroy();
            return result;
        }
        return None;
    }

    /// Returns None if the option is None, otherwise calls predicate.
    template <typename P>
        requires std::predicate<P, const T&>
    [[nodiscard]] constexpr fn filter(P&& predicate) && -> Option {
        if (has_value_ && std::invoke(std::forward<P>(predicate), *ptr())) {
            return std::move(*this);
        }
        destroy();
        return None;
    }

    /// Returns the option if it contains a value, otherwise returns optb.
    [[nodiscard]] constexpr fn or_(Option optb) && -> Option {
        if (has_value_) {
            return std::move(*this);
        }
        return std::move(optb);
    }

    /// Returns the option if it contains a value, otherwise calls f.
    template <typename F>
        requires std::invocable<F> &&
                 std::same_as<std::invoke_result_t<F>, Option>
    [[nodiscard]] constexpr fn or_else(F&& f) && -> Option {
        if (has_value_) {
            return std::move(*this);
        }
        return std::invoke(std::forward<F>(f));
    }

    /// Returns Some if exactly one of self or optb is Some, otherwise None.
    [[nodiscard]] constexpr fn xor_(Option optb) && -> Option {
        if (has_value_ && !optb.has_value_) {
            return std::move(*this);
        }
        if (!has_value_ && optb.has_value_) {
            return std::move(optb);
        }
        destroy();
        return None;
    }

    /// Inserts value into the option, then returns a mutable reference.
    constexpr fn insert(T value) -> T& {
        destroy();
        has_value_ = true;
        new (storage_) T(std::move(value));
        return *ptr();
    }

    /// Gets the contained value, inserting a default if None.
    constexpr fn get_or_insert(T value) -> T& {
        if (!has_value_) {
            has_value_ = true;
            new (storage_) T(std::move(value));
        }
        return *ptr();
    }

    /// Gets the contained value, inserting T{} if None.
    constexpr fn get_or_insert_default() -> T&
        requires std::default_initializable<T>
    {
        return get_or_insert(T{});
    }

    /// Gets the contained value, inserting from a closure if None.
    template <typename F>
        requires std::invocable<F> && std::same_as<std::invoke_result_t<F>, T>
    constexpr fn get_or_insert_with(F&& f) -> T& {
        if (!has_value_) {
            has_value_ = true;
            new (storage_) T(std::invoke(std::forward<F>(f)));
        }
        return *ptr();
    }

    /// Takes the value out of the option, leaving None in its place.
    [[nodiscard]] constexpr fn take() -> Option {
        if (has_value_) {
            Option result = std::move(*this);
            destroy();
            return result;
        }
        return None;
    }

    /// Takes the value if the predicate returns true, leaving None.
    template <typename P>
        requires std::predicate<P, T&>
    [[nodiscard]] constexpr fn take_if(P&& predicate) -> Option {
        if (has_value_ && std::invoke(std::forward<P>(predicate), *ptr())) {
            Option result = std::move(*this);
            destroy();
            return result;
        }
        return None;
    }

    /// Replaces the value, returning the old value.
    [[nodiscard]] constexpr fn replace(T value) -> Option {
        Option old = std::move(*this);
        destroy();
        has_value_ = true;
        new (storage_) T(std::move(value));
        return old;
    }

    /// Zips self with another Option.
    template <typename U>
    [[nodiscard]] constexpr fn zip(Option<U> other) && -> Option<std::pair<T, U>> {
        if (has_value_ && other.is_some()) {
            T a = std::move(*ptr());
            U b = std::move(other).unwrap();
            destroy();
            return Some(std::make_pair(std::move(a), std::move(b)));
        }
        destroy();
        return None;
    }

    /// Zips self and another Option with function f.
    template <typename U, typename F>
        requires std::invocable<F, T, U>
    [[nodiscard]] constexpr fn zip_with(Option<U> other, F&& f) &&
        -> Option<std::invoke_result_t<F, T, U>> {
        if (has_value_ && other.is_some()) {
            auto result =
                std::invoke(std::forward<F>(f), std::move(*ptr()), std::move(other).unwrap());
            destroy();
            return Some(std::move(result));
        }
        destroy();
        return None;
    }

    /// Converts from Option<Option<T>> to Option<T>.
    [[nodiscard]] constexpr fn flatten() && -> T
        requires requires { typename T::value_type; }
                     && std::same_as<Option<typename T::value_type>, T>
    {
        if (has_value_) {
            T inner = std::move(*ptr());
            destroy();
            return std::move(inner);
        }
        return None;
    }

    /// Consumes the Option, returning the value (lvalue version).
    /// Use when you want to extract the value from an lvalue Option.
    [[nodiscard]] constexpr fn consume() -> Option {
        Option result = std::move(*this);
        destroy();
        return result;
    }
};

} // namespace rstd
