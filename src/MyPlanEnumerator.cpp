#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;

uint64_t findMSB(uint64_t n) { // MSB pos
    if (n == 0)
        return -1;

    uint64_t msb = 0;
    n = n / 2;
    while (n != 0) {
        n = n / 2;
        msb++;
    }

    return (msb);
    //return 63 - __builtin_clzl(n);
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

    auto i = findMSB((uint64_t) N);
    auto t_N = (uint64_t) N;
    while (i != -1) { //TODO not nice! -1 overflows to max value
        erg.push_back((uint64_t) (S | SmallBitset(1 << i)));

        //join return vector of recursion to own return values
        auto toAdd = EnumerateCsgRec(G, S | SmallBitset(1 << i), X | N, M);
        rec_erg.insert(rec_erg.end(), toAdd.begin(), toAdd.end());

        t_N = t_N - (1 << i);
        i = findMSB(t_N);
    }


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

uint64_t getFirstSetBitPos(uint64_t n) {  //LSB pos
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

