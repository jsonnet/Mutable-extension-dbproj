#include "RowStore.hpp"

RowStore::RowStore(const m::Table &table)
        : Store(table)
{
    /* TODO 1.2.1: Allocate memory. */
    std::size_t numAttributes = table.size();  //amount of attributes
    std::size_t current_offset = 0;

    for(std::size_t i=0; i<table.size(); ++i){
        current_offset += table[i].type->size(); //in bits
    }


    //round_up(sum of all attribute bits needed + bitmap bits)
    size_t row_bits_total = current_offset + numAttributes;
    size_t row_total_bytes = row_bits_total % 8 == 0 ? (row_bits_total / 8) : (row_bits_total / 8) +1;

    auto bytes_first_elem = table[(size_t)0].type->size() / 8;  //changes this to element of first
    auto paddRowSize = (bytes_first_elem - ( row_total_bytes % bytes_first_elem)) % bytes_first_elem;
    std::size_t master_stride_bytes = row_total_bytes + paddRowSize;

    auto address = reinterpret_cast<uintptr_t>(malloc(row_total_bytes * 100000));

    /* TODO 1.2.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    //Create the row
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(numAttributes+1,1));
    size_t offset = 0;
    for(std::size_t i=0; i<table.size(); ++i){
        row->add_sequence(offset, 0, table[i]);
        offset += table[i].type->size();
    }
    row->add_null_bitmap(offset, 0);

    lin->add_sequence(address, master_stride_bytes, std::move(row));
    linearization(std::move(lin));
}

RowStore::~RowStore()
{
    /* TODO 1.2.1: Free allocated memory. */
    //free(address);
}

std::size_t RowStore::num_rows() const
{
    /* TODO 1.2.1: Implement */
}

//Append another tuple row dynamically
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
