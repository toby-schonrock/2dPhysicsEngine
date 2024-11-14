#pragma once

#include "details/StableVector.hpp"
#include "details/Vector2.hpp"

namespace physenv {

struct Point {
  public:
    Vec2   pos{};
    Vec2   vel{};
    Vec2   force{};
    double mass  = 1.0F;
    bool   fixed = false;

    Point() = default;

    Point(const Vec2& pos_, double mass_, const Vec2& vel_ = Vec2(), bool fixed_ = false)
        : pos(pos_), vel(vel_), mass(mass_), fixed(fixed_) {}

    void update(double deltaTime, double gravity) {
        if (!fixed) {
            vel += (force / mass + Vec2(0, -gravity)) * deltaTime; // TODO euler integration could be
                                                               // improved (e.g. runge kutta)
            pos += vel * deltaTime;
        }
        force = Vec2();
    }

    bool                 operator==(const Point& obj) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << p.fixed << ' ' << p.pos << ' ' << p.vel << ' ' << p.mass;
    }
};

using PointRef = details::Ref<Point>;

} // namespace physenv