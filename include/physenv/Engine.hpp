#pragma once

#include <algorithm>
#include <vector>

#include "Point.hpp"
#include "Polygon.hpp"
#include "Spring.hpp"

namespace physenv {

class Engine {
  public:
    double                gravity;
    StableVector<Polygon> polys;
    StableVector<Point>   points;
    StableVector<Spring>  springs;

    Engine(double gravity_ = 0) : gravity(gravity_) {}

    void simFrame(double deltaTime) {
        // calculate spring force worth doing in parralel
        std::for_each(springs.begin(), springs.end(), [&](auto& spring) {
            spring.obj.springHandler(points[spring.obj.p1], points[spring.obj.p2]);
        });

        // update point positions
        std::for_each(points.begin(), points.end(),
                      [d = deltaTime, g = gravity](auto& point) { point.obj.update(d, g); });

        // collide points with polygons
        for (const auto& poly: polys) {
            for (auto& point: points) {
                if (poly.obj.isBounded(point.obj.pos) &&
                    poly.obj.isContained(
                        point.obj.pos)) // not sure if bounded check is still faster
                    poly.obj.colHandler(point.obj);
            }
        }
    }

    template <typename T>
    PointRef addPoint(T&& p) {
        return points.insert(std::forward<T>(p));
    }

    template <typename T>
    SpringRef addSpring(T&& s) {
        return springs.insert(std::forward<T>(s));
    }

    void rmvPoint(PointRef pos) {
        points.erase(pos);

        // can be replace with ranges algo
        std::vector<SpringRef> remIds;
        for (const auto& spring: springs) {
            if (spring.obj.p1 == pos || spring.obj.p2 == pos) { // delete
                remIds.push_back(spring.ind);
            }
        }
        springs.erase(remIds);
    }

    void rmvSpring(SpringRef pos) { springs.erase(pos); }

    std::pair<PointRef, double> findClosestPoint(const Vec2 pos) const { //
        if (points.empty()) throw std::logic_error("Finding closest point with no points?!? ;)");
        double   closestDist = std::numeric_limits<double>::infinity();
        PointRef closestPos  = points.cbegin()->ind;
        for (auto p: points) {
            Vec2   diff = pos - p.obj.pos;
            double dist = diff.x * diff.x + diff.y * diff.y;
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = p.ind;
            }
        }
        return std::pair<PointRef, double>(closestPos, std::sqrt(closestDist));
    }

    std::pair<SpringRef, double> findClosestSpring(const Vec2 pos) const {
        if (springs.empty()) throw std::logic_error("Finding closest spring with no springs?!? ;)");
        double    closestDist = std::numeric_limits<double>::infinity();
        SpringRef closestPos  = springs.cbegin()->ind;
        for (auto spring: springs) {
            double dist = pos.distToLine(points[spring.obj.p1].pos, points[spring.obj.p2].pos);
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = spring.ind;
            }
        }
        return std::pair<SpringRef, double>(closestPos, std::sqrt(closestDist));
    }

    // void reset() { load(Previous, true, {true, true, true}, false); }

    static Engine softbody(const details::Vector2<std::size_t>& size, const Vec2& simPos,
                           float gravity, float gap, float springConst, float dampFact) {
        Engine sim{gravity};

        sim.polys.reserve(2);
        sim.polys.insert(Polygon::Square(Vec2(1, 0), -0.75));
        sim.polys.insert(Polygon::Square(Vec2(9, 0), 0.75));

        sim.points.reserve(size.x * size.y);
        std::vector<PointRef> tempPointRefs;
        for (unsigned x = 0; x != size.x; ++x) {
            for (unsigned y = 0; y != size.y; ++y) {
                tempPointRefs.push_back(sim.addPoint(Point{Vec2(x, y) * gap + simPos, 1.0}));
            }
        }

        for (std::size_t x = 0; x != size.x; ++x) {
            for (std::size_t y = 0; y != size.y; ++y) {
                PointRef p = tempPointRefs[x + y * size.x];
                if (x < size.x - 1) {
                    if (y < size.y - 1) {
                        sim.addSpring(Spring{
                            springConst, dampFact, std::numbers::sqrt2 * static_cast<double>(gap),
                            p, tempPointRefs[x + 1 + (y + 1) * size.x]}); // down right
                    }
                    sim.addSpring(Spring{springConst, dampFact, gap, p,
                                         tempPointRefs[x + 1 + (y)*size.x]}); // right
                }
                if (y < size.y - 1) {
                    if (x > 0) {
                        sim.addSpring(Spring{springConst, dampFact,
                                             std::numbers::sqrt2 * static_cast<double>(gap), p,
                                             tempPointRefs[x - 1 + (y + 1) * size.x]}); // down left
                    }
                    sim.addSpring(Spring{springConst, dampFact, gap, p,
                                         tempPointRefs[x + (y + 1) * size.x]}); // down
                }
            }
        }
        return sim;
    }
};

} // namespace physenv

// using PointRef = physenv::PointRef;
// using SpringRef = physenv::SpringRef;
// using PolyRef = physenv::PolyRef;