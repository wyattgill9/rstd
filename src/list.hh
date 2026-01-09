#include <memory>
#include <list>

template<
    typename T,
    class Allocator = std::allocator<T>
>
using LinkedList = std::list<T, Allocator>;
