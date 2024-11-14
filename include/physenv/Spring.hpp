#pragma once

#include "Point.hpp"

namespace physenv {

struct Spring {
    double   springConst;
    double   dampFact;
    double   naturalLength;
    PointRef p1;
    PointRef p2;

    void springHandler(Point& point1, Point& point2) const {
        Vec2 force = forceCalc(point1, point2);
        point1.force += force; // equal and opposite reaction
        point2.force -= force;
    }

    Vec2 forceCalc(const Point& point1, const Point& point2) const {
        Vec2 diff = point1.pos - point2.pos; // broken out alot "yes this is faster! really like 3x"
        double diffMag = diff.mag();
        if (diffMag < 1E-30) return {}; // prevent 0 length spring exploding sim
        Vec2   unitDiff = diff / diffMag;
        double ext      = diffMag - naturalLength;
        double springf  = -springConst * ext;                               // f = -ke hookes law
        double dampf    = unitDiff.dot(point2.vel - point1.vel) * dampFact; // damping force
        return (springf + dampf) * unitDiff;
    }

    friend std::ostream& operator<<(std::ostream& os, const Spring& s) {
        return os << s.springConst << ' ' << s.naturalLength << ' ' << s.dampFact << ' ';
    }
};

using SpringRef = details::Ref<Spring>;

} // namespace physenv