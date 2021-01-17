#include "MyPlanEnumerator.hpp"
#include <map>

using namespace m;
using namespace std;

int findMSB(int n) {
    if (n == 0)
        return -1;

    int msb = 0;
    n = n / 2;
    while (n != 0) {
        n = n / 2;
        msb++;
    }

    return (msb);
}

void MyPlanEnumerator::operator()(const QueryGraph &G, const CostFunction &CF, PlanTable &PT) const {
    const AdjacencyMatrix M(G); // compute the adjacency matrix for graph G


    auto csgs = EnumerateCsg(G, M);

    for (int S : csgs) {
        auto cmps = EnumerateCmp(G, M, SmallBitset(S));


        for (auto S_r : cmps) {
            PT.update(CF, SmallBitset(S), SmallBitset(S_r), 0);
            //PT.update(CF, SmallBitset(S_r), SmallBitset(S), 0);
        }
    }

}

SmallBitset makeB(int i) {
    return SmallBitset((1 << (i + 1)) - 1);
}

vector<int> EnumerateCsgRec(const QueryGraph &G, int S, SmallBitset xset, AdjacencyMatrix M) {
    //cout << S << " : " << X << endl;

    //Get neighbours of S
    SmallBitset neighbours = M.neighbors(SmallBitset(1 << S)); // find all neighbours

    auto N = neighbours - xset; // exclude all prohibited ones
    //cout << "N-X: " << N << " xset: " << xset << " neighbours: " << neighbours << endl;

    vector<int> erg;

    //TODO maybe enumerate subsets first ?!
    for(auto i = 1<<G.sources().size(); i > 0; i--){
    //for (auto i = 1; i <= (1 << G.sources().size()); i++) { //
        if (SmallBitset(i).is_subset(N)) {
            //emit ( i | S)
            //cout << "subset found: " << SmallBitset(i) << " emitting: " << SmallBitset(i | 1 << S) << endl;
            erg.push_back(i | 1 << S);
        }
    }

    vector<int> tmp_vector = erg;

    for (int i : tmp_vector) {
        int tmp = (uint64_t) xset | (uint64_t) N; // TODO check again
        auto toAdd = EnumerateCsgRec(G, i, SmallBitset(tmp), M);
        erg.insert(erg.end(), toAdd.begin(), toAdd.end());  //join return vector of recursion to own return values
    }

    return erg;
}

vector<int> EnumerateCsg(const QueryGraph &G, AdjacencyMatrix M) {
    vector<int> erg;

    for (int i = G.sources().size() - 1; i >= 0; i--) {
        erg.push_back(1 << i); // node itself as subgraph
        auto tmp = EnumerateCsgRec(G, i, makeB(i), M); // B_i set of valid nodes increases with smaller node i
        erg.insert(erg.end(), tmp.begin(), tmp.end()); // recursively enlarge subgraph with node
    }

    return erg;
}

unsigned int getFirstSetBitPos(int n) {
    return log2(n & -n);
}

vector<int> EnumerateCmp(const QueryGraph &G, AdjacencyMatrix M, SmallBitset S) {
    //min(S_1) ist Knoten mit kleinsten Index in S_1
    auto minS = getFirstSetBitPos((uint64_t) S);
    SmallBitset B_minS = makeB_i((uint64_t)S, (uint64_t) minS);
    auto X = B_minS | S;

    SmallBitset N = M.neighbors(S) - X;

    vector<int> erg;

    auto i = findMSB((uint64_t) N);
    auto t_N = (uint64_t) N;
    while (i != -1) {
        //cout << SmallBitset(1<<i) << endl;
        //emit(i)
        erg.push_back(1 << i);

        //auto tmp = EnumerateCsgRec(G, 1 << i, (uint64_t) (X | makeB((uint64_t) N)), M); // 100
        auto tmp = EnumerateCsgRec(G, 1 << (i-1), (X | makeB_i((uint64_t) N, i)), M); //TODO check filter
        //auto tmp = EnumerateCsgRec(G, 1 << i, SmallBitset(0/*(X /*| makeB_i((uint64_t) N, i)*/), M); //TODO check filter
        erg.insert(erg.end(), tmp.begin(), tmp.end());

        t_N = t_N - (1 << i);
        i = findMSB(t_N);
    }


    return erg;
}

SmallBitset makeB_i(int N, int i) {
    auto Nset = SmallBitset(N);
    SmallBitset erg = SmallBitset();
    for (auto k = 0; k < i; k++) {
        if (Nset.contains(k))
            erg.set(k);
    }

    return erg;
}


std::map<int, vector<SmallBitset>>
getSubPlanBitmaps(std::vector<m::DataSource *> arr, const AdjacencyMatrix M, PlanTable &PT) {
    map<int, std::vector<SmallBitset>> erg;

    for (auto i = 1; i < pow(2, arr.size()); i++) {
        auto tmp = SmallBitset(i);
        if (M.is_connected(tmp))
            erg[tmp.size()].push_back(tmp);
    }

    return erg;
}