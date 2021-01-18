#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;

uint64_t findMSB(uint64_t n) {
    if (n == 0)
        return -1;

    uint64_t msb = 0;
    n = n / 2;
    while (n != 0) {
        n = n / 2;
        msb++;
    }

    return (msb);
}

std::vector<SmallBitset> getSubsets(SmallBitset S){
    if (S.empty()) return std::vector<SmallBitset>();

    int pos[S.size()];
    int count = 0;
    std::vector<SmallBitset> subsets;

    for (int i = 0; i <= findMSB((uint64_t) S); i++){
        if (S.contains(i))
            pos[count++] = i;
    }

    for(int i =0; i<S.size(); i++){
        auto tmp = SmallBitset();
        tmp.set(pos[i]);
        subsets.push_back(tmp);
    }

    /*
    auto n = S.size();
    for (int i=0; i< pow(2, n); i++){
        SmallBitset bitRepr = SmallBitset(i);
        SmallBitset tmp = SmallBitset();
        for (int j=0; j<n; j++) {

            if (bitRepr.contains(j))
                tmp.set(pos[j]);
        }

        subsets.push_back(tmp);
    }

     */
    return subsets;

}

void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const {
    // compute the adjacency matrix for graph G
    const AdjacencyMatrix M(G);

    auto csgs = EnumerateCsg(G, M);

    for (uint64_t S : csgs) {
        auto cmps = EnumerateCmp(G, M, SmallBitset(S));

        for (uint64_t S_r : cmps) {
            PT.update(CF, SmallBitset(S), SmallBitset(S_r), 0);
        }
    }
}

SmallBitset makeB(uint64_t i) {
    return SmallBitset((1 << (i + 1)) - 1);
}

vector<uint64_t> EnumerateCsgRec(const QueryGraph &G, SmallBitset S, SmallBitset X, AdjacencyMatrix M) {
    // find all neighbours and exclude all prohibited ones
    SmallBitset neighbours = M.neighbors(S);
    auto N = neighbours - X;

    vector<uint64_t> erg;
    vector<uint64_t> rec_erg;


    auto subsets = getSubsets(N);
    for (SmallBitset i : subsets){
        erg.push_back((uint64_t) (S | i));

        //join return vector of recursion to own return values
        auto toAdd = EnumerateCsgRec(G, S | i, X | N, M);
        rec_erg.insert(rec_erg.end(), toAdd.begin(), toAdd.end());
    }

    /*
    //for(auto i = (uint64_t )N; i > 0; i--){
    for (uint64_t i = 1; i <= (uint64_t) N; i++) { //
        if (SmallBitset(i).is_subset(N)) {
            erg.push_back((uint64_t) (S | SmallBitset(i)));

            //join return vector of recursion to own return values
            auto toAdd = EnumerateCsgRec(G, S | SmallBitset(i), X | N, M);
            rec_erg.insert(rec_erg.end(), toAdd.begin(), toAdd.end());
        }
    } */

    erg.insert(erg.end(), rec_erg.begin(), rec_erg.end());
    return erg;
}

vector<uint64_t> EnumerateCsg(const QueryGraph &G, AdjacencyMatrix M) {
    vector<uint64_t> erg;

    for (int i = G.sources().size() - 1; i > -1; i--) {
        erg.push_back(1 << i); // node itself as subgraph

        // B_i set of valid nodes increases with smaller node i
        auto tmp = EnumerateCsgRec(G, SmallBitset(1 << i), makeB(i), M);
        erg.insert(erg.end(), tmp.begin(), tmp.end()); // recursively enlarge subgraph with node
    }

    return erg;
}

uint64_t getFirstSetBitPos(uint64_t n) {
    return log2(n & -n);
}

vector<uint64_t> EnumerateCmp(const QueryGraph &G, AdjacencyMatrix M, SmallBitset S) {
    //min(S_1) ist Knoten mit kleinsten Index in S_1
    auto minS = getFirstSetBitPos((uint64_t) S);
    SmallBitset B_minS = makeB(minS);//makeB_i((uint64_t)S, (uint64_t) minS);
    SmallBitset X = B_minS | S;

    SmallBitset N = M.neighbors(S) - X;

    vector<uint64_t> erg;

    auto i = findMSB((uint64_t) N);
    auto t_N = (uint64_t) N;
    while (i != -1) { //TODO not nice! -1 overflows to max value
        erg.push_back(1 << i);

        auto tmp = EnumerateCsgRec(G, SmallBitset(1 << i), (X | makeB_i((uint64_t) N, i)), M);
        erg.insert(erg.end(), tmp.begin(), tmp.end());

        t_N = t_N - (1 << i);
        i = findMSB(t_N);
    }

    return erg;
}

SmallBitset makeB_i(uint64_t N, uint64_t i) {
    auto Nset = SmallBitset(N);
    SmallBitset erg = SmallBitset();
    for (auto k = 0; k <= i; k++) {
        if (Nset.contains(k))
            erg.set(k);
    }

    return erg;
}

