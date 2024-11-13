#include "physenv/Persistance.hpp"
#include "gtest/gtest.h"
#include <ranges>

template <typename T, typename Q>
void equalityCheck(T subj, const std::vector<Q>& vec) { // subj is taken by value
    EXPECT_EQ(subj.size(), vec.size());
    for (auto v: vec) {
        EXPECT_TRUE(subj.contains(v.first));
        EXPECT_TRUE(subj[v.first] == v.second);
        subj.erase(v.first);
    }
    EXPECT_EQ(subj.size(), 0);
}

using namespace physenv;

template <typename T>
class StableVectors : public testing::Test {
  public:
    T vec;
};

using TestedTypes = testing::Types<details::PepperedVector<int>, details::CompactMap<int>>;
TYPED_TEST_SUITE(StableVectors, TestedTypes);

TYPED_TEST(StableVectors, IsEmptyIntially) { EXPECT_TRUE(this->vec.empty()); }

TYPED_TEST(StableVectors, Adding) {
    auto& vec     = this->vec;
    auto  newItem = vec.insert(0);
    EXPECT_TRUE(vec.contains(newItem));
    EXPECT_EQ(vec[newItem], 0);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_FALSE(vec.empty());

    std::vector<std::pair<decltype(newItem), int>> curr{{newItem, 0}};
    std::ranges::for_each(std::views::iota(1, 10),
                          [&](auto i) { curr.emplace_back(vec.insert(i), i); });
    equalityCheck(vec, curr);
}

TYPED_TEST(StableVectors, Iterating) {
    auto& vec = this->vec;

    for (auto i: std::views::iota(0, 10)) {
        vec.insert(i);
    }

    int sum{};
    std::ranges::for_each(vec, [&](const auto& i) {
        EXPECT_EQ(i.obj, vec[i.ind]);
        sum += i.obj;
    });
    EXPECT_EQ(sum, 45);
}

TYPED_TEST(StableVectors, RemovingSimple) {
    auto& vec     = this->vec;
    auto  newItem = vec.insert(0);
    vec.erase(newItem);
    EXPECT_FALSE(vec.contains(newItem));
    EXPECT_EQ(vec.size(), 0);
    newItem = vec.insert(0);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_TRUE(vec.contains(newItem));
    auto newItem2 = vec.insert(1);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_TRUE(vec.contains(newItem));
    EXPECT_TRUE(vec.contains(newItem2));
}

TYPED_TEST(StableVectors, Removing) {
    auto& vec     = this->vec;
    auto  newItem = vec.insert(0);
    vec.erase(newItem);

    std::vector<std::pair<decltype(newItem), int>> curr{};
    for (auto i: std::views::iota(0, 10)) {
        curr.emplace_back(vec.insert(i), i);
    }
    equalityCheck(vec, curr); // curr = 0,1,2,3,4,5,6,7,8,9

    for (auto i: std::vector<std::size_t>{7, 4, 3, 5, 5}) {
        vec.erase(curr.at(i).first);
        curr.erase(curr.begin() + static_cast<signed long long>(i));
        // curr.erase(curr.begin() + static_cast<signed long long>(i));
    }
    equalityCheck(vec, curr); // curr = 0,1,2,5,6

    for (auto i: std::views::iota(10, 15)) {
        curr.emplace_back(vec.insert(i), i);
    }
    equalityCheck(vec, curr); // curr = 0,1,2,5,6,10,11,12,13,14

    std::vector<decltype(newItem)> removes;
    for (auto i: std::vector<std::size_t>{9, 0, 3, 2, 5}) {
        removes.push_back(curr.at(i).first);
        curr.erase(curr.begin() + static_cast<long long int>(i));
    }
    vec.erase(removes);
    equalityCheck(vec, curr); // cur = 1,2,10,11,12

    for (int i: std::views::iota(0, 201)) {
        curr.emplace_back(vec.insert(i), i);
    }
    equalityCheck(vec, curr);

    int sum = std::accumulate(vec.begin(), vec.end(), 0,
                              [](auto total, const auto& e) { return total += e.obj; });
    EXPECT_EQ(sum, 20136);

    removes = {};
    for (auto i: curr) {
        removes.push_back(i.first);
    }
    vec.erase(removes);
    EXPECT_EQ(vec.size(), 0);
}

class EngineTest : public testing::Test {
  protected:
    Engine e = Engine::softbody({5, 5}, {0.0f, 0.0f}, 10.0f, 10.0f, 10.0f, 1.0f);
};

TEST_F(EngineTest, ConstructSoftBody) {
    ASSERT_EQ(e.points.size(), 25);
    ASSERT_EQ(e.springs.size(), 72);
    ASSERT_EQ(e.polys.size(), 2);
}

TEST_F(EngineTest, SaveAndLoadEng) {
    std::filesystem::path p = "SaveAndLoadTest.csv";
    std::cout << std::filesystem::current_path() << p << "\n";
    persisitance::saveEng(e, p, {true, true, true});
    Engine e2{};
    persisitance::loadEng(e2, p, false, {true, true, true});
    ASSERT_EQ(e.points.size(), e2.points.size());
    ASSERT_EQ(e.springs.size(), e2.springs.size());
    ASSERT_EQ(e.polys.size(), e2.polys.size());
}

TEST_F(EngineTest, SaveAndLoadEngPartial) {
    std::filesystem::path p = "SaveAndLoadTestPartial.csv";
    std::cout << std::filesystem::current_path() << p << "\n";
    persisitance::saveEng(e, p, {true, false, true});
    Engine e2{};
    persisitance::loadEng(e2, p, false, {true, true, false});
    ASSERT_EQ(e.points.size(), e2.points.size());
    ASSERT_EQ(e2.springs.size(), 0);
    ASSERT_EQ(e2.polys.size(), 0);
}