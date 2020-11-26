#pragma once

#include <mutable/mutable.hpp>
#include <mutable/util/memory.hpp>


struct RowStore : m::Store
{
    private:
    /* TODO 1.2.1: Declare necessary fields. */

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
