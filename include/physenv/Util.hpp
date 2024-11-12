#pragma once

#include "Polygon.hpp"
#include "Spring.hpp"

namespace physenv {

template <typename T>
inline void safeStreamRead(std::istream& is, T& value) {
    if (!is.good()) throw std::runtime_error("Not enough columns - read failed");
    is >> value;
}

template <typename T>
std::istream& operator>>(std::istream& is, details::Vector2<T>& v) {
    safeStreamRead(is, v.x);
    safeStreamRead(is, v.y);
    return is;
}

std::istream& operator>>(std::istream& is, Point& p) {
    safeStreamRead(is, p.fixed);
    safeStreamRead(is, p.pos);
    safeStreamRead(is, p.vel);
    safeStreamRead(is, p.mass);
    if (is.good()) {
        throw std::runtime_error("To many columns for a point - file invalid");
    }
    return is;
}

std::istream& operator>>(std::istream& is, Polygon& p) {
    std::vector<Vec2> verts{{}, {}, {}};
    safeStreamRead(is, verts[0]);
    safeStreamRead(is, verts[1]);
    safeStreamRead(is, verts[2]);
    while (is.good()) {
        verts.push_back(Vec2{});
        safeStreamRead(is, verts.back());
    }
    p = Polygon(verts);
    if (p.isConvex() == false)
        throw std::runtime_error("Polygon vertices do not form a convex polygon");
    return is;
}

// DOESN'T include PointRefs
std::istream& operator>>(std::istream& is, Spring& s) {
    safeStreamRead(is, s.springConst);
    safeStreamRead(is, s.naturalLength);
    safeStreamRead(is, s.dampFact);
    // safeStreamRead(is, s.p1);
    // safeStreamRead(is, s.p2);
    if (is.good()) {
        throw std::runtime_error("To many columns for a spring - file invalid");
    }
    return is;
}

} // namespace physenv