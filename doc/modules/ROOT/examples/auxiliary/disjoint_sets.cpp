#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>

int main() {
    const int N = 6;
    std::vector<int> rank(N, 0);
    std::vector<int> parent(N);

    boost::disjoint_sets<int*, int*> ds(rank.data(), parent.data());

    // Make each element its own set
    for (int i = 0; i < N; ++i) {
        ds.make_set(i);
    }

    // Union some sets: {0,1,2} and {3,4}
    ds.union_set(0, 1);
    ds.union_set(1, 2);
    ds.union_set(3, 4);

    // Find representatives
    for (int i = 0; i < N; ++i) {
        std::cout << "find(" << i << ") = " << ds.find_set(i) << "\n";
    }

    // Check connectivity
    std::cout << "0 and 2 same set? " << (ds.find_set(0) == ds.find_set(2)) << "\n";
    std::cout << "0 and 5 same set? " << (ds.find_set(0) == ds.find_set(5)) << "\n";
}
