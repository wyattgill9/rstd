#include <cassert>
#include <format>
#include <memory>
#include <span>
#include <stdexcept>

#include "macros.hh"
#include "option.hh"
#include "types.hh"

#include "result.hh"

namespace rstd {

template<
    typename T,
    class Allocator = std::allocator<T>
>
class Vec {
    using AllocTraits = std::allocator_traits<Allocator>;

public:
    // Constructors
    Vec() : begin_(nullptr), end_(nullptr), cap_(nullptr) {}

    // Copy constructor
    Vec(const Vec& other) : begin_(nullptr), end_(nullptr), cap_(nullptr), alloc_(other.alloc_) {
        if (other.len() > 0) {
            begin_ = AllocTraits::allocate(alloc_, other.len());
            end_ = begin_;
            cap_ = begin_ + other.len();
            for (usize i = 0; i < other.len(); ++i) {
                AllocTraits::construct(alloc_, end_, other.begin_[i]);
                ++end_;
            }
        }
    }

    // Move constructor
    constexpr Vec(Vec&& other) noexcept
        : begin_(other.begin_), end_(other.end_), cap_(other.cap_), alloc_(std::move(other.alloc_)) {
        other.begin_ = nullptr;
        other.end_ = nullptr;
        other.cap_ = nullptr;
    }

    // Destructor
    constexpr ~Vec() {
        if (begin_) {
            for (T* p = begin_; p != end_; ++p) {
                AllocTraits::destroy(alloc_, p);
            }
            AllocTraits::deallocate(alloc_, begin_, capacity());
        }
    }

    // Copy assignment
    constexpr fn operator=(const Vec& other) -> Vec& {
        if (this != &other) {
            for (T* p = begin_; p != end_; ++p) {
                AllocTraits::destroy(alloc_, p);
            }

            usize other_len = other.len();
            if (capacity() < other_len) {
                if (begin_) {
                    AllocTraits::deallocate(alloc_, begin_, capacity());
                }
                alloc_ = other.alloc_;
                begin_ = AllocTraits::allocate(alloc_, other_len);
                cap_ = begin_ + other_len;
            }

            end_ = begin_;
            for (usize i = 0; i < other_len; ++i) {
                AllocTraits::construct(alloc_, end_, other.begin_[i]);
                ++end_;
            }
        }
        return *this;
    }

    // Move assignment
    constexpr fn operator=(Vec&& other) noexcept -> Vec& {
        if (this != &other) {
            if (begin_) {
                for (T* p = begin_; p != end_; ++p) {
                    AllocTraits::destroy(alloc_, p);
                }
                AllocTraits::deallocate(alloc_, begin_, capacity());
            }

            begin_ = other.begin_;
            end_ = other.end_;
            cap_ = other.cap_;
            alloc_ = std::move(other.alloc_);

            other.begin_ = nullptr;
            other.end_ = nullptr;
            other.cap_ = nullptr;
        }
        return *this;
    }

    constexpr fn operator[](usize index) -> T& {
        return begin_[index];
    }

    constexpr fn operator[](usize index) const -> const T& {
        return begin_[index];
    }

    fn at(usize index) -> T& {
        if (index >= len()) {
            throw std::out_of_range(
                std::format("index out of bounds: the len is {} but the index is {}", len(), index)
            );
        }
        return begin_[index];
    }

    fn at(usize index) const -> const T& {
        if (index >= len()) {
            throw std::out_of_range(
                std::format("index out of bounds: the len is {} but the index is {}", len(), index)
            );
        }
        return begin_[index];
    }

    static fn with_capacity(usize capacity) -> Vec {
        Vec v;
        if(capacity > 0) {
            v.begin_ = AllocTraits::allocate(v.alloc_, capacity);
            v.end_ = v.begin_;
            v.cap_ = v.begin_ + capacity;
        }
        return v;
    }

    /// Creates a `Vec<T>` directly from a pointer, a length, and a capacity.
    static fn from_raw_parts(T* ptr, usize length, usize capacity) -> Vec {
        Vec v;
        if(capacity > 0) {           
            v.begin_ = ptr;
            v.end_ = v.begin_ + length;
            v.cap_ = v.begin_ + capacity;
        }
        return v;
    }

    /// Decomposes a `Vec<T>` into its raw components: `(pointer, length, capacity)`.
    fn into_raw_parts() -> std::tuple<T*, usize, usize> {
        return std::make_tuple(begin_, len(), capacity());
    }

    /// Constructs a new, empty `Vec<T, A>`.
    template<Allocator>
    static fn new_in(Allocator alloc) -> Vec {
        Vec v;
        v.alloc_ = alloc;
        return v;
    }

    /// Constructs a new, empty `Vec<T, A>` with at least the specified capacity
    /// with the provided allocator.
    // pub fn with_capacity_in(capacity: usize, alloc: A) -> Self {
    
    /// Constructs a new, empty `Vec<T, A>` with at least the specified capacity
    /// with the provided allocator.
    /// pub fn try_with_capacity_in(capacity: usize, alloc: A) -> Result<Self, TryReserveError> {

    /// Creates a `Vec<T, A>` directly from a pointer, a length, a capacity,
    /// and an allocator.
    static fn from_raw_parts_in(T* ptr, usize length, usize capacity, Allocator alloc) -> Vec {
        Vec v;
        if(capacity > 0) {           
            v.begin_ = ptr;
            v.end_ = v.begin_ + length;
            v.cap_ = v.begin_ + capacity;
        }
        v.alloc_ = alloc;
        return v;
    }
    
    /// Decomposes a `Vec<T>` into its raw components: `(pointer, length, capacity, allocator)`.
    ///
    /// Returns the raw pointer to the underlying data, the length of the vector (in elements),
    /// the allocated capacity of the data (in elements), and the allocator. These are the same
    /// arguments in the same order as the arguments to [`from_raw_parts_in`].
    ///
    /// After calling this function, the caller is responsible for the
    /// memory previously managed by the `Vec`. The only way to do
    /// this is to convert the raw pointer, length, and capacity back
    /// into a `Vec` with the [`from_raw_parts_in`] function, allowing
    /// the destructor to perform the cleanup.
    fn into_raw_parts_with_alloc() -> std::tuple<T*, usize, usize, Allocator> {
        return std::make_tuple(begin_, len(), capacity(), alloc_);
    }

    /// Returns the total number of elements the vector can hold without
    /// reallocating.
    constexpr fn capacity() const -> usize {
        return static_cast<usize>(cap_ - begin_);
    }

    /// Reserves capacity for at least `additional` more elements to be inserted
    /// in the given `Vec<T>`. The collection may reserve more space to
    /// speculatively avoid frequent reallocations. After calling `reserve`,
    /// capacity will be greater than or equal to `self.len() + additional`.
    /// Does nothing if capacity is already sufficient.
    fn reserve(usize additional) {
        usize required = len() + additional;
        if (required <= capacity()) {
            return;
        }
        usize new_cap = std::max(required, capacity() * 2);
        if (new_cap == 0) {
            new_cap = 1;
        }
        reallocate(new_cap);
    }
    
    /// Reserves the minimum capacity for at least `additional` more elements to
    /// be inserted in the given `Vec<T>`. Unlike [`reserve`], this will not
    /// deliberately over-allocate to speculatively avoid frequent allocations.
    /// After calling `reserve_exact`, capacity will be greater than or equal to
    /// `self.len() + additional`. Does nothing if the capacity is already
    /// sufficient.
    fn reserve_exact(usize additional) {
        usize required = len() + additional;
        if (required <= capacity()) {
            return;
        }
        reallocate(required);
    }

    /// Shrinks the capacity of the vector as much as possible.
    fn shrink_to_fit() {
        shrink_to(len());
    }

    /// Shrinks the capacity of the vector with a lower bound.
    fn shrink_to(usize min_capacity) {
        usize target = std::max(len(), min_capacity);
        if (target < capacity()) {
            reallocate(target);
        }
    }

    fn into_boxed_slice() -> std::span<T> {
        shrink_to_fit();
        std::span<T> slice(begin_, len());
        begin_ = nullptr;
        end_ = nullptr;
        cap_ = nullptr;
        return slice;
    }

    fn truncate(usize new_len) {
        if (new_len >= len()) {
            return;
        }
        // Destroy elements being dropped
        T* new_end = begin_ + new_len;
        for (T* p = new_end; p != end_; ++p) {
            AllocTraits::destroy(alloc_, p);
        }
        end_ = new_end;
    }

    fn as_slice() const -> std::span<const T> {
        return std::span(begin_, len());
    }

    fn as_mut_slice() -> std::span<T> {
        return std::span(begin_, len());
    }

    fn as_ptr() const -> const T* {
        return begin_;
    }

    fn as_mut_ptr() -> T* {
        return begin_;
    }

    fn allocator() const -> const Allocator& {
        return alloc_;
    }

    /// Forces the length of the vector to `new_len`.
    fn set_len(usize new_len) {
        if(new_len > capacity()) {
            throw std::out_of_range("Vec::set_len requires that new_len <= capacity()");
        }
        end_ = begin_ + new_len;
    }

    /// Removes an element from the vector and returns it.
    fn swap_remove(usize index) -> T {
        usize len_ = len();
        if (index >= len_) {
            throw std::out_of_range(
                std::format("swap_remove index (is {}) should be < len (is {})", index, len_)
            );
        }

        T removed_value = std::move(begin_[index]);
        if (index != len_ - 1) {
            begin_[index] = std::move(begin_[len_ - 1]);
        }
        end_--;
        return removed_value;
    }

    /// Inserts an element at position `index` within the vector, shifting all
    /// elements after it to the right.
    fn insert(usize index, T element) {
        insert_mut(index, std::move(element));
    }

    /// Inserts an element at position `index` within the vector, shifting all
    /// elements after it to the right, and returning a reference to the new
    /// element.
    fn insert_mut(usize index, T element) -> T* {
        usize len_ = len();
        if (index > len_) {
            throw std::out_of_range(
                std::format("insertion index (is {}) should be <= len (is {})", index, len_)
            );
        }

        reserve(1);

        T* insert_pos = begin_ + index;
        if (index < len_) {
            AllocTraits::construct(alloc_, end_, std::move(*(end_ - 1)));
            for (T* p = end_ - 1; p > insert_pos; --p) {
                *p = std::move(*(p - 1));
            }
            *insert_pos = std::move(element);
        } else {
            AllocTraits::construct(alloc_, insert_pos, std::move(element));
        }
        ++end_;
        return insert_pos;
    }

    /// Removes and returns the element at position `index` within the vector,
    /// shifting all elements after it to the left.
    fn remove(usize index) -> T {
        usize len_ = len();
        if (index >= len_) {
            throw std::out_of_range(
                std::format("removal index (is {}) should be < len (is {})", index, len_)
            );
        }

        T* remove_pos = begin_ + index;
        T removed_value = std::move(*remove_pos);

        for (T* p = remove_pos; p < end_ - 1; ++p) {
            *p = std::move(*(p + 1));
        }

        --end_;
        AllocTraits::destroy(alloc_, end_);

        return removed_value;
    }

    /// Remove and return the element at position `index` within the vector,
    /// shifting all elements after it to the left, or [`None`] if it does not
    /// exist.
    fn try_remove(usize index) -> Option<T> {
        if (index >= len()) {
            return None;
        }

        T* remove_pos = begin_ + index;
        T removed_value = std::move(*remove_pos);

        for (T* p = remove_pos; p < end_ - 1; ++p) {
            *p = std::move(*(p + 1));
        }

        --end_;
        AllocTraits::destroy(alloc_, end_);

        return Some(std::move(removed_value));
    }

    /// Retains only the elements specified by the predicate.
    ///
    /// In other words, remove all elements `e` for which `f(&e)` returns `false`.
    /// This method operates in place, visiting each element exactly once in the
    /// original order, and preserves the order of the retained elements.
    template<typename F>
    fn retain(F&& f) {
        retain_mut(f); // in rust its self.retain_mut(|elem| f(elem));
    }
    
    /// Retains only the elements specified by the predicate, passing a mutable reference to it.
    ///
    /// In other words, remove all elements `e` such that `f(&mut e)` returns `false`.
    /// This method operates in place, visiting each element exactly once in the
    /// original order, and preserves the order of the retained elements.
    template<typename F>
    fn retain_mut(F&& f) {
        T* write = begin_;
        for (T* read = begin_; read != end_; ++read) {
            if (std::invoke(f, *read)) {
                if (write != read) {
                    *write = std::move(*read);
                }
                ++write;
            } else {
                AllocTraits::destroy(alloc_, read);
            }
        }
        end_ = write;
    }

    /// Appends an element to the back of a collection.
    fn push(T value) {
        push_mut(std::move(value));
    }

    /// Appends an element if there is sufficient spare capacity, otherwise an error is returned
    /// with the element.
    fn push_within_capacity(T value) -> Result<UnitType, T> { // Result<(), T>
        if (len() < capacity()) {
            AllocTraits::construct(alloc_, end_, std::move(value));
            ++end_;
            return Ok<UnitType, T>(UnitType{});
        }
        return Err<UnitType, T>(std::move(value));
    }

    /// Appends an element to the back of a collection, returning a reference to it.
    fn push_mut(T value) -> T& {
        reserve(1);
        AllocTraits::construct(alloc_, end_, std::move(value));
        T& ref = *end_;
        ++end_;
        return ref;
    }

    /// Appends an element and returns a reference to it if there is sufficient spare capacity,
    /// otherwise an error is returned with the element.
    fn push_mut_within_capacity(T value) -> Result<T*, T> {
        if (len() < capacity()) {
            AllocTraits::construct(alloc_, end_, std::move(value));
            T* ptr = end_;
            ++end_;
            return Ok<T*, T>(ptr);
        }
        return Err<T*, T>(std::move(value));
    }

    /// Removes the last element from a vector and returns it, or [`None`] if it
    /// is empty.
    fn pop() -> Option<T> {
        if (len() == 0) {
            return None;
        }
        --end_;
        T value = std::move(*end_);
        AllocTraits::destroy(alloc_, end_);
        return Some(std::move(value));
    }

    /// Removes and returns the last element from a vector if the predicate
    /// returns `true`, or [`None`] if the predicate returns false or the vector
    /// is empty (the predicate will not be called in that case).
    template<typename Fn>
    fn pop_if(Fn&& predicate) -> Option<T> { // predicate: impl FnOnce(&mut T) -> bool
        if (len() == 0) {
            return None;
        }
        if (std::invoke(predicate, *(end_ - 1))) {
            --end_;
            T value = std::move(*end_);
            AllocTraits::destroy(alloc_, end_);
            return Some(std::move(value));
        }
        return None;
    }

    /// Moves all the elements of `other` into `self`, leaving `other` empty.
    fn append(Vec<T>& other) {
        reserve(other.len());
        for (T* p = other.begin_; p != other.end_; ++p) {
            AllocTraits::construct(alloc_, end_, std::move(*p));
            ++end_;
        }
        other.clear();
    }

    /// Appends elements to `self` from other buffer.
    fn append_elements(std::span<const T> other) { // unsafe fn append_elements(&mut self, other: *const [T]) {}
        reserve(other.size());
        for (const T& elem : other) {
            AllocTraits::construct(alloc_, end_, elem);
            ++end_;
        }
    }

    /// Clears the vector, removing all values.
    fn clear() {
        for (T* p = begin_; p != end_; ++p) {
            AllocTraits::destroy(alloc_, p);
        }
        end_ = begin_;
    }

    /// Returns the number of elements in the vector, also referred to
    /// as its 'length'.
    constexpr fn len() const -> usize {
        return static_cast<usize>(end_ - begin_);
    }
    
    /// Returns `true` if the vector contains no elements.
    fn is_empty() const -> bool {
        return len() == 0;
    }

private:
    fn reallocate(usize new_cap) {
        if (new_cap == 0) {
            if (begin_) {
                for (T* p = begin_; p != end_; ++p) {
                    AllocTraits::destroy(alloc_, p);
                }
                AllocTraits::deallocate(alloc_, begin_, capacity());
            }
            begin_ = nullptr;
            end_ = nullptr;
            cap_ = nullptr;
            return;
        }

        T* new_begin = AllocTraits::allocate(alloc_, new_cap);
        T* new_end = new_begin;

        for (T* p = begin_; p != end_; ++p) {
            AllocTraits::construct(alloc_, new_end, std::move(*p));
            AllocTraits::destroy(alloc_, p);
            ++new_end;
        }

        if (begin_) {
            AllocTraits::deallocate(alloc_, begin_, capacity());
        }

        begin_ = new_begin;
        end_ = new_end;
        cap_ = new_begin + new_cap;
    }

    [[no_unique_address]] Allocator alloc_;

    T* begin_;
    T* end_;
    T* cap_;
};

}; // namespace rstd
