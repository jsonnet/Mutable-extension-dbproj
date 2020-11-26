#include "RowStore.hpp"

using namespace rewire;

bool alignCompare(const std::tuple<size_t, size_t> &a, const std::tuple<size_t, size_t> &b) { return std::get<0>(a) > std::get<0>(b);}

RowStore::RowStore(const m::Table &table)
        : Store(table) {
    /* TODO 1.2.1: Allocate memory. */
    std::size_t numAttributes = table.size();  //amount of attributes
    std::size_t current_offset = 0;
    size_t current_biggest_align = 0;

    std::vector<std::tuple<size_t , size_t>> toSort;
    size_t counter = 0;

    // Check each attribute in table
    for (const auto &i : table) {
        current_offset += i.type->size(); // Attr sze added to current offset in bits
        toSort.push_back(std::make_tuple(i.type->alignment(), counter++));
        // Take the biggest attr alignment
        if (i.type->alignment() > current_biggest_align) {
            current_biggest_align = i.type->alignment();
        }

    }

    //sort attributes with alignment descending
    std::sort(toSort.begin(), toSort.end(), alignCompare);


    // ceil(sum of all attribute bits needed + bitmap bits)
    size_t row_bits_total = current_offset + numAttributes;

    row_total_bytes = row_bits_total % 8 == 0 ? (row_bits_total / 8) : (row_bits_total / 8) + 1;

    auto bytes_first_elem = (size_t) ceil(
            (double) table[(size_t) 0].type->size() / 8);  //changes this to element of first
    bytes_first_elem = (size_t) ceil(
            (double) current_biggest_align / 8);  //changes this to element of first


    auto paddRowSize = (bytes_first_elem - (row_total_bytes % bytes_first_elem)) % bytes_first_elem;
    std::size_t master_stride_bytes = row_total_bytes + paddRowSize;

    //address = (uintptr_t) malloc(master_stride_bytes * 200000);
    address = (uintptr_t) malloc(master_stride_bytes * 10000000); // 10 MB

    // FIXME TODO
    Memory mem;
    mem.allocator();
    //mem.map();

    /* TODO 1.2.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    //Create the row
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(numAttributes + 1, 1));
    size_t offset = 0;
    for (const auto &i : toSort) {
        row->add_sequence(offset, 0, table[std::get<1>(i)]);
        offset += table[std::get<1>(i)].type->size();
    }
    row->add_null_bitmap(offset, 0);
    rows_used = 0;

    lin->add_sequence(address, master_stride_bytes, std::move(row));
    linearization(std::move(lin));
}

RowStore::~RowStore() {
    /* TODO 1.2.1: Free allocated memory. */
    free((void *) address);
}

std::size_t RowStore::num_rows() const {
    /* TODO 1.2.1: Implement */
    return rows_used;
}

//Append another tuple row dynamically
void RowStore::append() {
    /* TODO 1.2.1: Implement */
    rows_used++;
}

void RowStore::drop() {
    /* TODO 1.2.1: Implement */
    rows_used--;
}

void RowStore::dump(std::ostream &out) const {
    /* TODO 1.2: Print description of this store to `out`. */
    out << "test";
}
