#pragma once

#include <mutable/mutable.hpp>


struct ColumnStore : m::Store
{
    private:
    /* 1.3.1: Declare necessary fields. */
    size_t row_count = 0;
    std::vector<uintptr_t> columnBuffers;
    const size_t bufferSize = 10000000;  //TODO find a size

    public:
    ColumnStore(const m::Table &table);
    ~ColumnStore();

    std::size_t num_rows() const override;
    void append() override;
    void drop() override;

    void accept(m::StoreVisitor &v) override { v(*this); }
    void accept(m::ConstStoreVisitor &v) const override { v(*this); }

    void dump(std::ostream &out) const override;
    using Store::dump;
};
