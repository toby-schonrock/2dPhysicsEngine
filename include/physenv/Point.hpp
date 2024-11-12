#pragma once

#include "details/StableVector.hpp"
#include "details/Vector2.hpp"

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

    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << p.fixed << ' ' << p.pos.x << ' ' << p.pos.y << ' ' << p.vel.x << ' ' << p.vel.y
                  << ' ' << p.mass;
    }
};

using PointRef = details::Ref<Point>;

} // namespace physenv