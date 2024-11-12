#pragma once

#include "details/Vector2.hpp"

namespace physenv {

template <typename T>
inline void safeStreamRead(std::istream& ss, T& value) {
    if (!ss.good()) throw std::runtime_error("Not enough columns - read failed");
    ss >> value;
}

template <details::Vectorisable T>
std::istream& operator>>(std::istream& is, details::Vector2<T>& v) {
    safeStreamRead(is, v.x);
    safeStreamRead(is, v.y);
    return is;
}

} // namespace physenv