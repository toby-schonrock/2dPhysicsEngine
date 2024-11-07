#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

template <typename>
class PepperedVector;

template <typename>
class CompactMap;

template <typename T>
struct Index {
  private:
    std::size_t id    = 0;
    constexpr Index() = default;
    explicit Index(std::size_t id_) : id(std::move(id_)) {}
    explicit         operator std::size_t() const { return id; }
    explicit         operator long long() const { return static_cast<long long>(id); }
    constexpr Index& operator+=(const Index& obj) {
        id += obj.id;
        return *this;
    }
    constexpr Index& operator-=(const Index& obj) {
        id -= obj.id;
        return *this;
    }
    constexpr friend Index operator+(Index lhs, const Index& rhs) { return lhs += rhs; }
    constexpr friend Index operator-(Index lhs, const Index& rhs) { return lhs -= rhs; }
    constexpr Index&       operator++() {
        ++id;
        return *this;
    }
    constexpr Index& operator--() {
        --id;
        return *this;
    }
    friend class PepperedVector<T>;
    friend class CompactMap<T>;

  public:
    auto                 operator<=>(const Index& obj) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Index<T>& i) { return os << i.id; }
};

template <typename T> // consider templating by iterator return type
class IterableMap {
  public:
    struct Elem {
        Index<T> ind;
        T        obj;
    };
    IterableMap()                                                  = default;
    virtual ~IterableMap()                                         = default;
    IterableMap(const IterableMap& other)                          = delete;
    IterableMap&               operator=(const IterableMap& other) = delete;
    virtual Index<T>           add(const T& elem);
    virtual void               rem(const Index<T>& ind);
    virtual void               rem(const std::vector<Index<T>>& ind);
    [[nodiscard]] virtual Elem front() const;
    [[nodiscard]] virtual Elem back() const;
    [[nodiscard]] virtual bool contains(const Index<T>& ind) const;
    [[nodiscard]] virtual bool empty() const;
    virtual void               reserve(std::size_t n);

    [[nodiscard]] virtual T&       operator[](const Index<T>& ind);
    [[nodiscard]] virtual const T& operator[](const Index<T>& ind) const;
};

template <typename T>
class PepperedVector : IterableMap<T> {
  private:
    using Elem = IterableMap<T>::Elem;
    struct ElemExists {
        bool isDeleted;
        Elem elem;
    };
    std::vector<ElemExists> vec;

  public:
    // PepperedVector& operator=(const PepperedVector& other) = default;
    // if you do not store the return value you will only be able to retrive/delete this element
    // through iteration
    Index<T> add(const T& elem)
        override { // TODO storing deleted in stack and refilling the holes (order not important)
        Index<T> ind = Index<T>(vec.size());
        vec.emplace_back(false, Elem(ind, elem));
        return ind;
    }
    // deletion does not change underyling array so feel free to delete while iterating over
    // PepperedVector
    void rem(const Index<T>& ind) override {
        if (ind.id < 0 || ind.id >= vec.size() || vec[ind.id].isDeleted)
            throw std::logic_error("Tried to delete Element in PepperedVector that doesn't exist "
                                   "or was already deleted");
        vec[ind.id].isDeleted = true;
    }

    void rem(const std::vector<Index<T>>& ind) override {
        for (auto i: ind) {
            rem(i);
        }
    }

    [[nodiscard]] Elem front() const override { return *(this->cbegin()); };
    [[nodiscard]] Elem back() const override { return *(--(this->cend())); };

    // checks if index still references a valid element
    [[nodiscard]] bool contains(const Index<T>& ind) const override {
        if (ind.id < 0 || ind.id >= vec.size()) return false;
        return !vec[ind.id].isDeleted;
    }

    [[nodiscard]] bool empty() const override { // TODO potentialy faster with stack (check
                                                // stack.size() vs vec.size()) sad cause O(n) rn
        bool empty = true;
        for (const auto& v: vec) {
            if (!v.isDeleted) {
                empty = false;
            }
        }
        return empty;
    }

    void reserve(std::size_t n) override { vec.reserve(n); }

    [[nodiscard]] T&       operator[](const Index<T>& ind) override { return vec[ind.id].elem.obj; }
    [[nodiscard]] const T& operator[](const Index<T>& ind) const override {
        return vec[ind.id].elem.obj;
    }

  private:
    struct Iterator {
        using difference_type = std::ptrdiff_t;
        using value_type      = Elem;
        using pointer         = Elem*; // or also value_type*
        using reference       = Elem&; // or also value_type&

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
        ConstIterator(const PepperedVector* vec, VecIt it) : vec(vec), it(it) {
            std::cout << "const it made!";
        }
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
        auto it  = Iterator(this, beg);
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

template <typename T>
class CompactMap : IterableMap<T> {
  private:
    using Elem = IterableMap<T>::Elem;
    std::vector<Elem>                  vec{};
    std::map<std::size_t, std::size_t> map{};
    Index<T>                           nextInd{};

  public:
    // if you do not store the return value you will only be able to retrive/delete this element
    // through iteration
    Index<T> add(const T& elem) override {
        map[nextInd.id] = vec.size();
        Index<T> ind{nextInd};
        vec.emplace_back(ind, elem);
        ++nextInd;
        return ind;
    }
    // deletion changes underyling array and therefore invalidates iterators-pointers
    void rem(const Index<T>& ind) override {
        auto find = map.find(ind.id);
        if (find == map.end())
            throw std::logic_error("Tried to delete Element in CompactVector that doesn't exist");

        auto delIndex = find->second;

        // swap elem to be deleted with last elem
        map[vec.back().ind.id] = delIndex;
        vec[delIndex]          = vec.back();

        // delete element
        map.erase(ind.id);
        vec.pop_back();
    }
    // deletion changes underyling array and therefore invalidates iterators-pointers
    void rem(const std::vector<Index<T>>& inds) override { // TODO actually make more efficient
        for (auto i: inds) {
            rem(i);
        }
    }

    [[nodiscard]] Elem front() const override { return vec.front(); };
    [[nodiscard]] Elem back() const override { return vec.back(); };

    // checks if index still references a valid element
    [[nodiscard]] bool contains(const Index<T>& ind) const override { return map.contains(ind.id); }

    [[nodiscard]] bool empty() const override { return vec.empty(); }

    void reserve(std::size_t n) override { vec.reserve(n); }

    // breaks if exists(ind) == false
    [[nodiscard]] T& operator[](const Index<T>& ind) override { return vec[map.at(ind.id)].obj; }
    [[nodiscard]] const T& operator[](const Index<T>& ind) const override {
        return vec[map.at(ind.id)].obj;
    }

  public:
    std::vector<Elem>::iterator begin() { return vec.begin(); }
    std::vector<Elem>::iterator end() { return vec.end(); }
    auto                        cbegin() const { return vec.cbegin(); }
    auto                        cend() const { return vec.cend(); }
};

template <typename T>
using StableVector = PepperedVector<T>;