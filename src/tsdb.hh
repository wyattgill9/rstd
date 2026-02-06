#pragma once

#include <absl/container/flat_hash_map.h>

#include "schema.hh"

#include <cassert>
#include <cstring>
#include <span>
#include <vector>

class Column {
public:
    explicit Column(std::string name, u32 element_size)
        : data_()
        , elem_size_(element_size)
        , name_(std::move(name))
    {
        assert(elem_size_ > 0);
    }

    auto append(const std::byte* source) -> void {
        assert(source != nullptr);
        data_.insert(data_.end(), source, source + elem_size_);
    }

    [[nodiscard]] auto name() const noexcept -> const std::string& { return name_; }
    [[nodiscard]] auto elem_size() const noexcept -> u32           { return elem_size_; }
    [[nodiscard]] auto num_rows() const noexcept -> const size_t   { return data_.size() / elem_size_; }

    [[nodiscard]] auto row_ptr(size_t row_idx) const noexcept -> const std::byte* {
        return data_.data() + row_idx * elem_size_;
    }
 
private:
    std::vector<std::byte> data_;
    u32                    elem_size_;  // bytes per element (fixed)
    std::string            name_;
};

class Table {
public:
    Table(schema::TypeHandle struct_type, schema::TypeRegistry& registry)
        : registry_(registry)
        , struct_type_(struct_type)
        , struct_size_(static_cast<u32>(registry_.size_of(struct_type)))
    {
        assert(registry_.type_of(struct_type_) == schema::TypeKind::STRUCT);

        auto fields = registry_.fields_of(struct_type_);
        columns_.reserve(fields.size());

        for (const schema::Field& field : fields) {
            columns_.emplace_back(
                field.name,
                static_cast<u32>(registry_.size_of(field.type))
            );
        }
    }

    //  struct -> member columns
    // `data` must point to a valid instance of `struct_type_`.
    auto insert(const std::byte* data) -> void {
        assert(data != nullptr);

        auto fields = registry_.fields_of(struct_type_);
        assert(fields.size() == columns_.size());

        for (u32 i = 0; i < fields.size(); i++) {
            const std::byte* field_data = data + fields[i].byte_offset;
            columns_[i].append(field_data);
        }
    }

    // return every type stored in the table (in order)
    auto query_all_into(std::byte* dest) const -> size_t {
        const size_t row_count = num_rows();
        if (row_count == 0) {
            return 0;
        }

        auto fields = registry_.fields_of(struct_type_);

        for (size_t row = 0; row < row_count; row++) {
            std::byte* row_dst = dest + row * struct_size_;
            
            for (u32 col = 0; col < columns_.size(); col++) {
                std::memcpy(
                    row_dst + fields[col].byte_offset,
                    columns_[col].row_ptr(row),
                    columns_[col].elem_size()
                );
            }
        }

        return row_count;
    }

private:
    [[nodiscard]] auto num_rows() const noexcept -> size_t {
        return columns_.empty() ? 0 : columns_[0].num_rows();
    }
    
    schema::TypeRegistry& registry_;
    schema::TypeHandle    struct_type_;
    u32                   struct_size_;
    std::vector<Column>   columns_;
};

class TSDB {
public:
    explicit TSDB(schema::TypeRegistry& registry)
        : registry_(registry)
    {}

    auto insert(const std::byte* data, schema::TypeHandle type) -> void {
        assert(data != nullptr);
        assert(registry_.type_of(type) == schema::TypeKind::STRUCT);

        Table& table = get_or_create_table(type);
        table.insert(data);
    }

    auto query_all_into(schema::TypeHandle type, std::byte* dest) const -> size_t {
        return tables_.at(type).query_all_into(dest);
    }

private:
    auto get_or_create_table(schema::TypeHandle type) -> Table& {
        auto [it, inserted] = tables_.try_emplace(type, type, registry_);
        return it->second;
    }

    schema::TypeRegistry&                          registry_;
    absl::flat_hash_map<schema::TypeHandle, Table> tables_;
};
