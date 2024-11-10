#pragma once

#include <iostream>
#include <queue>
#include <vector>

#include "absl/container/flat_hash_map.h"

struct DefRefType;

template <typename, typename>
class PepperedVector;

template <typename, typename>
class CompactMap;

template <typename T, typename RefType = DefRefType>
struct Ref {
  private:
    std::size_t id  = 0;
    constexpr Ref() = default;
    explicit Ref(std::size_t id_) : id(std::move(id_)) {}
    explicit       operator std::size_t() const { return id; }
    explicit       operator long long() const { return static_cast<long long>(id); }
    auto           operator<=>(const Ref& obj) const = default;
    constexpr Ref& operator+=(const Ref& obj) {
        id += obj.id;
        return *this;
    }
    constexpr Ref& operator-=(const Ref& obj) {
        id -= obj.id;
        return *this;
    }
    constexpr friend Ref operator+(Ref lhs, const Ref& rhs) { return lhs += rhs; }
    constexpr friend Ref operator-(Ref lhs, const Ref& rhs) { return lhs -= rhs; }
    constexpr Ref&       operator++() {
        ++id;
        return *this;
    }
    constexpr Ref& operator--() {
        --id;
        return *this;
    }
    friend class PepperedVector<T, RefType>;
    friend class CompactMap<T, RefType>;

  public:
    Ref(const Ref& obj)                            = default; // copy constructor
    Ref&                 operator=(const Ref& obj) = default; // copy assignment operator
    bool                 operator==(const Ref& obj) const { return id == obj.id; };
    friend std::ostream& operator<<(std::ostream& os, const Ref& i) { return os << i.id; }
    // friend std::ostream& operator>>(const Ref& i, std::istream& is) { return i.id >> is; }
};

template <typename T, typename RefType = DefRefType>
class PepperedVector {
  private:
    using Ref = Ref<T, RefType>;
    struct Elem {
        Ref ind;
        T   obj;
    };
    struct ElemExists {
        bool isDeleted;
        Elem elem;
    };
    std::vector<ElemExists>                                                               vec{};
    std::priority_queue<std::size_t, std::vector<std::size_t>, std::greater<std::size_t>> queue{};

  public:
    // store the return value or you will only be able to retrieve/delete this element through
    // iteration
    Ref insert(const T& elem) {
        Ref ind;
        if (!queue.empty()) { // if hole can be filled
            ind = Ref(queue.top());
            queue.pop();
            vec[ind.id] = {false, Elem(ind, elem)};
        } else { // back
            ind = Ref(vec.size());
            vec.emplace_back(false, Elem(ind, elem));
        }
        return ind;
    }
    // deletion does not change underyling array so can delete while iterating
    void erase(const Ref& ind) {
        vec.at(ind.id).isDeleted = true;
        queue.push(ind.id);
    }

    template <std::ranges::forward_range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, Ref>
    void erase(R&& range) { // std::priority_queue::push_range doesn't exist yet :(
                            // std::ranges::for_each(range, &this->rem);
        for (auto r: range) {
            erase(r);
        }
    }

    [[nodiscard]] Elem front() const { return *(this->cbegin()); };
    [[nodiscard]] Elem back() const { return *(--(this->cend())); };

    // checks if index still references a valid element
    [[nodiscard]] bool contains(const Ref& ind) const {
        if (ind.id < 0 || ind.id >= vec.size()) return false;
        return !vec[ind.id].isDeleted;
    }

    [[nodiscard]] std::size_t size() const {
        return vec.size() - queue.size();
    }

    [[nodiscard]] bool empty() const { return size() == 0; }

    void reserve(std::size_t n) { vec.reserve(n); }

    [[nodiscard]] T&       operator[](const Ref& ind) { return vec[ind.id].elem.obj; }
    [[nodiscard]] const T& operator[](const Ref& ind) const { return vec[ind.id].elem.obj; }

  private:
    struct Iterator {
        using difference_type = std::ptrdiff_t;
        using value_type      = Elem;
        using pointer         = Elem*; // or also value_type*
        using reference       = Elem&; // or also value_type&

        // using VecIt = std::vector<>;
        using VecIt = std::vector<ElemExists>::iterator;
        Iterator()  = default;
        Iterator(const PepperedVector* vec, VecIt it) : vec(vec), it(it) {}
        Iterator& operator=(const Iterator& other) {
            vec = other.vec;
            it  = other.it;
            return *this;
        };

        reference operator*() const { return it->elem; }
        pointer   operator->() { return &(it->elem); }

        Iterator& operator++() { // Prefix increment
            do {
                ++it;
            } while (it->isDeleted && it != (*vec).vec.cend());
            return *this;
        }

        Iterator operator++(int) { // Postfix increment
            Iterator tmp = *this;
            ++it;
            return tmp;
        }

        Iterator& operator--() { // Prefix decrement
            do {
                --it;
            } while (it->isDeleted && it != (*vec).vec.cbegin());
            return *this;
        }

        Iterator operator--(int) { // Postfix decrement
            Iterator tmp = *this;
            --it;
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) { return a.it == b.it; };
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.it != b.it; };

      private:
        const PepperedVector* vec; // has to be pointer to be default initialisable
        VecIt                 it;
    };

    struct ConstIterator {
        using difference_type = std::ptrdiff_t;
        using value_type      = Elem;
        using pointer         = const Elem*; // or also value_type*
        using reference       = const Elem&; // or also value_type&

        using VecIt     = std::vector<ElemExists>::const_iterator;
        ConstIterator() = default;
        ConstIterator(const PepperedVector* vec, VecIt it) : vec(vec), it(it) {}
        ConstIterator& operator=(const ConstIterator& other) {
            vec = other.vec;
            it  = other.it;
            return *this;
        };

        reference operator*() const { return ConstIterator::it->elem; }
        pointer   operator->() { return &(ConstIterator::it->elem); }

        ConstIterator& operator++() { // Prefix increment
            do {
                ++it;
            } while (it->isDeleted && it != (*vec).vec.cend());
            return *this;
        }

        ConstIterator operator++(int) { // Postfix increment
            ConstIterator tmp = *this;
            ++it;
            return tmp;
        }

        ConstIterator& operator--() { // Prefix decrement
            do {
                --it;
            } while (it->isDeleted && it != (*vec).vec.cbegin());
            return *this;
        }

        ConstIterator operator--(int) { // Postfix decrement
            ConstIterator tmp = *this;
            --it;
            return tmp;
        }

        friend bool operator==(const ConstIterator& a, const ConstIterator& b) {
            return a.it == b.it;
        };
        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) {
            return a.it != b.it;
        };

      private:
        const PepperedVector* vec; // has to be pointer to be default initialisable
        VecIt                 it;
    };

    static_assert(std::bidirectional_iterator<Iterator>);
    static_assert(std::bidirectional_iterator<ConstIterator>);

  public:
    Iterator begin() {
        auto beg = vec.begin();
        auto it  = Iterator(this, vec.begin());
        if (beg->isDeleted) ++it;
        return it;
    }
    Iterator end() {
        auto end = vec.end();
        do {
            --end;
        } while (end->isDeleted && end != vec.begin());
        return {this, ++end}; // end is out of bounds
    }
    ConstIterator begin() const { return cbegin(); }
    ConstIterator end() const { return cend(); }
    ConstIterator cbegin() const {
        auto beg = vec.cbegin();
        auto it  = ConstIterator(this, beg);
        if (beg->isDeleted) ++it;
        return it;
    }
    ConstIterator cend() const {
        auto end = vec.cend();
        do {
            --end;
        } while (end->isDeleted && end != vec.cbegin());
        return {this, ++end}; // end is out of bounds
    }
};

template <typename T, typename RefType = DefRefType>
class CompactMap {
  private:
    using Ref = Ref<T, RefType>;
    struct Elem {
        Ref ind;
        T   obj;
    };
    std::vector<Elem>                             vec{};
    absl::flat_hash_map<std::size_t, std::size_t> map{};
    Ref                                           nextInd{};

  public:
    // if you do not store the return value you will only be able to retrive/delete this element
    // through iteration
    Ref insert(const T& elem) {
        map[nextInd.id] = vec.size();
        Ref ind{nextInd};
        vec.emplace_back(ind, elem);
        ++nextInd;
        return ind;
    }
    // deletion changes underyling array and therefore invalidates iterators/pointers
    void erase(const Ref& ind) {
        auto delIndex = map.at(ind.id);

        map[vec.back().ind.id] = delIndex;
        vec[delIndex]          = vec.back();

        // delete element
        map.erase(ind.id);
        vec.pop_back();
    }
    // deletion changes underyling array and therefore invalidates iterators-pointers
    template <std::ranges::forward_range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, Ref>
    void erase(R&& range) {
        auto back = --vec.end();
        // TODO figure out for each here cause idk ^^ that breaks it
        for (auto ind: range) {
            auto delIndex     = map.at(ind.id);
            map[back->ind.id] = delIndex;
            vec[delIndex]     = *back;
            --back;
            map.erase(ind.id);
        }
        vec.erase(++back, vec.end());
    }

    [[nodiscard]] Elem        front() const { return vec.front(); };
    [[nodiscard]] Elem        back() const { return vec.back(); };
    [[nodiscard]] bool        contains(const Ref& ind) const { return map.contains(ind.id); }
    [[nodiscard]] std::size_t size() const { return vec.size(); }
    [[nodiscard]] bool        empty() const { return vec.empty(); }
    void                      reserve(std::size_t n) { vec.reserve(n); }
    [[nodiscard]] T&          operator[](const Ref& ind) {
        return vec[map.at(ind.id)].obj;
    } // TODO make map.at debug only
    [[nodiscard]] const T& operator[](const Ref& ind) const { return vec[map.at(ind.id)].obj; }

    std::vector<Elem>::iterator begin() { return vec.begin(); }
    std::vector<Elem>::iterator end() { return vec.end(); }
    auto                        begin() const { return vec.cbegin(); }
    auto                        end() const { return vec.cend(); }
    auto                        cbegin() const { return vec.cbegin(); }
    auto                        cend() const { return vec.cend(); }
};

template <typename T, typename RefType = DefRefType>
using StableVector = CompactMap<T, RefType>;