#include <mutable/mutable.hpp>


struct MyPlanEnumerator : m::PlanEnumerator
{
    void operator()(const m::QueryGraph &G, const m::CostFunction &CF, m::PlanTable &PT) const override;


};

std::map<int, std::vector<m::SmallBitset>> getSubPlanBitmaps(const std::vector<m::DataSource*> arr, const m::AdjacencyMatrix M, m::PlanTable &PT);