#include "ColumnStore.hpp"

ColumnStore::ColumnStore(const m::Table &table)
        : Store(table) {

    // Initial rows
    storable_in_buffer = 10;

    /* 1.3.1: Allocate columns for the attributes. */
    for (const auto &i : table) {
        // Create a buffer for each column/attribute
        size_t rowSizeBytes = ceil((double) i.type->size() / 8);
        auto buffer = malloc(rowSizeBytes * storable_in_buffer);
        columnBuffers.push_back(buffer);
    }

    /* 1.3.1: Allocate a column for the null bitmap. */
    bitmap_buffer = malloc(ceil((double) this->table().size() / 8) *
                           storable_in_buffer); //buffer for a bitmap for each tuple inserted with num of attributes bits each

    createLin();
}

ColumnStore::~ColumnStore() {
    /* 1.3.1: Free allocated memory. */
    for (const auto &i : columnBuffers)
        free((void *) i);
    free(bitmap_buffer);
}

std::size_t ColumnStore::num_rows() const {
    /* 1.3.1: Implement */
    return row_count;
}

void ColumnStore::append() {
    /* 1.3.1: Implement */
    // Increase used rows
    ++row_count;

    // Check if enough memory is pre allocated
    if (row_count < storable_in_buffer) return;
    // If not allocate 1.5*old_size (aka Java ArrayList)
    storable_in_buffer = storable_in_buffer + (storable_in_buffer >> 1u);

    // Create iterator over old buffers (to reallocate)
    auto buff_it = columnBuffers.cbegin();
    std::vector<void *> newBuffers;

    for (const auto &i : table()) {
        // For each attribute realloc
        size_t rowSizeBytes = ceil((double) i.type->size() / 8);
        auto buffer = realloc(*buff_it, rowSizeBytes * storable_in_buffer);
        newBuffers.push_back(buffer);

        ++buff_it;
    }

    // Overwrite with new vector
    columnBuffers = newBuffers;

    /* 1.3.1: Allocate a column for the null bitmap. */
    bitmap_buffer = realloc(bitmap_buffer, ceil((double) this->table().size() / 8) *
                                           storable_in_buffer); //buffer for a bitmap for each tuple inserted with num of attributes bits each

    createLin();
}

void ColumnStore::drop() {
    /* 1.3.1: Implement */
    //TODO decrease memory dynamically for all
    --row_count;
}

void ColumnStore::dump(std::ostream &out) const {
    /* TODO 1.3: Print description of this store to `out`. */
    out << "Some useful data" << std::endl;
}

/** A custom function to create a linearization, but you need to fill columnBuffers and bitmap_buffer first **/
void ColumnStore::createLin() {
    /* 1.3.1: Allocate a column for the null bitmap. */
    auto bitmap_column = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(1, 1));
    bitmap_column->add_null_bitmap(0, 0);

    /* 1.3.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(this->table().size() + 1));

    // Get the iterator for the buffer
    auto buff_it = columnBuffers.cbegin();

    for (const auto &i : table()) {
        // Create a row for each column/attribute (column_row)
        auto column = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(1, 1));
        column->add_sequence(0, 0, i);

        // Add the columns to the linearization (address space from buffer)
        size_t rowSizeBytes = ceil((double) i.type->size() / 8);
        lin->add_sequence(uint64_t(reinterpret_cast<uintptr_t>(*buff_it)), rowSizeBytes, std::move(column));

        // Advance iterator
        ++buff_it;
    }
    // Finally add the null bitmap
    lin->add_sequence(uint64_t(reinterpret_cast<uintptr_t>(bitmap_buffer)), ceil((double) table().size() / 8),
                      std::move(bitmap_column));

    linearization(std::move(lin));
}
