#include <vector>

template<
    typename T,
    class Allocator = std::allocator<T>
>
using Vec = std::vector<T, Allocator>;
