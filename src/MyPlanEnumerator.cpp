#include "MyPlanEnumerator.hpp"


using namespace m;


void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const
{
    const AdjacencyMatrix M(G); // compute the adjacency matrix for graph G

    // TODO 3.1.2.2 Implement an algorithm for plan enumeration
}
