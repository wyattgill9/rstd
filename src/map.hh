#include <memory>
#include <map>

template<
    class Key,
    class T,
    class Compare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key, T>>
>
using Map = std::map<Key, T, Compare, Allocator>;
