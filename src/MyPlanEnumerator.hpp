#include <mutable/mutable.hpp>


struct MyPlanEnumerator : m::PlanEnumerator
{
    void operator()(const m::QueryGraph &G, const m::CostFunction &CF, m::PlanTable &PT) const override;


};

std::map<int, std::vector<m::SmallBitset>> getSubPlanBitmaps(const std::vector<m::DataSource*> arr, const m::AdjacencyMatrix M, m::PlanTable &PT);
std::vector<int> EnumerateCsg(const m::QueryGraph &G, m::AdjacencyMatrix M);
std::vector<int> EnumerateCsgRec(const m::QueryGraph &G, int S, m::SmallBitset X, m::AdjacencyMatrix M);
unsigned int getFirstSetBitPos(int n);
m::SmallBitset makeB(int i);
std::vector<int> EnumerateCmp(const m::QueryGraph &G, m::AdjacencyMatrix M, m::SmallBitset S);
m::SmallBitset makeB_i(int N, int i);