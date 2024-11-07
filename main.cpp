#include <algorithm>
#include <iostream>

#include "include/fundamentals/StableVector.hpp"

int main() {
    PepperedVector<int> s;
    // CompactVector<int> s;
    s.add(1);
    auto b = s.add(2);
    s.add(3);
    s.rem(b);
    s.add(4);
    auto c = std::find_if(s.begin(), s.end(), [](const auto& i) { return i.obj == 1; });
    s.rem(c->ind);
    s.add(5);
    for (const auto& i: s) {
        std::cout << i.ind << " " << i.obj << '\n';
    }

    CompactMap<int> q;
    // CompactVector<int> s;
    q.add(1);
    b = q.add(2);
    q.add(3);
    q.rem(b);
    q.add(4);
    auto d = std::find_if(q.begin(), q.end(), [](const auto& i) { return i.obj == 1; });
    q.rem(d->ind);
    q.add(5);
    for (const auto i: q) {
        std::cout << i.ind << " " << i.obj << '\n';
    }

    // StableVector<std::string> v;
    // auto                      d = v.add("hi");

    return 0;
}