#include <mutable/mutable.hpp>


struct MyPlanEnumerator : m::PlanEnumerator
{
    void operator()(const m::QueryGraph &G, const m::CostFunction &CF, m::PlanTable &PT) const override;


};

std::vector<uint64_t> EnumerateCsg(const m::QueryGraph &G, m::AdjacencyMatrix M);
std::vector<uint64_t> EnumerateCsgRec(const m::QueryGraph &G, m::SmallBitset S, m::SmallBitset X, m::AdjacencyMatrix M);
uint64_t getFirstSetBitPos(uint64_t n);
m::SmallBitset makeB(uint64_t i);
std::vector<uint64_t> EnumerateCmp(const m::QueryGraph &G, m::AdjacencyMatrix M, m::SmallBitset S);
m::SmallBitset makeB_i(uint64_t N, uint64_t i);
uint64_t getFirstSetBitPos(uint64_t n);