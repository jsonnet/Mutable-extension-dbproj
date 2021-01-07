#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;

bool sortfunc(SmallBitset a, SmallBitset b){
    return a.size() < b.size();
}

void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const
{
    const AdjacencyMatrix M(G); // compute the adjacency matrix for graph G
    auto subPlans = getSubPlanBitmaps(G.sources(), M,   PT);
    sort(subPlans.begin(), subPlans.end(), sortfunc);

    for (auto i=0; i<subPlans.size(); i++){
        for(auto j=i+1; j<subPlans.size()-1; j++){

            if (M.is_connected(subPlans[i] | subPlans[j]))
                PT.update(CF, subPlans[i], subPlans[j], 0);

        }
    }



}


std::vector<SmallBitset> getSubPlanBitmaps(std::vector<m::DataSource*> arr, const AdjacencyMatrix M, PlanTable &PT){
    std::vector<SmallBitset> erg;

    for(auto i=1; i< pow(2, arr.size()); i++){
        auto tmp = SmallBitset(i);
        if (M.is_connected(tmp))
            erg.push_back(tmp);
    }

    return erg;
}