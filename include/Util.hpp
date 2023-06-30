#pragma once

#include "PhysicsSim/Fundamentals/Vector2.hpp"
#include "SFML/Config.hpp"
#include <sstream>

template <typename T>
inline void safeStreamRead(std::istream& ss, T& value) {
    if (!ss.good()) throw std::runtime_error("Not enough columns - read failed");
    ss >> value;
}

template <>
inline void safeStreamRead<sf::Uint8>(std::istream& ss, sf::Uint8& value) {
    unsigned t;
    safeStreamRead(ss, t);
    value = static_cast<sf::Uint8>(t);
}

template <Vectorisable T>
std::istream& Vector2<T>::operator>>(std::istream& is, Vector2<T>& v) {
    safeStreamRead(is, v.x);
    safeStreamRead(is, v.y);
    return is;
}