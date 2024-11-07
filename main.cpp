#include <algorithm>
#include <iostream>
#include <ranges>

#include "include/fundamentals/StableVector.hpp"

template <typename T>
void print(const T& range) {
    std::ranges::for_each(
        range, [](const auto& e) { std::cout << e.obj << " at index - " << e.ind << "\n"; });
}

template <typename T>
void inc(T& range) {
    std::ranges::for_each(range, [](auto& e) { ++e.obj; });
}

template <typename T>
void test(T& vec) {
    using refType = decltype(vec.add(0));
    vec.add(1);
    auto a = vec.add(2);
    vec.add(3);
    vec.rem(a);
    vec.add(4);
    // std::cout << "should be 1, 3, 4 \n";
    // print(vec);
    auto b = std::ranges::find_if(vec, [](const auto& i) { return i.obj == 1; });
    vec.rem(b->ind);
    // std::cout << "should be 3, 4 \n";
    // print(vec);

    std::vector<refType> rems;
    std::ranges::for_each(std::views::iota(5, 13), [&](auto i) {
        if (i > 5 && i < 9) {
            rems.push_back(vec.add(i));
        } else {
            vec.add(i);
        }
    });
    std::cout << "should be 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 \n";
    print(vec);

    vec.rem(rems);
    std::cout << "should be 3, 4, 5, 9, 10, 11, 12 \n";
    print(vec);

    vec.add(1);
    inc(vec);
    std::cout << "should be 2, 4, 5, 6, 10, 11, 12, 13 \n";
    print(vec);
}

int main() {
    std::cout << "starting \n";
    PepperedVector<int> pep;
    test(pep);

    std::cout << "half done \n";

    struct refType42;
    CompactMap<int, refType42> map;
    test(map);
    return 0;
}
