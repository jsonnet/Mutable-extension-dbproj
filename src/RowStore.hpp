#pragma once

#include <mutable/mutable.hpp>
#include <mutable/util/memory.hpp>


struct RowStore : m::Store
{
    private:
    /* 1.2.1: Declare necessary fields. */
    uintptr_t address;
    size_t rows_used;
    size_t row_total_bytes;

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
