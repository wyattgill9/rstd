#pragma once

#include <cstddef>
#include <new>

#ifdef __linux__
    #include <sys/mman.h>
#elif defined(__APPLE__)
    #include <sys/mman.h>
#else
    #include <cstdlib>
#endif

constexpr static size_t Huge2MB = 2ULL << 20;
constexpr static size_t Huge1GB = 1ULL << 30;

template <std::size_t NumPages = 1, std::size_t PageSize = Huge2MB>
class HugePageAlloc {
public:
    HugePageAlloc() {
        begin_ = static_cast<std::byte*>(alloc_pages(huge_pages));
        if (!begin_) [[unlikely]] {
            throw std::bad_alloc{};
        }
        cur_ = begin_;
    }
    
    ~HugePageAlloc() {
        if (begin_) {
            dealloc_pages(begin_);
        }
    }
    
    HugePageAlloc(const HugePageAlloc&) = delete;
    HugePageAlloc& operator=(const HugePageAlloc&) = delete;
    HugePageAlloc(HugePageAlloc&&) = delete;
    HugePageAlloc& operator=(HugePageAlloc&&) = delete;

    [[nodiscard]] auto allocate(std::size_t size, std::size_t alignment = 64) noexcept -> void* {
        auto* aligned = reinterpret_cast<std::byte*>(
            align_up(reinterpret_cast<uintptr_t>(cur_), alignment));

        if (auto* next = aligned + size; next <= end()) [[likely]] {
            cur_ = next;
            return aligned;
        }
        return nullptr;
    }
    
    template <typename T>
    [[nodiscard]] auto allocate(std::size_t count = 1) noexcept -> T* {
        return static_cast<T*>(allocate(sizeof(T) * count, alignof(T)));
    }
    
    auto reset() noexcept -> void {
        cur_ = begin_;
    }
    
    auto prefault() noexcept -> void {
        for (volatile auto* p = begin_; p < end(); p += PageSize) {
            (void)*p;
        }
    }

    [[nodiscard]] auto used() const noexcept -> std::size_t {
        return cur_ - begin_;
    }

    [[nodiscard]] auto available() const noexcept -> std::size_t {
        return end() - cur_;
    }

    [[nodiscard]] auto using_huge_pages() const noexcept -> bool {
        return huge_pages;
    }

private:
    [[nodiscard]] auto end() const noexcept -> std::byte* {
        return begin_ + cap_;
    }

    static auto alloc_pages(bool& huge) noexcept -> void* {
#ifdef __linux__
        constexpr int huge_flag = (PageSize == Huge2MB)
            ? (MAP_HUGETLB | (21 << MAP_HUGE_SHIFT))
            : (MAP_HUGETLB | (30 << MAP_HUGE_SHIFT));
        
        if (auto* p = ::mmap(nullptr, cap_, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | huge_flag, -1, 0);
            p != MAP_FAILED) {
            huge = true;
            return p;
        }

        huge = false;
        auto* p = ::mmap(nullptr, cap_, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (p == MAP_FAILED) ? nullptr : p;

#elif defined(__APPLE__)
        huge = false;
        auto* p = ::mmap(nullptr, cap_, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return (p == MAP_FAILED) ? nullptr : p;

#else
        huge = false;
        return std::aligned_alloc(PageSize, cap_);
#endif
    }

    static auto dealloc_pages(void* ptr) noexcept -> void {
#if defined(__linux__) || defined(__APPLE__)
        ::munmap(ptr, cap_);
#else
        std::free(ptr);
#endif
    }

    constexpr auto align_up(std::size_t value, std::size_t alignment) noexcept -> std::size_t {
        return value + (alignment - value % alignment) % alignment;
    }

    constexpr static std::size_t cap_  = NumPages * PageSize;

    std::byte* begin_ {nullptr};
    std::byte* cur_   {nullptr};
    bool huge_pages   {false};
};
