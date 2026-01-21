#include <string>
#include <vector>

#include "result.hh"
#include "utils.hh"

enum class Error {
    NotFound,
    InvalidInput,
    OutOfRange
};

struct User {
    int id;
    std::string name;
};

auto find_user(int id) -> Option<User> {
    if (id == 1) return Some(User {1, "Alice"});
    if (id == 2) return Some(User {2, "Bob"});
    return None;
}

auto parse_int(std::string_view s) -> Result<int, Error> {
    if (s.empty()) return Err(Error::InvalidInput);
    if (s == "42") return Ok(42);
    if (s == "100") return Ok(100);
    return Err(Error::NotFound);
}

auto divide(int a, int b) -> Result<int, Error> {
    if (b == 0) return Err(Error::InvalidInput);
    return Ok(a / b);
}

auto safe_get(const std::vector<int>& v, size_t i) -> Option<int> {
    if (i < v.size()) return Some(v[i]);
    return None;
}

TEST(option_basic) {
    auto opt_some = Some(42);
    auto opt_none = Option<int>(None);

    EXPECT(opt_some.is_some());
    EXPECT(opt_none.is_none());
    EXPECT(opt_some.is_some_and([](int x) { return x > 0; }));
    EXPECT(opt_none.is_none_or([](int x) { return x > 0; }));
}

TEST(option_unwrap) {
    auto opt_some = Some(42);
    auto opt_none = Option<int>(None);

    EXPECT_EQ(opt_some.unwrap_or(0), 42);
    EXPECT_EQ(opt_none.unwrap_or(0), 0);
    EXPECT_EQ(opt_none.unwrap_or_else([] { return -1; }), -1);
}

TEST(option_map) {
    auto opt_some = Some(42);

    auto doubled = opt_some.map([](int x) { return x * 2; });
    EXPECT_EQ(doubled.unwrap(), 84);

    auto str_len = opt_some
        .map([](int x) { return std::to_string(x); })
        .map([](const std::string& s) { return s.length(); });

    EXPECT_EQ(str_len.unwrap(), 2);
}

TEST(option_chaining) {
    auto chained = Some(5)
        .and_then([](int x) { return x > 0 ? Some(x * 10) : None; })
        .filter([](int x) { return x < 100; })
        .map([](int x) { return x + 1; });

    EXPECT_EQ(chained.unwrap(), 51);
}

TEST(option_fallback) {
    auto opt_none = Option<int>(None);

    auto fallback = opt_none | Some(999);
    EXPECT_EQ(fallback.unwrap(), 999);

    auto lazy_fallback = opt_none.or_else([] { return Some(123); });
    EXPECT_EQ(lazy_fallback.unwrap(), 123);
}

TEST(option_flatten) {
    auto nested = Some(Some(42));
    auto flat = std::move(nested).flatten();

    EXPECT_EQ(flat.unwrap(), 42);
}

TEST(option_zip) {
    auto zipped = Some(1)
        .zip(Some(std::string("one")));
    auto [num, str] = std::move(zipped).unwrap();

    EXPECT_EQ(num, 1);
    EXPECT_EQ(str, "one");
}

TEST(option_unzip) {
    auto pair_opt = Some(std::pair{42, std::string("answer")});
    auto [opt_a, opt_b] = std::move(pair_opt).unzip();

    EXPECT_EQ(opt_a.unwrap(), 42);
    EXPECT_EQ(opt_b.unwrap(), "answer");
}

TEST(option_with_user) {
    auto user = find_user(1);
    auto name = std::move(user)
        .map([](User u) { return u.name; });

    EXPECT_EQ(name.unwrap(), "Alice");

    auto missing = find_user(999);
    auto result_from_opt = std::move(missing)
        .ok_or(Error::NotFound);

    EXPECT(result_from_opt.is_err());
}

TEST(result_basic) {
    auto res_ok = Result<int, Error>(Ok(42));
    auto res_err = Result<int, Error>(Err(Error::NotFound));

    EXPECT(res_ok.is_ok());
    EXPECT(res_err.is_err());
    EXPECT(res_ok.is_ok_and([](int x) { return x == 42; }));
    EXPECT(res_err.is_err_and([](Error e) { return e == Error::NotFound; }));
}

TEST(result_unwrap) {
    auto res_ok = Result<int, Error>(Ok(42));
    auto res_err = Result<int, Error>(Err(Error::NotFound));

    EXPECT_EQ(res_ok.unwrap_or(0), 42);
    EXPECT_EQ(res_err.unwrap_or(0), 0);
    EXPECT_EQ(res_err.unwrap_or_else([](Error) { return -1; }), -1);
}

TEST(result_map) {
    auto mapped = parse_int("42")
        .map([](int x) { return x * 2; });

    EXPECT_EQ(mapped.unwrap(), 84);

    auto mapped_err = parse_int("")
        .map_err([](Error) { return std::string("parse failed"); });

    EXPECT_EQ(mapped_err.unwrap_err(), "parse failed");
}

TEST(result_chaining) {
    auto computation = parse_int("42")
        .and_then([](int x) { return divide(x, 2); })
        .map([](int x) { return x + 1; });

    EXPECT_EQ(computation.unwrap(), 22);
}

TEST(result_recovery) {
    auto recovered = parse_int("invalid")
        .or_else([](Error) { return Result<int, Error>(Ok(0)); });

    EXPECT_EQ(recovered.unwrap(), 0);

    auto alt = parse_int("bad")
        | Result<int, Error>(Ok(999));

    EXPECT_EQ(alt.unwrap(), 999);
}

TEST(result_flatten) {
    auto nested_result = Result<Result<int, Error>, Error>(
        Ok(Result<int, Error>(Ok(42)))
    );
    auto flat_result = std::move(nested_result).flatten();

    EXPECT_EQ(flat_result.unwrap(), 42);
}

TEST(result_conversion) {
    auto ok_as_opt = Result<int, Error>(Ok(42))
        .ok_value();
    auto err_as_opt = Result<int, Error>(Err(Error::NotFound))
        .err_value();

    EXPECT_EQ(ok_as_opt.unwrap(), 42);
    EXPECT_EQ(err_as_opt.unwrap(), Error::NotFound);
}

TEST(option_zip_with) {
    std::vector<int> vec{10, 20, 30};

    auto sum = safe_get(vec, 0)
        .zip_with(safe_get(vec, 1), [](int a, int b) { return a + b; })
        .and_then([&](int s) {
            return safe_get(vec, 2)
                .map([s](int c) { return s + c; });
        });

    EXPECT_EQ(sum.unwrap(), 60);
}

TEST(option_mutation) {
    Option<int> mutable_opt = None;
    mutable_opt.get_or_insert(42);

    EXPECT_EQ(mutable_opt.unwrap(), 42);

    Option<int> to_take = Some(100);
    auto taken = to_take.take();

    EXPECT_EQ(taken.unwrap(), 100);
    EXPECT(to_take.is_none());

    Option<int> to_replace = Some(1);
    auto old = to_replace.replace(2);

    EXPECT_EQ(old.unwrap(), 1);
    EXPECT_EQ(to_replace.unwrap(), 2);

    auto conditional_take = Some(50)
        .take_if([](int& x) { return x > 25; });

    EXPECT_EQ(conditional_take.unwrap(), 50);
}

TEST(option_xor) {
    auto xor_result = Some(1)
        ^ Option<int>(None);
    EXPECT_EQ(xor_result.unwrap(), 1);

    auto both_some = Some(1)
        ^ Some(2);

    EXPECT(both_some.is_none());
}

TEST(option_inspect) {
    bool inspected = false;
    auto _ = Some(42)
        .inspect([&](int) { inspected = true; });

    EXPECT(inspected);
}

TEST(result_inspect_err) {
    bool err_inspected = false;
    auto _ = Result<int, Error>(Err(Error::NotFound))
        .inspect_err([&](Error) { err_inspected = true; });

    EXPECT(err_inspected);
}

TEST(complex_chain) {
    auto complex_chain = find_user(2)
        .map([](User u) { return u.id; })
        .ok_or(Error::NotFound)
        .and_then([](int id) { return divide(100, id); })
        .map([](int x) { return std::to_string(x); })
        .unwrap_or("error");

    EXPECT_EQ(complex_chain, "50");
}

auto main() -> i32 {
    return test::run_all();
}
