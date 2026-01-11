# Bringing Rust to C++

**API EXAMPLE:**
```cpp
fn divide(i32 a, i32 b) -> Result<i32, std::string> {
    return b == 0
        ? Err<i32, std::string>("division by zero")
        : Ok<i32, std::string>(a / b);
}

fn parse_number(const char* s) -> Option<i32> {
    try {
        return Some(std::stoi(s));
    }
    catch (...) {
        return None;
    }
}

fn main() -> i32 {
    Vec<i32> primes;
    for (auto n : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29})
        primes.push(n);

    primes.retain([](const i32& x) { return x > 10; });

    std::print("Primes > 10: ");
    for (auto p = primes.pop(); p.is_some(); p = primes.pop())
        std::print("{} ", std::move(p).unwrap());
    std::println("");

    auto safe_divide = [](i32 a, i32 b) -> Option<i32> {
        return b == 0 ? None : Some(a / b);
    };

    auto result = Some(100)
        .and_then([&](i32 x) { return safe_divide(x, 5); })
        .and_then([&](i32 x) { return safe_divide(x, 2); })
        .map([](i32 x) { return x * 3; })
        .filter([](const i32& x) { return x > 20; });

    auto combo = Some(7).zip_with(Some(8), [](i32 a, i32 b) {
        return a * b;
    });
    std::println("7 × 8 = {}", std::move(combo).unwrap_or(0));

    auto calculate = divide(144, 12)
        .and_then([](i32 x) { return divide(x, 3); })
        .and_then([](i32 x) { return divide(x, 0); })
        .map([](i32 x) { return x * 100; })
        .map_err([](auto e) { return std::string("Error: ") + e; })
        .or_else([](auto) { return Ok<i32, std::string>(42); });

    auto validated = parse_number("256")
        .ok_or(std::string("invalid input"))
        .and_then([](i32 n) {
            return n > 0 && n < 1000
                ? Ok<i32, std::string>(n)
                : Err<i32, std::string>("out of range");
        })
        .map([](i32 n) { return n * 2; });

    std::println("{}", std::move(validated).unwrap_or(-1));

    auto pipeline = Some(std::string("hello"))
        .inspect([](const auto& s) { std::print("input: {} → ", s); })
        .map([](auto s) { return s + " world"; })
        .inspect([](const auto& s) { std::print("{} → ", s); })
        .map([](auto s) { return s.length(); });

    return 0;
}
```
