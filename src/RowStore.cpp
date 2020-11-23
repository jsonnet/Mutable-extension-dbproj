#include "RowStore.hpp"


RowStore::RowStore(const m::Table &table)
        : Store(table)
{
    /* TODO 1.2.1: Allocate memory. */
    std::size_t numAttributes = table.size();  //amount of attributes

    /* TODO 1.2.2: Create linearization. */
    auto lin = std::make_unique<m::Linearization>(m::Linearization::CreateInfinite(1));

    //generate the one row and add it to lin
    auto row = std::make_unique<m::Linearization>(m::Linearization::CreateFinite(numAttributes+1,1));
    //auto attributes = table.primary_key();

    std::size_t current_offset = 0;

    for(std::size_t i=0; i<table.size(); ++i){
        std::size_t size_of_attr = table[i].type->size(); //in bits

        //offset = start addresse, stride = length of my variable
        row->add_sequence(current_offset, 0, table[i]);
        current_offset += size_of_attr;
    }

    row->add_null_bitmap(current_offset, 0);

    //round_up(sum of all attribute bits needed + bitmap bits)
    size_t first_bytes = ((current_offset + numAttributes) / 8);
    auto align = table[(size_t)0].type->size();
    auto padding = (align - (first_bytes % align)) % align;

    std::size_t master_stride_bytes = first_bytes + padding;


    auto address =  malloc(master_stride_bytes);

    lin->add_sequence((uint64_t)address, master_stride_bytes, std::move(row));
    linearization(std::move(lin));
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
