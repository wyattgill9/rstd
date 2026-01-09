#pragma once

#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "macros.hh"
#include "option.hh"

namespace rstd {

template <typename T>
class Option;

template <typename T, typename E>
class Result;

// Tag types for Ok and Err
template<typename T>
struct OkTag {
    T value;
};

template<typename E>
struct ErrTag {
    E value;
};

template<typename T, typename E>
constexpr inline fn Ok(const T& val) -> Result<T, E> {
    return Result<T,E>(OkTag<T>{val});
}

template<typename T, typename E>
constexpr inline fn Ok(T&& val) -> Result<T, E> {
    return Result<T,E>(OkTag<T>{std::forward<T>(val)});
}

template<typename T, typename E>
constexpr inline fn Err(const E& e) -> Result<T, E> {
    return Result<T,E>(ErrTag<E>{e});
}

template<typename T, typename E>
constexpr inline fn Err(E&& e) -> Result<T, E> {
    return Result<T,E>(ErrTag<E>{std::forward<E>(e)});
}

template <typename T, typename E>
class Result {
public:
    using ok_type = T;
    using err_type = E;

private:
    union Storage {
        T ok_val;
        E err_val;

        constexpr Storage() {}
        constexpr ~Storage() {}
    } storage_;
    bool is_ok_;

    constexpr fn destroy() -> void {
        if (is_ok_) {
            storage_.ok_val.~T();
        } else {
            storage_.err_val.~E();
        }
    }

public:
    // Constructors for OkTag
    constexpr Result(OkTag<T>&& ok) : is_ok_(true) {
        new (&storage_.ok_val) T(std::move(ok.value));
    }

    constexpr Result(const OkTag<T>& ok) : is_ok_(true) {
        new (&storage_.ok_val) T(ok.value);
    }

    // Constructors for ErrTag
    constexpr Result(ErrTag<E>&& err) : is_ok_(false) {
        new (&storage_.err_val) E(std::move(err.value));
    }

    constexpr Result(const ErrTag<E>& err) : is_ok_(false) {
        new (&storage_.err_val) E(err.value);
    }

    // Copy constructor
    constexpr Result(const Result& other) : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&storage_.ok_val) T(other.storage_.ok_val);
        } else {
            new (&storage_.err_val) E(other.storage_.err_val);
        }
    }

    // Move constructor
    constexpr Result(Result&& other) noexcept : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&storage_.ok_val) T(std::move(other.storage_.ok_val));
        } else {
            new (&storage_.err_val) E(std::move(other.storage_.err_val));
        }
    }

    // Destructor
    constexpr ~Result() { destroy(); }

    // Copy assignment
    constexpr fn operator=(const Result& other) -> Result& {
        if (this != &other) {
            destroy();
            is_ok_ = other.is_ok_;
            if (is_ok_) {
                new (&storage_.ok_val) T(other.storage_.ok_val);
            } else {
                new (&storage_.err_val) E(other.storage_.err_val);
            }
        }
        return *this;
    }

    // Move assignment
    constexpr fn operator=(Result&& other) noexcept -> Result& {
        if (this != &other) {
            destroy();
            is_ok_ = other.is_ok_;
            if (is_ok_) {
                new (&storage_.ok_val) T(std::move(other.storage_.ok_val));
            } else {
                new (&storage_.err_val) E(std::move(other.storage_.err_val));
            }
        }
        return *this;
    }

    /// Returns true if the result is Ok.
    [[nodiscard]] constexpr fn is_ok() const noexcept -> bool {
        return is_ok_;
    }
    
    /// Returns true if the result is Err.
    [[nodiscard]] constexpr fn is_err() const noexcept -> bool {
        return !is_ok_;
    }

    /// Returns true if the result is Ok and the value matches a predicate.
    template <typename F>
        requires std::predicate<F, const T&>
    [[nodiscard]] constexpr fn is_ok_and(F&& f) const -> bool {
        return is_ok_ && std::invoke(std::forward<F>(f), storage_.ok_val);
    }

    /// Returns true if the result is Err and the value matches a predicate.
    template <typename F>
        requires std::predicate<F, const E&>
    [[nodiscard]] constexpr fn is_err_and(F&& f) const -> bool {
        return !is_ok_ && std::invoke(std::forward<F>(f), storage_.err_val);
    }

    /// Converts from Result<T, E> to Option<T>.
    [[nodiscard]] constexpr fn ok() && -> Option<T> {
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return Some(std::move(val));
        }
        destroy();
        return None;
    }

    /// Converts from Result<T, E> to Option<E>.
    [[nodiscard]] constexpr fn err() && -> Option<E> {
        if (!is_ok_) {
            E val = std::move(storage_.err_val);
            destroy();
            return Some(std::move(val));
        }
        destroy();
        return None;
    }

    /// Converts from &Result<T, E> to Result<const T*, const E*>.
    [[nodiscard]] constexpr fn as_ref() const noexcept -> Result<const T*, const E*> {
        if (is_ok_) {
            return Ok<const T*, const E*>(&storage_.ok_val);
        }
        return Err<const T*, const E*>(&storage_.err_val);
    }

    /// Converts from &mut Result<T, E> to Result<T*, E*>.
    [[nodiscard]] constexpr fn as_mut() noexcept -> Result<T*, E*> {
        if (is_ok_) {
            return Ok<T*, E*>(&storage_.ok_val);
        }
        return Err<T*, E*>(&storage_.err_val);
    }

    /// Maps a Result<T, E> to Result<U, E> by applying a function.
    template <typename F>
        requires std::invocable<F, T>
    [[nodiscard]] constexpr fn map(F&& f) && -> Result<std::invoke_result_t<F, T>, E> {
        using U = std::invoke_result_t<F, T>;
        if (is_ok_) {
            U result = std::invoke(std::forward<F>(f), std::move(storage_.ok_val));
            destroy();
            return Ok<U, E>(std::move(result));
        }
        E err = std::move(storage_.err_val);
        destroy();
        return Err<U, E>(std::move(err));
    }

    /// Returns the provided default or applies a function.
    template <typename U, typename F>
        requires std::invocable<F, T> && std::convertible_to<std::invoke_result_t<F, T>, U>
    [[nodiscard]] constexpr fn map_or(U&& default_val, F&& f) && -> U {
        if (is_ok_) {
            U result = std::invoke(std::forward<F>(f), std::move(storage_.ok_val));
            destroy();
            return result;
        }
        destroy();
        return std::forward<U>(default_val);
    }

    /// Maps a Result<T, E> to U by applying fallback or function.
    template <typename D, typename F>
        requires std::invocable<D, E> && std::invocable<F, T>
    [[nodiscard]] constexpr fn map_or_else(D&& default_fn, F&& f) &&
        -> std::common_type_t<std::invoke_result_t<D, E>, std::invoke_result_t<F, T>> {
        if (is_ok_) {
            auto result = std::invoke(std::forward<F>(f), std::move(storage_.ok_val));
            destroy();
            return result;
        }
        auto result = std::invoke(std::forward<D>(default_fn), std::move(storage_.err_val));
        destroy();
        return result;
    }

    /// Maps a Result<T, E> to Result<T, F> by applying a function to the error.
    template <typename O>
        requires std::invocable<O, E>
    [[nodiscard]] constexpr fn map_err(O&& op) && -> Result<T, std::invoke_result_t<O, E>> {
        using F = std::invoke_result_t<O, E>;
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return Ok<T, F>(std::move(val));
        }
        F result = std::invoke(std::forward<O>(op), std::move(storage_.err_val));
        destroy();
        return Err<T, F>(std::move(result));
    }

    /// Calls a function with a reference to the contained Ok value.
    template <typename F>
        requires std::invocable<F, const T&>
    [[nodiscard]] constexpr fn inspect(F&& f) && -> Result {
        if (is_ok_) {
            std::invoke(std::forward<F>(f), storage_.ok_val);
        }
        return std::move(*this);
    }

    /// Calls a function with a reference to the contained Err value.
    template <typename F>
        requires std::invocable<F, const E&>
    [[nodiscard]] constexpr fn inspect_err(F&& f) && -> Result {
        if (!is_ok_) {
            std::invoke(std::forward<F>(f), storage_.err_val);
        }
        return std::move(*this);
    }

    /// Returns the contained Ok value, consuming the self value.
    /// Throws if the value is Err.
    [[nodiscard]] constexpr fn expect(const char* msg) && -> T {
        if (UNLIKELY(!is_ok_)) {
            throw std::runtime_error(msg);
        }
        T val = std::move(storage_.ok_val);
        destroy();
        return val;
    }

    /// Returns the contained Ok value, consuming the self value.
    /// Throws if the value is Err.
    [[nodiscard]] constexpr fn unwrap() && -> T {
        return std::move(*this).expect("called `Result::unwrap()` on an `Err` value");
    }

    /// Returns the contained Ok value or a default.
    [[nodiscard]] constexpr fn unwrap_or_default() && -> T
        requires std::default_initializable<T>
    {
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return val;
        }
        destroy();
        return T{};
    }

    /// Returns the contained Err value, consuming the self value.
    /// Throws if the value is Ok.
    [[nodiscard]] constexpr fn expect_err(const char* msg) && -> E {
        if (UNLIKELY(is_ok_)) {
            throw std::runtime_error(msg);
        }
        E val = std::move(storage_.err_val);
        destroy();
        return val;
    }

    /// Returns the contained Err value, consuming the self value.
    /// Throws if the value is Ok.
    [[nodiscard]] constexpr fn unwrap_err() && -> E {
        return std::move(*this).expect_err("called `Result::unwrap_err()` on an `Ok` value");
    }

    /// Returns the contained Ok value without checking.
    /// Undefined behavior if the value is Err.
    [[nodiscard]] constexpr fn unwrap_unchecked() && noexcept -> T {
        T val = std::move(storage_.ok_val);
        destroy();
        return val;
    }

    /// Returns the contained Err value without checking.
    /// Undefined behavior if the value is Ok.
    [[nodiscard]] constexpr fn unwrap_err_unchecked() && noexcept -> E {
        E val = std::move(storage_.err_val);
        destroy();
        return val;
    }

    /// Returns res if the result is Ok, otherwise returns the Err value.
    template <typename U>
    [[nodiscard]] constexpr fn and_(Result<U, E> res) && -> Result<U, E> {
        if (is_ok_) {
            destroy();
            return std::move(res);
        }
        E err = std::move(storage_.err_val);
        destroy();
        return Err<U, E>(std::move(err));
    }

    /// Calls op if the result is Ok, otherwise returns the Err value.
    template <typename F>
        requires std::invocable<F, T>
    [[nodiscard]] constexpr fn and_then(F&& op) && -> std::invoke_result_t<F, T> {
        using ResultType = std::invoke_result_t<F, T>;
        if (is_ok_) {
            auto result = std::invoke(std::forward<F>(op), std::move(storage_.ok_val));
            destroy();
            return result;
        }
        E err = std::move(storage_.err_val);
        destroy();
        // Extract E from Result<U, E>
        using U = typename ResultType::ok_type;
        return Err<U, E>(std::move(err));
    }

    /// Returns res if the result is Err, otherwise returns the Ok value.
    template <typename F>
    [[nodiscard]] constexpr fn or_(Result<T, F> res) && -> Result<T, F> {
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return Ok<T, F>(std::move(val));
        }
        destroy();
        return std::move(res);
    }

    /// Calls op if the result is Err, otherwise returns the Ok value.
    template <typename O>
        requires std::invocable<O, E>
    [[nodiscard]] constexpr fn or_else(O&& op) && -> std::invoke_result_t<O, E> {
        using ResultType = std::invoke_result_t<O, E>;
        using F = typename ResultType::err_type;
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return Ok<T, F>(std::move(val));
        }
        auto result = std::invoke(std::forward<O>(op), std::move(storage_.err_val));
        destroy();
        return result;
    }

    /// Returns the contained Ok value or a provided default.
    template <typename U>
        requires std::convertible_to<U, T>
    [[nodiscard]] constexpr fn unwrap_or(U&& default_val) && -> T {
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return val;
        }
        destroy();
        return static_cast<T>(std::forward<U>(default_val));
    }

    /// Returns the contained Ok value or computes it from a closure.
    template <typename F>
        requires std::invocable<F, E> && std::convertible_to<std::invoke_result_t<F, E>, T>
    [[nodiscard]] constexpr fn unwrap_or_else(F&& f) && -> T {
        if (is_ok_) {
            T val = std::move(storage_.ok_val);
            destroy();
            return val;
        }
        T result = std::invoke(std::forward<F>(f), std::move(storage_.err_val));
        destroy();
        return result;
    }

    /// Consumes the Result, returning a moved copy (lvalue version).
    [[nodiscard]] constexpr fn consume() -> Result {
        return std::move(*this);
    }
};

template <typename T>
template <typename E>
constexpr fn Option<T>::ok_or(E&& err) && -> Result<T, std::decay_t<E>> {
    using ErrType = std::decay_t<E>;
    if (has_value_) {
        T val = std::move(*ptr());
        destroy();
        return Ok<T, ErrType>(std::move(val));
    }
    return Err<T, ErrType>(std::forward<E>(err));
}

template <typename T>
template <typename F>
    requires std::invocable<F>
constexpr fn Option<T>::ok_or_else(F&& f) && -> Result<T, std::invoke_result_t<F>> {
    using ErrType = std::invoke_result_t<F>;
    if (has_value_) {
        T val = std::move(*ptr());
        destroy();
        return Ok<T, ErrType>(std::move(val));
    }
    return Err<T, ErrType>(std::invoke(std::forward<F>(f)));
}

} // namespace rstd
