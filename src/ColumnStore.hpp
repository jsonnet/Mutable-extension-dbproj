#pragma once

#include <mutable/mutable.hpp>


struct ColumnStore : m::Store
{
    private:
    /* TODO 1.3.1: Declare necessary fields. */

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
