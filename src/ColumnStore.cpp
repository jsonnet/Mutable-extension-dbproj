#include "ColumnStore.hpp"


ColumnStore::ColumnStore(const m::Table &table)
    : Store(table)
{
    /* TODO 1.3.1: Allocate columns for the attributes. */
    for (const auto &i : table){
        //Create a buffer for each column
        size_t rowSizeBytes = ceil((double) i.type->alignment() / 8);
        auto buffer = (uintptr_t) malloc(rowSizeBytes * 10000);
        columnBuffers.push_back(buffer);
    }

    /* TODO 1.3.1: Allocate a column for the null bitmap. */
    auto bitmap_column= std::make_unique<m::Linearization>(m::Linearization::CreateFinite(1,1));
    bitmap_column->add_null_bitmap(0,0);
    auto bitmap_buffer = (uintptr_t) malloc(ceil( (double) table.size() / 8 ));


    /* TODO 1.3.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(table.size()+1));

    auto buff_it = columnBuffers.cbegin();

    for (const auto &i : table){
        //Create a row for each attribute
        auto column = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(1,1));
        column->add_sequence(0, 0, i);

        //Create a buffer for each column
        size_t rowSizeBytes = ceil((double) i.type->size() / 8);

        lin->add_sequence(*buff_it, rowSizeBytes, std::move(column));
        ++buff_it;
    }
    lin->add_sequence(bitmap_buffer, ceil( (double) table.size() / 8 ), std::move(bitmap_column));

    linearization(std::move(lin));

}

ColumnStore::~ColumnStore()
{
    /* TODO 1.3.1: Free allocated memory. */
    for (const auto &i : columnBuffers){
        free((void *)i);
    }
}

std::size_t ColumnStore::num_rows() const
{
    /* TODO 1.3.1: Implement */
    return row_count;
}

void ColumnStore::append()
{
    /* TODO 1.3.1: Implement */
    ++row_count;
}

void ColumnStore::drop()
{
    /* TODO 1.3.1: Implement */
    --row_count;
}

void ColumnStore::dump(std::ostream &out) const
{
    /* TODO 1.3: Print description of this store to `out`. */
}
