# Bringing Rust to C++

- Function Comments courtesy of Claude

```cpp
using namespace rstd;

fn divide(i32 a, i32 b) -> Result<i32, std::string> {
    if (b == 0) {
        return Err<i32, std::string>("division by zero");
    }
    return Ok<i32, std::string>(a / b);
}

fn main() -> i32 {
    auto result1 = Some(5)
        .and_then([](i32 x) { return x > 0 ? Some(x * 2) : Option<i32>(None); })
        .and_then([](i32 x) { return Some(x + 1); });

    std::println("{}", std::move(result1).unwrap());

    auto result2 = divide(10, 2)
        .and_then([](i32 x) { return divide(x, 2); })
        .map([](i32 x) { return x + 1; })
        .unwrap_or(0);

    std::println("{}", result2);
    
    return 0;
}
```
