#include "RowStore.hpp"


RowStore::RowStore(const m::Table &table)
        : Store(table)
{
    /* TODO 1.2.1: Allocate memory. */

    /* TODO 1.2.2: Create linearization. */
}

RowStore::~RowStore()
{
    /* TODO 1.2.1: Free allocated memory. */
}

std::size_t RowStore::num_rows() const
{
    /* TODO 1.2.1: Implement */
}

void RowStore::append()
{
    /* TODO 1.2.1: Implement */
}

void RowStore::drop()
{
    /* TODO 1.2.1: Implement */
}

void RowStore::dump(std::ostream &out) const
{
    /* TODO 1.2: Print description of this store to `out`. */
}
