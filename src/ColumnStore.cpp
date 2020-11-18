#include "ColumnStore.hpp"


ColumnStore::ColumnStore(const m::Table &table)
    : Store(table)
{
    /* TODO 1.3.1: Allocate columns for the attributes. */

    /* TODO 1.3.1: Allocate a column for the null bitmap. */

    /* TODO 1.3.2: Create linearization. */
}

ColumnStore::~ColumnStore()
{
    /* TODO 1.3.1: Free allocated memory. */
}

std::size_t ColumnStore::num_rows() const
{
    /* TODO 1.3.1: Implement */
}

void ColumnStore::append()
{
    /* TODO 1.3.1: Implement */
}

void ColumnStore::drop()
{
    /* TODO 1.3.1: Implement */
}

void ColumnStore::dump(std::ostream &out) const
{
    /* TODO 1.3: Print description of this store to `out`. */
}
