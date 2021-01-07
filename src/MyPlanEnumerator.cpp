#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;

void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const
{
    const AdjacencyMatrix M(G); // compute the adjacency matrix for graph G
    auto subPlans = getSubPlanBitmaps(G.sources(), M,   PT);
    subPlans.size();

    for(SmallBitset s : subPlans)
        cout << s << endl;

    /*
        SELECT T0.id, T1.id, T2.id
        FROM T0, T1, T2
        WHERE T0.id = T1.fid_T0 AND T1.id = T2.fid_T1;
     */

    for (auto i=0; i<subPlans.size(); i++){
        for(auto j=1; j<subPlans.size(); j++){

            if (M.is_connected(subPlans[i] | subPlans[j]))
                PT.update(CF, subPlans[i], subPlans[j], 0);

        }
    }

}


std::vector<SmallBitset> getSubPlanBitmaps(std::vector<m::DataSource*> arr, const AdjacencyMatrix M, PlanTable &PT){
    std::vector<SmallBitset> erg;

    for(auto i=1; i< pow(2, arr.size()); i++){
        auto tmp = SmallBitset(i);
        //if (M.is_connected(tmp))
            //cout << SmallBitset(i) << " : "<< PT.c(SmallBitset(i)) << endl;
            erg.push_back(tmp);
    }

    return erg;
}