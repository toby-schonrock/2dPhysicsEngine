#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "Point.hpp"
#include "Polygon.hpp"
#include "Spring.hpp"
#include "Util.hpp"

namespace physenv {

static const std::string PointHeaders{"point-id fixed posx posy velx vely mass color(rgba)"};
static const std::string SpringHeaders =
    "spring-id spring-const natural-length damping-factor point1 point2";
static const std::string PolyHeaders = "polygon-verts: x y ...";

struct ObjectEnabled {
    bool points;
    bool springs;
    bool polygons;
};

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

    // void load(std::filesystem::path path, bool replace, ObjectEnabled enabled) {
    //     path.make_preferred();
    //     if (replace) {
    //         points.clear();
    //         springs.clear();
    //         polys.clear();
    //     }
    //     PointId pointOffset = static_cast<PointId>(points.size());

    //     std::ifstream file{path, std::ios_base::in};
    //     if (!file.is_open()) {
    //         throw std::runtime_error("failed to open file \"" + path.string() + '"');
    //     }

    //     std::string line;
    //     // points
    //     std::getline(file, line);
    //     if (line != PointHeaders)
    //         throw std::runtime_error("Point headers invalid: \n is - " + line + "\n should be - "
    //         +
    //                                  PointHeaders);

    //     std::stringstream ss;
    //     std::size_t       index = 0;
    //     while (true) {
    //         std::getline(file, line);
    //         if (checkIfHeader(SpringHeaders, line)) break;
    //         if (enabled.points) {
    //             Point point{};
    //             ss = std::stringstream(line);
    //             std::size_t temp;
    //             safeStreamRead(ss, temp);
    //             if (temp != index)
    //                 throw std::runtime_error("Non continous point indicie - " + line);
    //             safeStreamRead(ss, point);
    //             ++index;
    //             addPoint(point);
    //         }
    //     }

    //     index = 0;
    //     while (true) {
    //         std::getline(file, line);
    //         if (checkIfHeader(PolyHeaders, line)) break;
    //         if (enabled.springs) {
    //             ss = std::stringstream(line);
    //             Spring      spring{};
    //             std::size_t temp;
    //             safeStreamRead(ss, temp);
    //             if (temp != index)
    //                 throw std::runtime_error("Non continous spring indicie - " + line);
    //             safeStreamRead(ss, spring);
    //             spring.p1 += pointOffset;
    //             spring.p2 += pointOffset;
    //             ++index;
    //             addSpring(spring);
    //         }
    //     }

    //     while (!file.eof()) {
    //         std::getline(file, line);
    //         if (enabled.polygons) {
    //             ss = std::stringstream(line);
    //             if (ss.good()) { // deal with emtpy new lines at end
    //                 Polygon poly{};
    //                 safeStreamRead(ss, poly);
    //                 polys.push_back(poly);
    //             }
    //         }
    //     }
    // }

    void save(std::filesystem::path path, ObjectEnabled enabled) const {
        path.make_preferred();
        std::ofstream file{path};
        if (!file.is_open()) {
            throw std::runtime_error("Falied to open fstream \n");
        }
        std::cout << "made it so far" << std::endl;

        file << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
        file << PointHeaders << "\n";
        std::unordered_map<PointRef, std::size_t> tempPointIds;
        if (enabled.points) {
            std::size_t i = 0;
            for (const auto& p: points) {
                file << i << ' ' << p.obj << "\n";
                tempPointIds[p.ind] = i;
                tempPointIds.insert(std::make_pair(p.ind, i));
                ++i;
            }
        }
        file << SpringHeaders << "\n";
        if (enabled.springs) {
            std::size_t i = 0;
            for (const auto& s: springs) {
                file << i << ' ' << s.obj << "\n";
                ++i;
            }
        }
        file << PolyHeaders;
        if (enabled.polygons) {
            for (const auto& p: polys) {
                if (!p.obj.edges.empty()) file << "\n" << p.obj;
            }
        }
    }

    static bool checkIfHeader(const std::string& header, const std::string& line) {
        if (header.size() != line.size()) return false;
        for (std::size_t i = 0; i != line.size(); ++i) {
            if (header[i] != line[i]) return false;
        }
        return true;
    }

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
                tempPointRefs.push_back(sim.addPoint(Point{Vec2(x, y) * gap + simPos, 1.0, false}));
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