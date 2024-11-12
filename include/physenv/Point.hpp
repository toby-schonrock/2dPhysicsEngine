#pragma once

#include "Util.hpp"
#include "details/StableVector.hpp"

namespace physenv {

struct Point {
  public:
    Vec2   pos;
    Vec2   vel{};
    Vec2   f;
    double mass = 1.0;
    bool   fixed;

    Point() = default;

    Point(const Vec2& pos_, double mass_, bool fixed_) : pos(pos_), mass(mass_), fixed(fixed_) {}

    void update(double deltaTime, double gravity) {
        if (!fixed) {
            vel += (f / mass + Vec2(0, -gravity)) * deltaTime; // TODO euler integration could be
                                                               // improved (e.g. runge kutta)
            pos += vel * deltaTime;
        }
        f = Vec2();
    }

    // used for saving point to file as text
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << p.fixed << ' ' << p.pos.x << ' ' << p.pos.y << ' ' << p.vel.x << ' ' << p.vel.y
                  << ' ' << p.mass;
    }

    // used for creating point from file
    friend std::istream& operator>>(std::istream& is, Point& p) {
        safeStreamRead(is, p.fixed);
        safeStreamRead(is, p.pos);
        safeStreamRead(is, p.vel);
        safeStreamRead(is, p.mass);
        if (is.good()) {
            throw std::runtime_error("To many columns for a point - file invalid");
        }
        return is;
    }
};

using PointRef = details::Ref<Point>;

} // namespace physenv