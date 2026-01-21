#pragma once

#include <print>
#include <string_view>
#include <vector>
#include <source_location>

using usize = size_t;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;

using f64 = double;
using f32 = float;

namespace test {

struct Test {
    std::string_view name;
    void (*fn)();
};

inline auto registry() -> std::vector<Test>& {
    static std::vector<Test> tests;
    return tests;
}

inline auto register_test(std::string_view name, void (*fn)()) -> int {
    registry().push_back({name, fn});
    return 0;
}

inline auto run_all() -> int {
    int passed = 0, failed = 0;
    
    for (const auto& t : registry()) {
        std::print("test: {0} ...", t.name);
        
        try {
            t.fn();
            std::print("ok\n");
            ++passed;
        } catch (const std::exception& e) {
            std::print("FAILED\n  {0}\n", e.what());
            ++failed;
        }
    }
    
    std::print("\nResult: {0} passed, {1} failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}

struct AssertFailed : std::exception {
    std::string msg;
    AssertFailed(std::string_view expr, const std::source_location& loc)
        : msg(std::string(loc.file_name()) + ":" + std::to_string(loc.line()) + 
              ": assertion failed: " + std::string(expr)) {}

    const char* what() const noexcept override {
        return msg.c_str();
    }
};

}; // namespace test

// Zig-like Testing API
/*
TEST(addition) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_EQ(10 + 5, 15);
}
*/ 

#define TEST(name) \
    static void test_##name(); \
    static int test_reg_##name = test::register_test(#name, test_##name); \
    static void test_##name()

#define EXPECT(expr) \
    do { if (!(expr)) throw test::AssertFailed(#expr, std::source_location::current()); } while(0)

#define EXPECT_EQ(a, b) EXPECT((a) == (b))
