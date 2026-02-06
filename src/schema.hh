#pragma once

#include "utils.hh"

#include <bit>
#include <cassert>
#include <initializer_list>
#include <span>
#include <vector>
#include <algorithm>

namespace schema {

using     TypeHandle = u32;
constexpr TypeHandle INVALID_TYPE = ~TypeHandle(0);

enum class TypeKind : u8 {
    U64, U32, U16, U8,
    I64, I32, I16, I8,
    F64, F32,
    BOOL,
    NUM_PRIMITIVES,

    STRUCT,
};

struct Field {
    TypeHandle  type;         // Type of this field
    u32         byte_offset; // Byte offset within parent struct
    std::string name;
};

struct TypeMeta {
    u32 size;            // Total size in bytes
    u32 alignment;       // Alignment requirement
    TypeKind kind;       // Type discriminator
    u8 _pad[3];          // Padding for alignment
    u32 fields_offset;   // Index into fields_ for structs/composite types
    u32 fields_count;    // # of fields
};

class TypeRegistry {
public:
    explicit TypeRegistry(size_t reserve_types = 50) {
        types_.reserve(reserve_types);
        fields_.reserve(reserve_types * 4);

        init_primitives();
    }

    ~TypeRegistry() = default;
    TypeRegistry(const TypeRegistry& other) = delete;
    TypeRegistry(TypeRegistry&& other) = delete;
    TypeRegistry& operator=(const TypeRegistry& other) = delete;
    TypeRegistry& operator=(TypeRegistry&& other) = delete;

    [[nodiscard]]
    auto size_of(TypeHandle handle) const noexcept -> u64 {
        return types_[handle].size;
    }

    [[nodiscard]]
    auto align_of(TypeHandle handle) const noexcept -> u32 {
        return types_[handle].alignment;
    }

    [[nodiscard]]
    auto type_of(TypeHandle handle) const noexcept -> TypeKind {
        return types_[handle].kind;
    }

    [[nodiscard]]
    auto register_struct(std::initializer_list<std::pair<std::string, const TypeHandle>> field_types) -> TypeHandle {
        u32 total_size = 0;
        u32 max_align  = 1;

        const u32 field_base = static_cast<u32>(fields_.size()); // get the pos in the array
        fields_.reserve(fields_.size() + field_types.size());

        for (auto&& [name, type] : field_types) {
            const TypeMeta& field_meta  = types_[type];
            const u32       field_align = field_meta.alignment;

            total_size = align_up(total_size, field_align);

            fields_.push_back(Field {
                .type        = type,
                .byte_offset = total_size,
                .name        = name,
            });

            total_size += field_meta.size;
            max_align   = std::max(max_align, field_align);
        }

        total_size = align_up(total_size, max_align);

        return register_type(TypeMeta {
            .size        = total_size,
            .alignment   = max_align,
            .kind        = TypeKind::STRUCT,
            ._pad        = {0, 0, 0},
            .fields_offset = field_base,
            .fields_count  = static_cast<u32>(field_types.size())
        });
    }

    [[nodiscard]]
    auto fields_of(TypeHandle handle) const noexcept -> std::span<const Field> {
        const TypeMeta& meta = types_[handle];
        return { fields_.data() + meta.fields_offset, meta.fields_count };
    }

    [[nodiscard]]
    inline auto field_offset(TypeHandle handle, u32 idx) const noexcept -> u32 {
        const TypeMeta& meta = types_[handle];
        return fields_[meta.fields_offset + idx].byte_offset;
    }

    [[nodiscard]]
    inline auto field_type(TypeHandle handle, u32 idx) const noexcept -> TypeHandle {
        const TypeMeta& meta = types_[handle];
        return fields_[meta.fields_offset + idx].type;
    }

    [[nodiscard]]
    inline auto types() -> std::vector<TypeMeta>& { // indexed by TypeHandle
        return types_;
    }

    static constexpr TypeHandle U64  = 0;
    static constexpr TypeHandle U32  = 1;
    static constexpr TypeHandle U16  = 2;
    static constexpr TypeHandle U8   = 3;
    static constexpr TypeHandle I64  = 4;
    static constexpr TypeHandle I32  = 5;
    static constexpr TypeHandle I16  = 6;
    static constexpr TypeHandle I8   = 7;
    static constexpr TypeHandle F64  = 8;
    static constexpr TypeHandle F32  = 9;
    static constexpr TypeHandle BOOL = 10;

private:
    static constexpr auto align_up(u32 value, u32 alignment) noexcept -> u32 {
        assert(std::has_single_bit(alignment));
        return (value + alignment - 1) & ~(alignment - 1);
    }

    auto register_type(TypeMeta meta) -> TypeHandle {
        const TypeHandle handle = static_cast<TypeHandle>(types_.size());
        types_.push_back(meta);
        return handle;
    }

    auto init_primitives() -> void {
        register_type({8, 8, TypeKind::U64,  {}, 0, 0});  // 0
        register_type({4, 4, TypeKind::U32,  {}, 0, 0});  // 1
        register_type({2, 2, TypeKind::U16,  {}, 0, 0});  // 2
        register_type({1, 1, TypeKind::U8,   {}, 0, 0});  // 3
        register_type({8, 8, TypeKind::I64,  {}, 0, 0});  // 4
        register_type({4, 4, TypeKind::I32,  {}, 0, 0});  // 5
        register_type({2, 2, TypeKind::I16,  {}, 0, 0});  // 6
        register_type({1, 1, TypeKind::I8,   {}, 0, 0});  // 7
        register_type({8, 8, TypeKind::F64,  {}, 0, 0});  // 8
        register_type({4, 4, TypeKind::F32,  {}, 0, 0});  // 9
        register_type({1, 1, TypeKind::BOOL, {}, 0, 0});  // 10
    }

    std::vector<TypeMeta> types_;
    std::vector<Field>    fields_;
};

} // namespace schema
