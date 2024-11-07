#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <vector>

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
};

template <typename T, typename RefType> // consider templating by iterator return type
class IterableMap {
  public:
    using Ref = Ref<T, RefType>;
    struct Elem {
        Ref ind;
        T   obj;
    };
    IterableMap()                                    = default;
    virtual ~IterableMap()                           = default;
    IterableMap(const IterableMap& other)            = delete;
    IterableMap& operator=(const IterableMap& other) = delete;
    virtual Ref  add(const T& elem);
    virtual void rem(const Ref& ind);
    template <std::ranges::forward_range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, Ref>
    void                       rem(const R&& range);
    [[nodiscard]] virtual Elem front() const;
    [[nodiscard]] virtual Elem back() const;
    [[nodiscard]] virtual bool contains(const Ref& ind) const;
    [[nodiscard]] virtual bool empty() const;
    virtual void               reserve(std::size_t n);

    [[nodiscard]] virtual T&       operator[](const Ref& ind);
    [[nodiscard]] virtual const T& operator[](const Ref& ind) const;
};

template <typename T, typename RefType = DefRefType>
class PepperedVector : IterableMap<T, RefType> {
  private:
    using Ref  = IterableMap<T, RefType>::Ref;
    using Elem = IterableMap<T, RefType>::Elem;
    struct ElemExists {
        bool isDeleted;
        Elem elem;
    };
    std::vector<ElemExists>                                                               vec;
    std::priority_queue<std::size_t, std::vector<std::size_t>, std::greater<std::size_t>> queue;

  public:
    // store the return value or you will only be able to retrieve/delete this element through
    // iteration
    Ref add(const T& elem) override {
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
    void rem(const Ref& ind) override {
        vec.at(ind.id).isDeleted = true;
        queue.push(ind.id);
    }

    template <std::ranges::forward_range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, Ref>
    void rem(R&& range) { // std::priority_queue::push_range doesn't exist yet :(
        // std::ranges::for_each(range, &this->rem);
        for (auto r: range) {
            rem(r);
        }
    }

    [[nodiscard]] Elem front() const override { return *(this->cbegin()); };
    [[nodiscard]] Elem back() const override { return *(--(this->cend())); };

    // checks if index still references a valid element
    [[nodiscard]] bool contains(const Ref& ind) const override {
        if (ind.id < 0 || ind.id >= vec.size()) return false;
        return !vec[ind.id].isDeleted;
    }

    [[nodiscard]] bool empty() const override { return vec.size() - queue.size() != 0; }

    void reserve(std::size_t n) override { vec.reserve(n); }

    [[nodiscard]] T&       operator[](const Ref& ind) override { return vec[ind.id].elem.obj; }
    [[nodiscard]] const T& operator[](const Ref& ind) const override {
        return vec[ind.id].elem.obj;
    }

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
        using pointer         = Elem*; // or also value_type*
        using reference       = Elem&; // or also value_type&

        using VecIt     = std::vector<ElemExists>::const_iterator;
        ConstIterator() = default;
        ConstIterator(const PepperedVector* vec, VecIt it) : vec(vec), it(it) {}
        ConstIterator& operator=(const ConstIterator& other) {
            vec = other.vec;
            it  = other.it;
            return *this;
        };

        const Elem&   operator*() const { return ConstIterator::it->elem; }
        const pointer operator->() { return &(ConstIterator::it->elem); }

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
class CompactMap : IterableMap<T, RefType> {
  private:
    using Elem = IterableMap<T, RefType>::Elem;
    using Ref  = IterableMap<T, RefType>::Ref;
    std::vector<Elem>                  vec{};
    std::map<std::size_t, std::size_t> map{};
    Ref                                nextInd{};

  public:
    // if you do not store the return value you will only be able to retrive/delete this element
    // through iteration
    Ref add(const T& elem) override {
        map[nextInd.id] = vec.size();
        Ref ind{nextInd};
        vec.emplace_back(ind, elem);
        ++nextInd;
        return ind;
    }
    // deletion changes underyling array and therefore invalidates iterators-pointers
    void rem(const Ref& ind) override {
        auto delIndex = map.at(ind.id);

        // swap elem to be deleted with last elem
        map[vec.back().ind.id] = delIndex;
        vec[delIndex]          = vec.back();

        // delete element
        map.erase(ind.id);
        vec.pop_back();
    }
    // deletion changes underyling array and therefore invalidates iterators-pointers
    template <std::ranges::forward_range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, Ref>
    void rem(R&& range) {
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

    [[nodiscard]] Elem front() const override { return vec.front(); };
    [[nodiscard]] Elem back() const override { return vec.back(); };

    // checks if index still references a valid element
    [[nodiscard]] bool contains(const Ref& ind) const override { return map.contains(ind.id); }

    [[nodiscard]] bool empty() const override { return vec.empty(); }

    void reserve(std::size_t n) override { vec.reserve(n); }

    // breaks if exists(ind) == false
    [[nodiscard]] T&       operator[](const Ref& ind) override { return vec[map.at(ind.id)].obj; }
    [[nodiscard]] const T& operator[](const Ref& ind) const override {
        return vec[map.at(ind.id)].obj;
    }

  public:
    std::vector<Elem>::iterator begin() { return vec.begin(); }
    std::vector<Elem>::iterator end() { return vec.end(); }
    auto                        begin() const { return vec.cbegin(); }
    auto                        end() const { return vec.cend(); }
    auto                        cbegin() const { return vec.cbegin(); }
    auto                        cend() const { return vec.cend(); }
};

template <typename T, typename RefType = DefRefType>
using StableVector = CompactMap<T, RefType>;