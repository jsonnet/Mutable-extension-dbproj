#include "RowStore.hpp"
#include <cstdlib>

using namespace rewire;

// Custom compare method to make sure only to compare the first item in the tuple
bool alignCompare(const std::tuple<size_t, size_t> &a, const std::tuple<size_t, size_t> &b) {
    return std::get<0>(a) > std::get<0>(b);
}

RowStore::RowStore(const m::Table &table)
        : Store(table) {
    /* 1.2.1: Allocate memory. */
    std::size_t numAttributes = table.size();  //amount of attributes
    std::size_t current_offset = 0;
    size_t current_biggest_align = 0;

    //Set first buffer to size of 10 rows
    storable_in_buffer = 10;

    size_t index_counter = 0;
    // Check each attribute in table
    for (const auto &i : table) {
        // Attribute size added to current offset in bits
        current_offset += i.type->size();

        // Save a tuple containing the alignment size and index
        toSort.push_back(std::make_tuple(i.type->alignment(), index_counter++));

        // Save the largest attribute alignment
        if (i.type->alignment() > current_biggest_align)
            current_biggest_align = i.type->alignment();
    }

    // Sort attributes with alignment descending
    std::sort(toSort.begin(), toSort.end(), alignCompare);

    // Total size of a row consisting of the sum of each attributes size and the no. of attributes
    size_t row_total_bits = current_offset + numAttributes;
    // Ceiled value of the row bits in bytes
    row_total_bytes = row_total_bits % 8 == 0 ? (row_total_bits / 8) : (row_total_bits / 8) + 1;

    // ?changes this to element of first?
    auto bytes_first_elem = (size_t) ceil((double) current_biggest_align / 8);
    auto paddRowSize = (bytes_first_elem - (row_total_bytes % bytes_first_elem)) % bytes_first_elem;
    master_stride_bytes = row_total_bytes + paddRowSize;

    // Allocate memory (just with an initial size)
    address = malloc(master_stride_bytes * storable_in_buffer);

    /* 1.2.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    // Create the row and add the correct attribute based on the sorted list
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(numAttributes + 1, 1));

    size_t offset = 0;
    for (const auto &i : toSort) {
        row->add_sequence(offset, 0, table[std::get<1>(i)]);
        offset += table[std::get<1>(i)].type->size();
    }

    // Add null bitmap
    row->add_null_bitmap(offset, 0);

    // Finalize linearization at allocated memory
    lin->add_sequence(uint64_t(reinterpret_cast<uintptr_t>(address)), master_stride_bytes, std::move(row));
    linearization(std::move(lin));
}

RowStore::~RowStore() {
    /* 1.2.1: Free allocated memory. */
    free((void *) address);
}

std::size_t RowStore::num_rows() const {
    /* 1.2.1: Implement */
    return rows_used;
}

//Append another tuple row dynamically
void RowStore::append() {
    /* 1.2.1: Implement */
    // Increase row size
    rows_used++;

    // if we have enough storage left in buffer -> all good
    if (rows_used < storable_in_buffer) return;

    //if not -> grow buffer size, 1.5*old_size (aka nearly golden ratio)
    previous_buffer_size = storable_in_buffer;
    storable_in_buffer = storable_in_buffer + (storable_in_buffer >> 1u);

    // realloc new memory and create a new linearization
    address = realloc(address, master_stride_bytes * storable_in_buffer);
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    // Create the row and add the correct attribute based on the sorted list
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(this->table().size() + 1, 1));

    size_t offset = 0;
    for (const auto &i : toSort) {
        row->add_sequence(offset, 0, this->table()[std::get<1>(i)]);
        offset += this->table()[std::get<1>(i)].type->size();
    }

    // Add null bitmap
    row->add_null_bitmap(offset, 0);

    lin->add_sequence(uint64_t(reinterpret_cast<uintptr_t>(address)), master_stride_bytes, std::move(row));
    linearization(std::move(lin));
}

void RowStore::drop() {
    /* 1.2.1: Implement */
    rows_used--;

    // if we have enough storage left in buffer -> all good
    if (rows_used > previous_buffer_size) return;

    //if not -> grow buffer size, 1.5*old_size (aka nearly golden ratio)
    storable_in_buffer = previous_buffer_size;
    previous_buffer_size = ceil(storable_in_buffer / 1.5);

    // realloc new memory and create a new linearization
    address = realloc(address, master_stride_bytes * storable_in_buffer);
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    // Create the row and add the correct attribute based on the sorted list
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(this->table().size() + 1, 1));

    size_t offset = 0;
    for (const auto &i : toSort) {
        row->add_sequence(offset, 0, this->table()[std::get<1>(i)]);
        offset += this->table()[std::get<1>(i)].type->size();
    }

    // Add null bitmap
    row->add_null_bitmap(offset, 0);

    lin->add_sequence(uint64_t(reinterpret_cast<uintptr_t>(address)), master_stride_bytes, std::move(row));
    linearization(std::move(lin));
}

void RowStore::dump(std::ostream &out) const {
    /* TODO 1.2: Print description of this store to `out`. */
    out << "Some useful data" << std::endl;
}
