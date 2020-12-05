#pragma once

#include <mutable/mutable.hpp>
#include <mutable/util/memory.hpp>

struct RowStore : m::Store
{
    private:
    /* 1.2.1: Declare necessary fields. */
    void* address;
    size_t rows_used = 0;
    size_t row_total_bytes;
    size_t storable_in_buffer;
    size_t previous_buffer_size;

    std::size_t master_stride_bytes;
    // List of all attributes to be sorted
    std::vector<std::tuple<size_t, size_t>> toSort;

    public:
    RowStore(const m::Table &table);
    ~RowStore();

    std::size_t num_rows() const override;
    void append() override;
    void drop() override;

    void accept(m::StoreVisitor &v) override { v(*this); }
    void accept(m::ConstStoreVisitor &v) const override { v(*this); }

    void dump(std::ostream &out) const override;
    using Store::dump;

};
