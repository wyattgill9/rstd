#include <cassert>
#include <format>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>

#include "macros.hh"
#include "option.hh"
#include "types.hh"

#include "result.hh"

namespace rstd {

// TODO -> dont do any of the functions that use NonNull<T>, only T* should be considered
// No reason for any of the NonNull functions -> @NonNull

template<
    typename T,
    class Allocator = std::allocator<T>
>
class Vec {
    using AllocTraits = std::allocator_traits<Allocator>;

public:
    // Constructors - FIX -> Decide if nessasary
    Vec() : begin_(nullptr), end_(nullptr), cap_(nullptr) {}
    // Copy constructor
    Vec(const Vec& other) {}
    // Move constructor
    constexpr Vec(Vec&& other)  {}
    // Destructor
    constexpr ~Vec() {}
    // Copy assignment
    constexpr fn operator=(const Vec& other) -> Vec& {}
    // Move assignment
    constexpr fn operator=(Vec&& other) noexcept -> Vec& {}

    // Functions
    static fn with_capacity(usize capacity) -> Vec {
        Vec v;
        if(capacity > 0) {
            v.begin_ = AllocTraits::allocate(v.alloc_, capacity);
            v.end_ = v.begin_;
            v.cap_ = v.begin_ + capacity;
        }
        return v;
    }

    /// Constructs a new, empty `Vec<T>` with at least the specified capacity.
    // pub fn try_with_capacity(capacity: usize) -> Result<Self, TryReserveError> {

    /// Constructs a new, empty `Vec<T>` with at least the specified capacity.
    // `static fn try_with_capacity(usize capacity) -> Vec {}`

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

    // @NonNull
    /// Creates a `Vec<T>` directly from a `NonNull` pointer, a length, and a capacity.
    // `static fn from_parts(NonNull<T> ptr, usize lenght, usize capacity) -> Vec {}`

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
    
    // @NonNull
    // pub unsafe fn from_parts_in(ptr: NonNull<T>, length: usize, capacity: usize, alloc: A) -> Self {

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

    // @NonNull
    // pub fn into_parts_with_alloc(self) -> (NonNull<T>, usize, usize, A) {

    /// Returns the total number of elements the vector can hold without
    /// reallocating.
    constexpr fn capacity() -> usize {
        return static_cast<usize>(begin_ - cap_);
    }

    /// Reserves capacity for at least `additional` more elements to be inserted
    /// in the given `Vec<T>`. The collection may reserve more space to
    /// speculatively avoid frequent reallocations. After calling `reserve`,
    /// capacity will be greater than or equal to `self.len() + additional`.
    /// Does nothing if capacity is already sufficient.
    fn reserve(usize additional) {
        
    }
    
    /// Reserves the minimum capacity for at least `additional` more elements to
    /// be inserted in the given `Vec<T>`. Unlike [`reserve`], this will not
    /// deliberately over-allocate to speculatively avoid frequent allocations.
    /// After calling `reserve_exact`, capacity will be greater than or equal to
    /// `self.len() + additional`. Does nothing if the capacity is already
    /// sufficient.
    fn reserve_exact(usize additional) {
        
    }

    /// Tries to reserve capacity for at least `additional` more elements to be inserted
    /// in the given `Vec<T>`. The collection may reserve more space to speculatively avoid
    /// frequent reallocations. After calling `try_reserve`, capacity will be
    /// greater than or equal to `self.len() + additional` if it returns
    /// `Ok(())`. Does nothing if capacity is already sufficient. This method
    /// preserves the contents even if an error occurs.
    // pub fn try_reserve(&mut self, additional: usize) -> Result<(), TryReserveError> {

    /// Tries to reserve the minimum capacity for at least `additional`
    /// elements to be inserted in the given `Vec<T>`. Unlike [`try_reserve`],
    /// this will not deliberately over-allocate to speculatively avoid frequent
    /// allocations. After calling `try_reserve_exact`, capacity will be greater
    /// than or equal to `self.len() + additional` if it returns `Ok(())`.
    /// Does nothing if the capacity is already sufficient.
    // pub fn try_reserve_exact(&mut self, additional: usize) -> Result<(), TryReserveError> {

    /// Shrinks the capacity of the vector as much as possible.
    fn shrink_to_fit() {
        if(capacity() > len()) {
            cap_ = end_; // ?
        }
    }

    /// Shrinks the capacity of the vector with a lower bound.
    fn shrink_to(usize min_capacity) {
        if(capacity() > len()) {
            // shrink to std::max(len(), min_capacity)
        }
    }

    //pub fn into_boxed_slice(mut self) -> Box<[T], A> {} // A is allocator
    fn into_boxed_slice() -> std::unique_ptr<T[]> {
        shrink_to_fit();
        std::unique_ptr<T[]> slice(begin_);

        return slice;
    }

    /// Shortens the vector, keeping the first `len` elements and dropping
    /// the rest.
    ///
    /// If `len` is greater or equal to the vector's current length, this has
    /// no effect.
    ///
    /// The [`drain`] method can emulate `truncate`, but causes the excess
    /// elements to be returned instead of dropped.
    ///
    /// Note that this method has no effect on the allocated capacity
    /// of the vector.
    ///
    /// # Examples
    ///
    /// Truncating a five element vector to two elements:
    ///
    /// ```
    /// let mut vec = vec![1, 2, 3, 4, 5];
    /// vec.truncate(2);
    /// assert_eq!(vec, [1, 2]);
    /// ```
    ///
    /// No truncation occurs when `len` is greater than the vector's current
    /// length:
    ///
    /// ```
    /// let mut vec = vec![1, 2, 3];
    /// vec.truncate(8);
    /// assert_eq!(vec, [1, 2, 3]);
    /// ```
    ///
    /// Truncating when `len == 0` is equivalent to calling the [`clear`]
    /// method.
    ///
    /// ```
    /// let mut vec = vec![1, 2, 3];
    /// vec.truncate(0);
    /// assert_eq!(vec, []);
    /// ```
    ///
    /// [`clear`]: Vec::clear
    /// [`drain`]: Vec::drain
    fn truncate(usize new_len) {
        if(new_len > len()) {
            return;
        }
        usize remaining_length = new_len - len();
        // we need to drop the rest pointer
        // let s = ptr::slice_from_raw_parts_mut(self.as_mut_ptr().add(len), remaining_len);
        end_ = begin_ + new_len;
    }

    fn as_slice() -> std::span<const T> {
        return std::span(begin_, len());
    }

    fn as_mut_slice() -> std::span<T> {
        return std::span(begin_, len());
    }

    fn as_ptr() -> const T* {
        return begin_;
    }

    fn as_mut_ptr() -> T* {
        return begin_;
    }

    // @NonNull
    // pub const fn as_non_null(&mut self) -> NonNull<T> {
    
    fn allocator() -> Allocator& {
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

    }

    /// Inserts an element at position `index` within the vector, shifting all
    /// elements after it to the right, and returning a reference to the new
    /// element.
    fn insert_mut(usize index, T element) -> T* {
        
    }

    /// Removes and returns the element at position `index` within the vector,
    /// shifting all elements after it to the left.
    fn remove(usize index) -> T {
        
    }
    
    /// Remove and return the element at position `index` within the vector,
    /// shifting all elements after it to the left, or [`None`] if it does not
    /// exist.
    fn try_remove(usize index) -> Option<T> {
        
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
        
    }

    /// Removes all but the first of consecutive elements in the vector that resolve to the same
    /// key.
    ///
    /// If the vector is sorted, this removes all duplicates.
    // #[stable(feature = "dedup_by", since = "1.16.0")]
    // #[inline]
    // pub fn dedup_by_key<F, K>(&mut self, mut key: F)
    // where
    //     F: FnMut(&mut T) -> K,
    //     K: PartialEq,
    // {
    //     self.dedup_by(|a, b| key(a) == key(b))
    // }
    // fn dedup_by_key(F key) {
    //
    // }


    // pub fn dedup_by<F>(&mut self, mut same_bucket: F)
    // where
    //     F: FnMut(&mut T, &mut T) -> bool,
    // {
    //     let len = self.len();
    //     if len <= 1 {
    //         return;
    //     }

    //     // Check if we ever want to remove anything.
    //     // This allows to use copy_non_overlapping in next cycle.
    //     // And avoids any memory writes if we don't need to remove anything.
    //     let mut first_duplicate_idx: usize = 1;
    //     let start = self.as_mut_ptr();
    //     while first_duplicate_idx != len {
    //         let found_duplicate = unsafe {
    //             // SAFETY: first_duplicate always in range [1..len)
    //             // Note that we start iteration from 1 so we never overflow.
    //             let prev = start.add(first_duplicate_idx.wrapping_sub(1));
    //             let current = start.add(first_duplicate_idx);
    //             // We explicitly say in docs that references are reversed.
    //             same_bucket(&mut *current, &mut *prev)
    //         };
    //         if found_duplicate {
    //             break;
    //         }
    //         first_duplicate_idx += 1;
    //     }
    //     // Don't need to remove anything.
    //     // We cannot get bigger than len.
    //     if first_duplicate_idx == len {
    //         return;
    //     }

    //     /* INVARIANT: vec.len() > read > write > write-1 >= 0 */
    //     struct FillGapOnDrop<'a, T, A: core::alloc::Allocator> {
    //         /* Offset of the element we want to check if it is duplicate */
    //         read: usize,

    //         /* Offset of the place where we want to place the non-duplicate
    //          * when we find it. */
    //         write: usize,

    //         /* The Vec that would need correction if `same_bucket` panicked */
    //         vec: &'a mut Vec<T, A>,
    //     }

    //     impl<'a, T, A: core::alloc::Allocator> Drop for FillGapOnDrop<'a, T, A> {
    //         fn drop(&mut self) {
    //             /* This code gets executed when `same_bucket` panics */

    //             /* SAFETY: invariant guarantees that `read - write`
    //              * and `len - read` never overflow and that the copy is always
    //              * in-bounds. */
    //             unsafe {
    //                 let ptr = self.vec.as_mut_ptr();
    //                 let len = self.vec.len();

    //                 /* How many items were left when `same_bucket` panicked.
    //                  * Basically vec[read..].len() */
    //                 let items_left = len.wrapping_sub(self.read);

    //                 /* Pointer to first item in vec[write..write+items_left] slice */
    //                 let dropped_ptr = ptr.add(self.write);
    //                 /* Pointer to first item in vec[read..] slice */
    //                 let valid_ptr = ptr.add(self.read);

    //                 /* Copy `vec[read..]` to `vec[write..write+items_left]`.
    //                  * The slices can overlap, so `copy_nonoverlapping` cannot be used */
    //                 ptr::copy(valid_ptr, dropped_ptr, items_left);

    //                 /* How many items have been already dropped
    //                  * Basically vec[read..write].len() */
    //                 let dropped = self.read.wrapping_sub(self.write);

    //                 self.vec.set_len(len - dropped);
    //             }
    //         }
    //     }

    //     /* Drop items while going through Vec, it should be more efficient than
    //      * doing slice partition_dedup + truncate */

    //     // Construct gap first and then drop item to avoid memory corruption if `T::drop` panics.
    //     let mut gap =
    //         FillGapOnDrop { read: first_duplicate_idx + 1, write: first_duplicate_idx, vec: self };
    //     unsafe {
    //         // SAFETY: we checked that first_duplicate_idx in bounds before.
    //         // If drop panics, `gap` would remove this item without drop.
    //         ptr::drop_in_place(start.add(first_duplicate_idx));
    //     }

    //     /* SAFETY: Because of the invariant, read_ptr, prev_ptr and write_ptr
    //      * are always in-bounds and read_ptr never aliases prev_ptr */
    //     unsafe {
    //         while gap.read < len {
    //             let read_ptr = start.add(gap.read);
    //             let prev_ptr = start.add(gap.write.wrapping_sub(1));

    //             // We explicitly say in docs that references are reversed.
    //             let found_duplicate = same_bucket(&mut *read_ptr, &mut *prev_ptr);
    //             if found_duplicate {
    //                 // Increase `gap.read` now since the drop may panic.
    //                 gap.read += 1;
    //                 /* We have found duplicate, drop it in-place */
    //                 ptr::drop_in_place(read_ptr);
    //             } else {
    //                 let write_ptr = start.add(gap.write);

    //                 /* read_ptr cannot be equal to write_ptr because at this point
    //                  * we guaranteed to skip at least one element (before loop starts).
    //                  */
    //                 ptr::copy_nonoverlapping(read_ptr, write_ptr, 1);

    //                 /* We have filled that place, so go further */
    //                 gap.write += 1;
    //                 gap.read += 1;
    //             }
    //         }

    //         /* Technically we could let `gap` clean up with its Drop, but
    //          * when `same_bucket` is guaranteed to not panic, this bloats a little
    //          * the codegen, so we just do it manually */
    //         gap.vec.set_len(gap.write);
    //         mem::forget(gap);
    //     }
    // }
    //
    // fn dedup_by() {
        
    // }
           
    /// Appends an element to the back of a collection.
    fn push(T value) {
        push_mut(value);
    }
    
    /// Appends an element if there is sufficient spare capacity, otherwise an error is returned
    /// with the element.
    fn push_within_capacity(T value) -> Result<UnitType, T> { // Result<(), T>
        
    }

    /// Appends an element to the back of a collection, returning a reference to it.
    fn push_mut(T value) -> T& {
        
    }

    /// Appends an element and returns a reference to it if there is sufficient spare capacity,
    /// otherwise an error is returned with the element.
    fn push_mut_within_capacity() -> Result<T*, T> {
        
    }

    /// Removes the last element from a vector and returns it, or [`None`] if it
    /// is empty.
    fn pop() -> Option<T> {
        if(len() == 0) {
            return Option<T>(None);
        } else {
            
        }
    }

    /// Removes and returns the last element from a vector if the predicate
    /// returns `true`, or [`None`] if the predicate returns false or the vector
    /// is empty (the predicate will not be called in that case).
    template<typename Fn>
    fn pop_if(Fn&& predicate) -> Option<T> { // predicate: impl FnOnce(&mut T) -> bool
        
    }

    // NO PEEKING!!!!
    /// Returns a mutable reference to the last item in the vector, or
    /// `None` if it is empty.
    // pub fn peek_mut(&mut self) -> Option<PeekMut<'_, T, A>> {}

    /// Moves all the elements of `other` into `self`, leaving `other` empty.
    fn append(Vec<T> other) {
        
    }

    /// Appends elements to `self` from other buffer.
    fn append_elements(std::span<const T> other) { // unsafe fn append_elements(&mut self, other: *const [T]) {}
        
    }

    /// Removes the subslice indicated by the given range from the vector,
    /// returning a double-ended iterator over the removed subslice.
    ///
    /// If the iterator is dropped before being fully consumed,
    /// it drops the remaining removed elements.
    ///
    /// The returned iterator keeps a mutable borrow on the vector to optimize
    /// its implementation.
    // #[stable(feature = "drain", since = "1.6.0")]
    // pub fn drain<R>(&mut self, range: R) -> Drain<'_, T, A>
    // where
    //     R: RangeBounds<usize>,
    // {
    //     // Memory safety
    //     //
    //     // When the Drain is first created, it shortens the length of
    //     // the source vector to make sure no uninitialized or moved-from elements
    //     // are accessible at all if the Drain's destructor never gets to run.
    //     //
    //     // Drain will ptr::read out the values to remove.
    //     // When finished, remaining tail of the vec is copied back to cover
    //     // the hole, and the vector length is restored to the new length.
    //     //
    //     let len = self.len();
    //     let Range { start, end } = slice::range(range, ..len);

    //     unsafe {
    //         // set self.vec length's to start, to be safe in case Drain is leaked
    //         self.set_len(start);
    //         let range_slice = slice::from_raw_parts(self.as_ptr().add(start), end - start);
    //         Drain {
    //             tail_start: end,
    //             tail_len: len - end,
    //             iter: range_slice.iter(),
    //             vec: NonNull::from(self),
    //         }
    //     }
    // }
    // I have no fucking idea how to implement this without some complex other class, so you decide if its genuinly
    // worth our time, if not just dont implement it or if there something similar in C++ do that but probs not
    // template<typename R>
    // fn drain() {
        
    // }

    /// Clears the vector, removing all values.
    fn clear() {
        
    }

    /// Returns the number of elements in the vector, also referred to
    /// as its 'length'.
    constexpr fn len() -> usize {
        return static_cast<usize>(end_ - begin_);
    }
    
    /// Returns `true` if the vector contains no elements.
    fn is_empty() -> bool {
        return len() == 0;
    }

    /// Splits the collection into two at the given index.
    ///
    /// Returns a newly allocated vector containing the elements in the range
    /// `[at, len)`. After the call, the original vector will be left containing
    /// the elements `[0, at)` with its previous capacity unchanged.
    ///
    /// - If you want to take ownership of the entire contents and capacity of
    ///   the vector, see [`mem::take`] or [`mem::replace`].
    /// - If you don't need the returned vector at all, see [`Vec::truncate`].
    /// - If you want to take ownership of an arbitrary subslice, or you don't
    ///   necessarily want to store the removed items in a vector, see [`Vec::drain`].

        
private:
    [[no_unique_address]] Allocator alloc_;

    T* begin_;
    T* end_;
    T* cap_;
};

}; // namespace rstd
