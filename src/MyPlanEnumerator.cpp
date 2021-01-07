#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;


void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const {
    const AdjacencyMatrix M(G); // compute the adjacency matrix for graph G
    auto subPlans = getSubPlanBitmaps(G.sources(), M, PT);

    for (auto i = 1; i < G.sources().size(); i++) {  // we do not need to consider last size (only one sub elem)
        //cout << "I: " << i << endl;
        auto tmp = subPlans[i];

        for (auto j = 0; j < tmp.size() - 1; j++) {  // last elem does not have a further pair so -1
            //cout << tmp[j] << endl;
            //cout << "J: " << j << endl;
            for (auto k = j + 1; k < tmp.size(); k++) {
                //cout << "K: " << k << endl;
                if (M.is_connected(tmp[j] | tmp[k]))
                    PT.update(CF, tmp[j], tmp[k], 0);
            }
        }
    }

    1+1;

}


std::map<int, vector<SmallBitset>> getSubPlanBitmaps(std::vector<m::DataSource *> arr, const AdjacencyMatrix M, PlanTable &PT) {
    map<int, std::vector<SmallBitset>> erg;

    for (auto i = 1; i < pow(2, arr.size()); i++) {
        auto tmp = SmallBitset(i);
        if (M.is_connected(tmp))
            erg[tmp.size()].push_back(tmp);
    }

    return erg;
}