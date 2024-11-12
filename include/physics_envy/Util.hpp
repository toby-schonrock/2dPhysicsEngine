#pragma once

#include "fundamentals/StableVector.hpp"
#include "fundamentals/Vector2.hpp"

template <typename T>
inline void safeStreamRead(std::istream& ss, T& value) {
    if (!ss.good()) throw std::runtime_error("Not enough columns - read failed");
    ss >> value;
}

template <Vectorisable T>
std::istream& operator>>(std::istream& is, Vector2<T>& v) {
    safeStreamRead(is, v.x);
    safeStreamRead(is, v.y);
    return is;
}